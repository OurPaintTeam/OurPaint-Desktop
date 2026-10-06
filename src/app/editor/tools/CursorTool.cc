#include "CursorTool.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <set>
#include <type_traits>

#include "../ConstraintActions.h"
#include "DSU.h"
#include "Document.h"

CursorTool::CursorTool(Document& document, Camera2D& camera, Cpu2dPicker& picker, OverlayModel& overlay, ConstraintActions& constraintActions)
    : document_(document), sketch_(document.sketch()), camera_(camera), picker_(picker), overlay_(overlay), constraintActions_(constraintActions), data_() {}

void CursorTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});

    if (input::has_flag(e.buttons, input::MouseButton::Left)) {
        if (state_ == State::DraggingSelection) {
            const auto& refs = overlay_.selection_.model.items();
            if (!refs.empty()) {
                auto result = sketch_.entity(refs[0].entity);
                if (!result) {
                    return;
                }
                const auto& geometry = result.value().geometry;
                const auto* circle = std::get_if<sketch::Circle2>(&geometry);
                const sketch::Vec2 offset{v.x - pressWorldPos_.x, v.y - pressWorldPos_.y};
                if (refs.size() == 1 && refs[0].sub == sketch::SubElement::Whole && circle) {
                    const double radius = std::hypot(v.x - circle->center.x, v.y - circle->center.y);
                    if (!sketch_.updateCircleRadius(refs[0].entity, radius)) {
                        return;
                    }
                    const auto solved = sketch_.solve();
                    if (!solved || solved.value().status != sketch::SolveStatus::Converged) {
                        return;
                    }
                } else {
                    const auto dragged = moveSelection(offset);
                    if (!dragged || dragged.value().status != sketch::SolveStatus::Converged) {
                        return;
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
    lastCursorWorldPos_ = v;
}

void CursorTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        bool shift = input::has_flag(e.modifiers, input::Modifiers::Shift);
        bool alt = input::has_flag(e.modifiers, input::Modifiers::Alt);

        std::optional<PickResult> pickRes = picker_.pickAtScreenLogical(e.x, e.y);

        if (alt) {
            if (tryApplyPointOnPointNearCursor(e.x, e.y)) {
                state_ = State::Idle;
            }
            return;
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
                if (!prepareDragSelection(overlay_.selection_.model.items())) {
                    state_ = State::Idle;
                    return;
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

        return;
    }

    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Release) {
        overlay_.selectionRect_.reset();
        marqueeBaseSelection_.clear();
        dragTargets_.clear();
        state_ = State::Idle;
    }
}

void CursorTool::onKey(const input::KeyEvent& e) {
    if (e.action != input::KeyAction::Press) {
        return;
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
        if (!sketch_.removeEntities(ids)) {
            return;
        }
        cancel();
        overlay_.selection_.model.clear();
        return;
    }

    if (e.modifiers == input::Modifiers::Ctrl) {
        if (e.key == input::KeyCode::C) {
            copySelection();
            return;
        }
        if (e.key == input::KeyCode::V) {
            pasteSelection();
            return;
        }
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

void CursorTool::pasteSelection() {
    if (data_.entities.empty()) {
        return;
    }

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
        return;
    }
    std::map<sketch::EntityId, sketch::EntityId> remapped;
    std::vector<sketch::GeometryRef> pasted;
    for (size_t i = 0; i < data_.entities.size(); ++i) {
        remapped.emplace(data_.entities[i].id, added.value()[i]);
        pasted.push_back({added.value()[i], sketch::SubElement::Whole});
    }

    cancel();
    overlay_.selection_.model.replace(pasted);
    for (auto definition : data_.constraints) {
        for (auto& ref : definition.refs) {
            ref.entity = remapped.at(ref.entity);
        }
        if (definition.fixedPosition) {
            translate(*definition.fixedPosition);
        }
        if (!sketch_.addConstraint(definition)) {
            return;
        }
    }
    const auto solved = sketch_.solve();
    if (!solved || solved.value().status != sketch::SolveStatus::Converged) {
        return;
    }
}

bool CursorTool::cancel() {
    // overlay_.selection_.model.clear(); // need for Dimension tool maybe
    overlay_.selectionRect_.reset();
    marqueeBaseSelection_.clear();
    dragTargets_.clear();
    state_ = State::Idle;
    return true;
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
    return sketch_.dragPoints(requests);
}

bool CursorTool::tryApplyPointOnPointNearCursor(double xLogic, double yLogic) {
    const glm::dvec2 worldCursor = camera_.screenLogicalToWorld({xLogic, yLogic});
    const auto points = sketch_.pointElements();
    if (!points || points.value().size() < 2) {
        return false;
    }
    const auto constraints = sketch_.constraints();
    if (!constraints) {
        return false;
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
        return false;
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
        return false;
    }

    const std::array refs{elements[*first].ref, elements[*second].ref};
    const auto prepared = constraintActions_.prepare({ConstraintAction::Coincident, std::nullopt}, refs);
    return prepared.state == ConstraintPreparation::State::Ready && prepared.definition && constraintActions_.apply(*prepared.definition);
}
