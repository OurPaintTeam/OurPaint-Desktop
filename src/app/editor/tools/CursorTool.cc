#include "CursorTool.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <type_traits>
#include <utility>

#include "../../viewport/constraints/ConstraintLayout.h"
#include "../ConstraintActions.h"
#include "DSU.h"
#include "Document.h"

namespace {
bool sameGeometry(const sketch::SketchGeometry& first, const sketch::SketchGeometry& second) {
    if (first.index() != second.index()) {
        return false;
    }
    return std::visit(
        [&](const auto& value) {
            using Geometry = std::decay_t<decltype(value)>;
            const auto& other = std::get<Geometry>(second);
            if constexpr (std::is_same_v<Geometry, sketch::Point2>) {
                return value.position == other.position;
            } else if constexpr (std::is_same_v<Geometry, sketch::Line2>) {
                return value.start == other.start && value.end == other.end;
            } else if constexpr (std::is_same_v<Geometry, sketch::Circle2>) {
                return value.center == other.center && value.radius == other.radius;
            } else {
                return value.center == other.center && value.start == other.start && value.end == other.end;
            }
        },
        first);
}

bool sameConstraint(const sketch::ConstraintDefinition& first, const sketch::ConstraintDefinition& second) {
    return first.type == second.type && first.refs == second.refs && first.value == second.value && first.fixedPosition == second.fixedPosition &&
           first.tangencyBranch == second.tangencyBranch;
}
}  // namespace

CursorTool::CursorTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, Cpu2dPicker& picker, OverlayModel& overlay,
                       ConstraintActions& constraintActions, const app::ConstraintLayout& constraintLayout,
                       app::ConstraintPresentationState& constraintPresentation)
    : IInteractionTool(document, camera, snap, overlay, picker),
      sketch_(document.sketch()),
      constraintActions_(constraintActions),
      constraintLayout_(constraintLayout),
      constraintPresentation_(constraintPresentation) {}

std::optional<DimensionPickResult> CursorTool::pickDimension(glm::dvec2 position) const {
    const auto hit = picker_.pickDimensionAtScreenLogical(position.x, position.y, constraintLayout_);
    // Labels win over geometry, but a point remains reachable where a witness meets it.
    if (hit && (hit->part == DimensionPickResult::Part::Label || !picker_.pickPointAtScreenLogical(position.x, position.y))) {
        return hit;
    }
    return std::nullopt;
}

void CursorTool::beginDimensionDrag(const DimensionPickResult& hit, glm::dvec2 position) {
    if (!sketch_.constraint(hit.constraint)) {
        return;
    }
    const auto found = constraintPresentation_.labelPlacements.find(hit.constraint);
    const auto initial = found == constraintPresentation_.labelPlacements.end() ? std::nullopt : std::optional(found->second);
    dimensionDrag_ = DimensionDrag{hit.constraint, position, hit.labelCenter - position, initial};
    state_ = State::PressedDimension;
    overlay_.clearSelection();
    overlay_.hoveredDimension = hit.constraint;
    overlay_.draggedDimension = hit.constraint;
}

void CursorTool::updateDimensionDrag(glm::dvec2 position) {
    if (!dimensionDrag_) {
        return;
    }
    if (!sketch_.constraint(dimensionDrag_->constraint)) {
        endDimensionDrag(false);
        return;
    }
    constexpr double kDragThresholdPx = 3.0;
    if (state_ == State::PressedDimension && glm::length(position - dimensionDrag_->pressPosition) < kDragThresholdPx) {
        return;
    }
    const auto center = camera_.screenLogicalToWorld(position + dimensionDrag_->grabOffsetPx);
    if (!std::isfinite(center.x) || !std::isfinite(center.y)) {
        endDimensionDrag(false);
        return;
    }
    constraintPresentation_.labelPlacements[dimensionDrag_->constraint] = {{center.x, center.y}};
    state_ = State::DraggingDimension;
}

void CursorTool::endDimensionDrag(bool commit) {
    if (!dimensionDrag_) {
        return;
    }
    const auto id = dimensionDrag_->constraint;
    if (!sketch_.constraint(id)) {
        constraintPresentation_.labelPlacements.erase(id);
    } else if (!commit && state_ == State::DraggingDimension) {
        // Cancelling a presentation preview also restores the absence of an explicit placement.
        if (dimensionDrag_->initialPlacement) {
            constraintPresentation_.labelPlacements[id] = *dimensionDrag_->initialPlacement;
        } else {
            constraintPresentation_.labelPlacements.erase(id);
        }
    }
    dimensionDrag_.reset();
    overlay_.hoveredDimension.reset();
    overlay_.draggedDimension.reset();
    state_ = State::Idle;
}

std::optional<ActionReport> CursorTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;

    if (dimensionDrag_) {
        if (input::has_flag(e.buttons, input::MouseButton::Left)) {
            updateDimensionDrag({e.x, e.y});
        } else {
            // Recover if mouse capture was lost and no release event reached the tool.
            endDimensionDrag(false);
        }
        return std::nullopt;
    }
    overlay_.hoveredDimension.reset();
    if (state_ == State::Idle && e.buttons == input::MouseButton::None && e.modifiers == input::Modifiers::None) {
        if (const auto hit = pickDimension({e.x, e.y})) {
            overlay_.hoveredDimension = hit->constraint;
        }
    }

    if (input::has_flag(e.buttons, input::MouseButton::Left)) {
        if (state_ == State::DraggingSelection) {
            const auto& refs = overlay_.selection_.model.items();
            if (!refs.empty() && !dragStopped_ && (v.x != lastPos_.x || v.y != lastPos_.y)) {
                auto constraints = sketch_.constraints();
                if (!dragReport_) {
                    dragReport_.emplace(ActionReport{ActionKind::Drag});
                    std::set<sketch::EntityId> owners;
                    for (const auto& target : dragTargets_) {
                        owners.insert(target.point.entity);
                    }
                    dragReport_->requestedEntities = owners.size();
                    auto entities = sketch_.entities();
                    if (!entities) {
                        dragReport_->operationError = entities.error();
                        dragStopped_ = true;
                        return std::nullopt;
                    }
                    dragInitialEntities_ = std::move(entities.value());
                    if (!constraints) {
                        dragReport_->operationError = constraints.error();
                        dragStopped_ = true;
                        return std::nullopt;
                    }
                    dragInitialConstraints_ = std::move(constraints.value());
                }
                auto result = sketch_.entity(refs[0].entity);
                if (!result) {
                    dragReport_->operationError = result.error();
                    dragStopped_ = true;
                    return std::nullopt;
                }
                const auto& geometry = result.value().geometry;
                const auto* circle = std::get_if<sketch::Circle2>(&geometry);
                const sketch::Vec2 offset{v.x - pressWorldPos_.x, v.y - pressWorldPos_.y};
                bool c = false;
                if (circle) {
                    c = true;
                    for (auto constraint : constraints.value()) {
                        if (constraint.definition.type == sketch::ConstraintType::Radius &&
                            constraint.definition.refs[0] == refs[0]) {
                            c = false;
                        }
                    }
                }
                if (refs.size() == 1 && refs[0].sub == sketch::SubElement::Whole && c) {
                    const double radius = std::hypot(v.x - circle->center.x, v.y - circle->center.y);
                    if (radius == circle->radius) {
                        lastPos_ = v;
                        return std::nullopt;
                    }
                    dragAttempted_ = true;
                    const auto updated = sketch_.updateCircleRadius(refs[0].entity, radius);
                    if (!updated) {
                        dragReport_->mutationFailed(updated.error());
                        dragStopped_ = true;
                        return std::nullopt;
                    }
                    dragReport_->change = ModelChange::Changed;
                    const auto solved = sketch_.solve();
                    dragReport_->recordSolve(solved);
                    if (!solved || solved.value().status != sketch::SolveStatus::Converged) {
                        dragStopped_ = true;
                        return std::nullopt;
                    }
                } else {
                    dragAttempted_ = true;
                    const auto dragged = moveSelection(offset);
                    if (!dragged) {
                        dragReport_->mutationFailed(dragged.error());
                        dragStopped_ = true;
                        return std::nullopt;
                    }
                    dragReport_->change = ModelChange::Changed;
                    dragReport_->recordSolve(dragged);
                    if (dragged.value().status != sketch::SolveStatus::Converged) {
                        //dragStopped_ = true;
                        //return std::nullopt;
                    }
                }
                lastPos_ = v;
            }
        } else if (state_ == State::MarqueeSelection) {
            if (overlay_.selectionRect_.has_value()) {
                overlay_.selectionRect_->xMax = v.x;
                overlay_.selectionRect_->yMax = v.y;

                const bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift);

                const auto pickedRefs = picker_.pickInRectAtScreenLogical(lastScreenPos_.x, lastScreenPos_.y, e.x, e.y);
                if (shift) {
                    overlay_.selection_.model.replace(marqueeBaseSelection_);
                    overlay_.selection_.model.add(pickedRefs);
                } else {
                    overlay_.selection_.model.replace(pickedRefs);
                }
                lastPos_ = v;
            }
        }
    }
    return std::nullopt;
}

std::optional<ActionReport> CursorTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        endDimensionDrag(false);
        overlay_.hoveredDimension.reset();
        bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift);
        bool alt = input::has_flag(e.modifiers, input::Modifiers::Alt);

        std::optional<PickResult> pickRes = picker_.pickAtScreenLogical(e.x, e.y);

        if (alt) {
            state_ = State::Idle;
            return tryApplyPointOnPointNearCursor(e.x, e.y);
        }

        if (e.modifiers == input::Modifiers::None) {
            if (const auto hit = pickDimension({e.x, e.y})) {
                beginDimensionDrag(*hit, {e.x, e.y});
                return std::nullopt;
            }
        }

        if (pickRes.has_value()) {
            const auto ref = pickRes->ref;

            if (shift) {
                overlay_.selection_.model.toggle(ref);
                state_ = State::Idle;
            } else {
                if (!overlay_.selection_.model.contains(ref)) {
                    overlay_.selection_.model.replace(ref);
                }
                pressWorldPos_ = camera_.screenLogicalToWorld({e.x, e.y});
                lastPos_ = pressWorldPos_;
                const auto prepared = prepareDragSelection(overlay_.selection_.model.items());
                if (!prepared) {
                    state_ = State::Idle;
                    ActionReport report{ActionKind::Drag};
                    report.operationError = prepared.error();
                    return report;
                }
                state_ = State::DraggingSelection;
            }
        } else {
            state_ = State::MarqueeSelection;

            marqueeBaseSelection_ = overlay_.selection_.model.items();

            if (!shift) {
                overlay_.selection_.model.clear();
            }

            glm::dvec2 worldPos = camera_.screenLogicalToWorld({e.x, e.y});
            overlay_.selectionRect_ = OverlayModel::Rect(worldPos.x, worldPos.y, worldPos.x, worldPos.y);
            lastScreenPos_ = {e.x, e.y};
            pressWorldPos_ = worldPos;
        }

        return std::nullopt;
    }

    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Release) {
        if (dimensionDrag_) {
            // Use the release position even if the last mouse-move event was coalesced.
            updateDimensionDrag({e.x, e.y});
            endDimensionDrag(true);
            return std::nullopt;
        }
        return cancel().report;
    }
    return std::nullopt;
}

std::optional<ActionReport> CursorTool::onKey(const input::KeyEvent& e) {
    if (e.action != input::KeyAction::Press || e.isAutoRepeat) {
        return std::nullopt;
    }

    const auto& selected = overlay_.selection_.model.items();
    if (e.key == input::KeyCode::Delete) {
        std::set<sketch::EntityId> seen;
        std::vector<sketch::EntityId> ids;
        for (const auto& ref : selected) {
            if (seen.insert(ref.entity).second) {
                ids.push_back(ref.entity);
            }
        }
        if (ids.empty()) {
            return std::nullopt;
        }
        ActionReport report{ActionKind::Delete};
        report.requestedEntities = ids.size();
        const auto removed = sketch_.removeEntities(ids);
        if (!removed) {
            report.mutationFailed(removed.error());
            return report;
        }
        report.change = ModelChange::Changed;
        report.entityIds = std::move(ids);
        overlay_.selection_.model.clear();
        return report;
    }

    if (e.modifiers == input::Modifiers::Ctrl) {
        if (e.key == input::KeyCode::C) {
            copySelection();
            return std::nullopt;
        }
        if (e.key == input::KeyCode::V) {
            return pasteSelection();
        }
    }
    return std::nullopt;
}

void CursorTool::copySelection() {
    ClipboardFragment fragment;
    std::set<sketch::EntityId> included;
    sketch::Vec2 minimum{std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
    sketch::Vec2 maximum{-std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()};
    const auto expand = [&](sketch::Vec2 point) {
        minimum.x = std::min(minimum.x, point.x);
        minimum.y = std::min(minimum.y, point.y);
        maximum.x = std::max(maximum.x, point.x);
        maximum.y = std::max(maximum.y, point.y);
    };

    // Copy each selected owner once, including its owned endpoints/center.
    for (const auto& ref : overlay_.selection_.model.items()) {
        if (!included.insert(ref.entity).second) {
            continue;
        }
        const auto entity = sketch_.entity(ref.entity);
        const auto points = sketch_.pointElements(ref.entity);
        if (!entity || !points) {
            return;
        }
        for (const auto& point : points.value()) {
            expand(point.position);
        }
        if (const auto* circle = std::get_if<sketch::Circle2>(&entity.value().geometry)) {
            expand({circle->center.x - circle->radius, circle->center.y - circle->radius});
            expand({circle->center.x + circle->radius, circle->center.y + circle->radius});
        }
        fragment.entities.push_back(entity.value());
    }

    if (fragment.entities.empty()) {
        data_ = {};
        return;
    }
    fragment.center = {std::midpoint(minimum.x, maximum.x), std::midpoint(minimum.y, maximum.y)};

    const auto constraints = sketch_.constraints();
    if (!constraints) {
        return;
    }
    for (const auto& constraint : constraints.value()) {
        const auto& definition = constraint.definition;
        if (std::all_of(definition.refs.begin(), definition.refs.end(), [&](const sketch::GeometryRef& ref) { return included.contains(ref.entity); })) {
            fragment.constraints.push_back(definition);
        }
    }
    data_ = std::move(fragment);
}

std::optional<ActionReport> CursorTool::pasteSelection() {
    if (data_.entities.empty()) {
        return std::nullopt;
    }

    ActionReport report{ActionKind::Paste};
    report.requestedEntities = data_.entities.size();
    report.requestedConstraints = data_.constraints.size();
    const sketch::Vec2 offset{lastCursorWorldPos_.x - data_.center.x, lastCursorWorldPos_.y - data_.center.y};
    const auto translate = [offset](sketch::Vec2& point) {
        point.x += offset.x;
        point.y += offset.y;
    };
    std::vector<sketch::EntityCreation> creations;
    creations.reserve(data_.entities.size());
    for (const auto& entity : data_.entities) {
        auto geometry = entity.geometry;
        std::visit(
            [&](auto& value) {
                using Geometry = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<Geometry, sketch::Point2>) {
                    translate(value.position);
                } else if constexpr (std::is_same_v<Geometry, sketch::Line2>) {
                    translate(value.start);
                    translate(value.end);
                } else if constexpr (std::is_same_v<Geometry, sketch::Circle2>) {
                    translate(value.center);
                } else if constexpr (std::is_same_v<Geometry, sketch::Arc2>) {
                    translate(value.center);
                    translate(value.start);
                    translate(value.end);
                }
            },
            geometry);
        creations.push_back({std::move(geometry), entity.construction});
    }

    const auto added = sketch_.addEntities(creations);
    if (!added) {
        report.mutationFailed(added.error());
        return report;
    }
    report.change = ModelChange::Changed;
    report.entityIds = added.value();
    std::map<sketch::EntityId, sketch::EntityId> remapped;
    std::vector<sketch::GeometryRef> pasted;
    for (size_t i = 0; i < data_.entities.size(); ++i) {
        remapped.emplace(data_.entities[i].id, added.value()[i]);
        pasted.push_back({added.value()[i], sketch::SubElement::Whole});
    }

    overlay_.selection_.model.replace(pasted);
    std::vector<sketch::ConstraintDefinition> defs;
    for (auto definition : data_.constraints) {
        for (auto& ref : definition.refs) {
            ref.entity = remapped.at(ref.entity);
        }
        if (definition.fixedPosition) {
            translate(*definition.fixedPosition);
        }
        defs.push_back(definition);
        // const auto constraint = sketch_.addConstraint(definition);
        // if (!constraint) {
        //     report.mutationFailed(constraint.error());
        //     return report;
        // }
        // report.constraintIds.push_back(constraint.value());
    }
    sketch_.addConstraints(defs);
    const auto solved = sketch_.solve();
    report.recordSolve(solved);
    return report;
}

ToolCancellation CursorTool::cancel() {
    const bool handled = state_ != State::Idle;
    endDimensionDrag(false);
    overlay_.hoveredDimension.reset();
    overlay_.draggedDimension.reset();
    auto report = finishDrag();
    overlay_.selectionRect_.reset();
    marqueeBaseSelection_.clear();
    dragTargets_.clear();
    state_ = State::Idle;
    return {handled, std::move(report)};
}

std::optional<ActionReport> CursorTool::finishDrag() {
    auto report = std::move(dragReport_);
    dragReport_.reset();
    if (report && dragAttempted_ && dragInitialEntities_ && dragInitialConstraints_) {
        const auto entities = sketch_.entities();
        const auto constraints = sketch_.constraints();
        if (!entities || !constraints) {
            if (!report->operationError) {
                report->operationError = !entities ? entities.error() : constraints.error();
            }
            if (report->change != ModelChange::Unchanged) {
                report->change = ModelChange::PotentiallyChanged;
            }
        } else {
            const auto& initial = *dragInitialEntities_;
            const auto& current = entities.value();
            for (size_t i = 0; i < current.size(); ++i) {
                if (i >= initial.size() || current[i].id != initial[i].id || current[i].construction != initial[i].construction ||
                    !sameGeometry(current[i].geometry, initial[i].geometry)) {
                    report->entityIds.push_back(current[i].id);
                }
            }
            const auto& initialConstraints = *dragInitialConstraints_;
            const auto& currentConstraints = constraints.value();
            for (size_t i = 0; i < currentConstraints.size(); ++i) {
                if (i >= initialConstraints.size() || currentConstraints[i].id != initialConstraints[i].id ||
                    !sameConstraint(currentConstraints[i].definition, initialConstraints[i].definition)) {
                    report->constraintIds.push_back(currentConstraints[i].id);
                }
            }
            // Drag retains topology. Comparing once also catches solver movement
            // and tangency branch changes without full-model queries on every move.
            if (report->change != ModelChange::PotentiallyChanged) {
                report->change = report->entityIds.empty() && report->constraintIds.empty() && current.size() == initial.size() &&
                                         currentConstraints.size() == initialConstraints.size()
                                     ? ModelChange::Unchanged
                                     : ModelChange::Changed;
            }
        }
    }
    dragInitialEntities_.reset();
    dragInitialConstraints_.reset();
    dragAttempted_ = false;
    dragStopped_ = false;
    if (report && report->change == ModelChange::Unchanged && !report->operationError && !report->solveError &&
        (!report->solve || report->solve->status == sketch::SolveStatus::Converged)) {
        return std::nullopt;
    }
    return report;
}

sketch::Status CursorTool::prepareDragSelection(std::span<const sketch::GeometryRef> refs) {
    dragTargets_.clear();
    std::set<std::pair<sketch::EntityId, sketch::SubElement>> seen;
    std::vector<sketch::DragRequest> targets;
    for (const auto& ref : refs) {
        auto points = sketch_.pointElements(ref.entity);
        if (!points) {
            return sketch::Status::failure(points.error().code, points.error().message);
        }
        bool found = false;
        for (const auto& point : points.value()) {
            // Whole expands to all owned points; overlapping selections move each point once.
            if (ref.sub == sketch::SubElement::Whole || point.ref == ref) {
                found = true;
                if (seen.emplace(point.ref.entity, point.ref.sub).second) {
                    targets.push_back({point.ref, point.position});
                }
            }
        }
        if (!found) {
            return sketch::Status::failure(sketch::ErrorCode::InvalidArgument, "Selected sub-element does not belong to entity kind");
        }
    }
    dragTargets_ = std::move(targets);
    return sketch::Status::success();
}

sketch::Result<sketch::SolveDiagnostics> CursorTool::moveSelection(sketch::Vec2 offset) {
    auto requests = dragTargets_;
    for (auto& request : requests) {
        // Targets stay anchored to the press position even if the solver projects a previous step.
        request.target.x += offset.x;
        request.target.y += offset.y;
    }
    return sketch_.dragPoints(requests, {sketch::TangencyDragPolicy::SwitchLineSide, camera_.screenLogicalToWorld(4.0)});
}

ActionReport CursorTool::tryApplyPointOnPointNearCursor(double xLogic, double yLogic) {
    ActionReport rejected{ActionKind::ApplyConstraint};
    rejected.requestedConstraints = 1;
    const glm::dvec2 worldCursor = camera_.screenLogicalToWorld({xLogic, yLogic});
    const auto points = sketch_.pointElements();
    if (!points) {
        rejected.operationError = points.error();
        return rejected;
    }
    if (points.value().size() < 2) {
        rejected.rejection = ActionRejection::InsufficientTargets;
        return rejected;
    }
    const auto constraints = sketch_.constraints();
    if (!constraints) {
        rejected.operationError = constraints.error();
        return rejected;
    }

    const auto& elements = points.value();
    using RefKey = std::pair<sketch::EntityId, sketch::SubElement>;
    std::map<RefKey, size_t> indices;
    DSU<size_t> groups;
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& ref = elements[i].ref;
        indices.emplace(RefKey{ref.entity, ref.sub}, i);
        groups.makeSet(i);
    }
    // Public coincidence constraints define groups, including transitive links.
    for (const auto& constraint : constraints.value()) {
        const auto& definition = constraint.definition;
        if (definition.type != sketch::ConstraintType::Coincident || definition.refs.size() != 2) {
            continue;
        }
        const auto& a = definition.refs[0];
        const auto& b = definition.refs[1];
        const auto first = indices.find({a.entity, a.sub});
        const auto second = indices.find({b.entity, b.sub});
        if (first != indices.end() && second != indices.end()) {
            groups.unite(first->second, second->second);
        }
    }

    const auto distance = [&](size_t i) {
        const auto& position = elements[i].position;
        return std::hypot(position.x - worldCursor.x, position.y - worldCursor.y);
    };
    std::optional<size_t> first;
    double firstDistance = std::numeric_limits<double>::infinity();
    for (size_t i = 0; i < elements.size(); ++i) {
        const double d = distance(i);
        if (d < firstDistance) {
            first = i;
            firstDistance = d;
        }
    }
    if (!first) {
        rejected.rejection = ActionRejection::InsufficientTargets;
        return rejected;
    }

    std::optional<size_t> second;
    double secondDistance = std::numeric_limits<double>::infinity();
    for (size_t i = 0; i < elements.size(); ++i) {
        if (groups.same(*first, i)) {
            continue;
        }
        const double d = distance(i);
        if (d < secondDistance) {
            second = i;
            secondDistance = d;
        }
    }
    if (!second) {
        rejected.rejection = ActionRejection::InsufficientTargets;
        return rejected;
    }

    const std::array refs{elements[*first].ref, elements[*second].ref};
    const auto prepared = constraintActions_.prepare({ConstraintAction::Coincident, std::nullopt}, refs);
    if (prepared.state != ConstraintPreparation::State::Ready || !prepared.definition) {
        return ConstraintActions::rejectedReport(prepared);
    }
    return constraintActions_.apply(*prepared.definition);
}
