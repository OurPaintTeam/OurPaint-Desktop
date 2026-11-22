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

#include "../render/constraints/ConstraintLayout.h"
#include "DSU.h"
#include "Document.h"

using namespace app;

namespace {
bool canSelectDimension(input::Modifiers modifiers) {
    return !input::has_flag(modifiers, input::Modifiers::Alt) && !input::has_flag(modifiers, input::Modifiers::Meta);
}
}  // namespace

CursorTool::CursorTool(Document& document,
                       render::Camera2D& camera,
                       snap::SnapSystem& snap,
                       Cpu2dPicker& picker,
                       OverlayModel& overlay,
                       const ConstraintLayout& constraintLayout,
                       ConstraintPresentationState& constraintPresentation)
    : IInteractionTool(document, camera, snap, overlay, picker),
      sketch_(document.sketch()),
      constraintLayout_(constraintLayout),
      constraintPresentation_(constraintPresentation) {}

void CursorTool::onMouseMove(const input::MouseMoveEvent& e) {
    snapVisualClear();
    const glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;
    lastLogicalPos_ = {e.x, e.y};

    if (dimensionDrag_) {
        if (input::has_flag(e.buttons, input::MouseButton::Left)) {
            updateDimensionDrag({e.x, e.y});
        } else {
            // Recover if mouse capture was lost and no release event reached the tool.
            endDimensionDrag(false);
        }
        return;
    }
    overlay_.hoveredDimension.reset();
    if (state_ == State::Idle && e.buttons == input::MouseButton::None && canSelectDimension(e.modifiers)) {
        if (const auto dimension = picker_.pickSelectableDimensionAtScreenLogical(e.x, e.y, constraintLayout_)) {
            overlay_.hoveredDimension = dimension->constraint;
        }
    }

    if (input::has_flag(e.buttons, input::MouseButton::Left)) {

        // Dragging selection items
        if (state_ == State::DraggingSelection) {
            const auto& refs = dragGeometry_;
            if (!refs.empty() && !dragStopped_ && (v.x != lastPos_.x || v.y != lastPos_.y)) {
                auto constraints = sketch_.constraints();

                // Check: first selected entity is circle
                auto result = sketch_.entity(refs[0].entity);
                if (!result) {
                    dragStopped_ = true;
                    return;
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

                // If this is just one circle then change radius
                if (refs.size() == 1 && refs[0].sub == sketch::SubElement::Whole && c) {
                    const double radius = std::hypot(v.x - circle->center.x, v.y - circle->center.y);
                    if (radius == circle->radius) {
                        lastPos_ = v;
                        return;
                    }
                    dragAttempted_ = true;
                    const auto updated = sketch_.updateCircleRadius(refs[0].entity, radius);
                    if (!updated) {
                        dragStopped_ = true;
                        return;
                    }
                    const auto solved = sketch_.solve();
                    if (!solved || solved.value().solveStatus != sketch::SolveStatus::Converged) {
                        dragStopped_ = true;
                        return;
                    }
                }

                // Else move selection items
                else {
                    dragAttempted_ = true;
                    const auto dragged = moveSelection(offset);
                    if (!dragged) {
                        dragStopped_ = true;
                        return;
                    }
                    if (dragged.value().solveStatus != sketch::SolveStatus::Converged) {
                        //dragStopped_ = true;
                        //return;
                    }
                }
                lastPos_ = v;
            }
        }

        // Rectangular Marquee
        else if (state_ == State::MarqueeSelection) {
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

    if (e.modifiers == input::Modifiers::Alt) {
        overlay_.points_.clear();
        const auto& pair = findTwoPointsNearCursor(lastLogicalPos_.x, lastLogicalPos_.y);
        if (pair) {
            previewTwoPoints(pair.value().first, pair.value().second);
        }
    }
}

void CursorTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        endDimensionDrag(false);
        overlay_.hoveredDimension.reset();
        const bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift) || input::has_flag(e.modifiers, input::Modifiers::Ctrl);
        const bool alt = input::has_flag(e.modifiers, input::Modifiers::Alt);

        // Add point-on-point constraint for near points
        if (alt) {
            state_ = State::Idle;
            overlay_.points_.clear();
            const auto& pair = findTwoPointsNearCursor(e.x, e.y);
            if (pair) {
                applyPointOnPointConstraint(pair.value().first, pair.value().second);
                sketch_.pushState(); // FOR UNDO/REDO
                return;
            }
        }

        const auto hit = canSelectDimension(e.modifiers) ? picker_.pickAtScreenLogical(e.x, e.y, constraintLayout_) : picker_.pickGeometryAtScreenLogical(e.x, e.y);
        if (hit) {
            const auto* dimension = std::get_if<DimensionPickResult>(&*hit);
            const SelectionItem item = dimension ? SelectionItem{dimension->constraint} : SelectionItem{std::get<GeometryPickResult>(*hit).ref};
            auto& selection = overlay_.selection_.model;
            if (shift) {
                selection.toggle(item);
                state_ = State::Idle;
                return;
            }
            if (!selection.contains(item)) {
                selection.replace(item);
            }
            if (dimension) {
                beginDimensionDrag(*dimension, {e.x, e.y});
                return;
            }
            pressWorldPos_ = camera_.screenLogicalToWorld({e.x, e.y});
            lastPos_ = pressWorldPos_;
            dragGeometry_ = selection.geometryItems();
            state_ = prepareDragSelection(dragGeometry_) ? State::DraggingSelection : State::Idle;
        }
        // nothing selected -> make a marquee rectangle
        else {
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

        return;
    }

    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Release) {
        if (dimensionDrag_) {
            // Use the release position even if the last mouse-move event was coalesced.
            updateDimensionDrag({e.x, e.y});
            endDimensionDrag(true);
        }

        if (state_ == State::DraggingSelection) {
            document_.sketch().pushState(); // FOR UNDO/REDO
        }

        cancel();
    }
}

void CursorTool::onKey(const input::KeyEvent& e) {
    // Delete
    if (e.action == input::KeyAction::Press && e.key == input::KeyCode::Delete) {
        deleteSelection();
        return;
    }

    // ctrl+C/ctrl+V
    if (e.action == input::KeyAction::Press && e.modifiers == input::Modifiers::Ctrl) {
        if (e.key == input::KeyCode::C) {
            copySelection();
        } else if (e.key == input::KeyCode::V) {
            pasteSelection();
            document_.sketch().pushState(); // FOR UNDO/REDO
            return;
        }
    }

    // alt (Coincident)
    if (e.action == input::KeyAction::Press && e.modifiers == input::Modifiers::Alt) {
        overlay_.points_.clear();

        const auto& pair = findTwoPointsNearCursor(lastLogicalPos_.x, lastLogicalPos_.y);
        if (pair) {
            previewTwoPoints(pair.value().first, pair.value().second);
        }
    }

    if (e.action == input::KeyAction::Release) {
        overlay_.points_.clear();
    }
}

bool CursorTool::cancel() {
    endDimensionDrag(false);
    overlay_.hoveredDimension.reset();
    overlay_.draggedDimension.reset();
    overlay_.selectionRect_.reset();
    marqueeBaseSelection_.clear();
    dragGeometry_.clear();
    dragTargets_.clear();
    state_ = State::Idle;
    return false;
}

void CursorTool::beginDimensionDrag(const DimensionPickResult& hit, glm::dvec2 position) {
    if (!sketch_.constraint(hit.constraint)) {
        return;
    }
    const auto found = constraintPresentation_.labelPlacements.find(hit.constraint);
    const auto initial = found == constraintPresentation_.labelPlacements.end() ? std::nullopt : std::optional(found->second);
    dimensionDrag_ = DimensionDrag{hit.constraint, position, hit.labelCenter - position, initial};
    state_ = State::PressedDimension;
    overlay_.hoveredDimension = hit.constraint;
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
    overlay_.draggedDimension = dimensionDrag_->constraint;
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

void CursorTool::deleteSelection() {
    auto& selection = overlay_.selection_.model;
    if (selection.empty()) {
        return;
    }
    cancel();
    const auto before = sketch_.snapshot();
    if (!before) {
        return;
    }

    bool changed = false;
    for (const auto& item : selection.items()) {
        const auto* ref = std::get_if<sketch::GeometryRef>(&item);
        const auto removed = ref ? sketch_.removeEntity(ref->entity) : sketch_.removeConstraint(std::get<sketch::ConstraintId>(item));
        // Removing an owner can invalidate later sub-elements and dependent dimensions in this selection.
        if (removed) {
            changed = true;
        } else if (removed.error().code != sketch::ErrorCode::NotFound) {
            // An earlier removal may have succeeded before the backend failed.
            if (const auto restored = sketch_.replaceState(before.value()); !restored) {
                std::cerr << "Could not restore the sketch after a failed deletion: " << restored.error().message << '\n';
            }
            return;
        }
    }
    selection.clear();
    if (changed) {
        sketch_.pushState();
    }
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
    for (const auto& ref : overlay_.selection_.model.geometryItems()) {
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

void CursorTool::pasteSelection() {
    if (data_.entities.empty()) {
        return;
    }

    //ActionReport report{ActionKind::Paste};
    //report.requestedEntities = data_.entities.size();
    //report.requestedConstraints = data_.constraints.size();
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
    // if (!added) {
    //     report.mutationFailed(added.error());
    //     return report;
    // }
    //report.change = ModelChange::Changed;
    //report.entityIds = added.value();
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
    //const auto solved = sketch_.solve();
    //report.recordSolve(solved);
    //return report;
}

sketch::Status CursorTool::prepareDragSelection(std::span<const sketch::GeometryRef> refs) {
    dragTargets_.clear();
    std::set<sketch::GeometryRef> seen;
    std::vector<sketch::DragRequest> targets;
    for (const auto& ref : refs) {
        auto points = sketch_.pointElements(ref.entity);
        if (!points) {
            return sketch::Status::failure(points.error().code, points.error().message);
        }
        bool found = false;
        for (const auto& point : points.value()) {
            // Whole expands to all owned points; overlapping selections move each point once.
            if (ref == sketch::GeometryRef{ref.entity, sketch::SubElement::Whole} || point.ref == ref) {
                found = true;
                if (seen.insert(point.ref).second) {
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

std::optional<std::pair<sketch::GeometryRef, sketch::GeometryRef>> CursorTool::findTwoPointsNearCursor(double xLogic, double yLogic) {
    const glm::dvec2 worldCursor = camera_.screenLogicalToWorld({xLogic, yLogic});
    const auto points = sketch_.pointElements();
    if (!points) {
        return std::nullopt;
    }
    if (points.value().size() < 2) {
        return std::nullopt;
    }
    const auto constraints = sketch_.constraints();
    if (!constraints) {
        return std::nullopt;
    }

    const auto& elements = points.value();
    std::map<sketch::GeometryRef, size_t> indices;
    DSU<size_t> groups;
    for (size_t i = 0; i < elements.size(); ++i) {
        const auto& ref = elements[i].ref;
        indices.emplace(ref, i);
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
        const auto first = indices.find(a);
        const auto second = indices.find(b);
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
        return std::nullopt;
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
        return std::nullopt;
    }

    return std::make_pair(elements[*first].ref, elements[*second].ref);
}

void CursorTool::applyPointOnPointConstraint(sketch::GeometryRef p1, sketch::GeometryRef p2) {
    sketch::ConstraintDefinition def;
    def.type = sketch::ConstraintType::Coincident;
    def.refs = {p1, p2};

    const auto first = sketch_.pointPosition(p1);
    const auto second = sketch_.pointPosition(p2);
    if (!first || !second) {
        return;
    }
    const sketch::Vec2 position =
        sketch::Vec2{std::midpoint(first.value().x, second.value().x), std::midpoint(first.value().y, second.value().y)};
    const sketch::PointTarget target{p1, position};
    const sketch::SolveOptions options{std::span(&target, 1), sketch::TargetPolicy::Prefer};
    auto result = sketch_.addConstraint(def, options);
    if (!result) {
        return;
    }
    sketch_.solve();
}

void CursorTool::previewTwoPoints(sketch::GeometryRef p1, sketch::GeometryRef p2) {
    const auto result1 = sketch_.pointPosition(p1);
    const auto result2 = sketch_.pointPosition(p2);
    if (result1 && result2) {
        const auto pushPoint = [&](double x, double y) {
            OverlayModel::Point p;
            p.type = OverlayModel::OverlayType::ConstraintHoverToObject;
            p.x = x;
            p.y = y;
            overlay_.points_.push_back(p);
        };

        pushPoint(result1.value().x, result1.value().y);
        pushPoint(result2.value().x, result2.value().y);
    }
}
