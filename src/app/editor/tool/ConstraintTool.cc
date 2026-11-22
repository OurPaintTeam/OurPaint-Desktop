#include "ConstraintTool.h"

#include <iostream>
#include <numeric>
#include <optional>

#include "../pick/Cpu2dPicker.h"
#include "../OverlayModel.h"
#include "Document.h"

ConstraintTool::ConstraintTool(Document& document,
                               render::Camera2D& camera,
                               snap::SnapSystem& snap,
                               OverlayModel& overlay,
                               Cpu2dPicker& picker)
    : IInteractionTool(document, camera, snap, overlay, picker), subTool_(), sketch_(document_.sketch()) {}

void ConstraintTool::onMouseMove(const input::MouseMoveEvent& e) {
    overlay_.points_.clear();
    overlay_.lines_.clear();
    overlay_.circles_.clear();
    overlay_.arcs_.clear();

    // Highlighting entities that already in refs_
    for (const auto& ref : refs_) {
        auto result = sketch_.entity(ref.entity);
        if (!result) {
            return;
        }
        pushEntityToOverlay(ref);
    }

    // Highlighting entities that can be constraint
    switch (subTool_) {
        case ToolId::Distance: {
            if (refs_.empty()) {
                auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                }
                pushEntityToOverlay(pick.value().ref);
            } else if (refs_.size() == 1) {
                auto result = sketch_.entity(refs_[0].entity);
                if (!result) {
                    return;
                }
                auto& ent = result.value();

                bool firstIsPoint = std::holds_alternative<sketch::Point2>(ent.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent.geometry)  && refs_[0].sub == sketch::SubElement::Whole;

                if (firstIsPoint) {
                    auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                    pushEntityToOverlay(pick.value().ref);
                } else if (firstIsLine) {
                    auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                }
            }
            break;
        }
        case ToolId::HorizontalDistance:
        case ToolId::VerticalDistance: {
            auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
            if (!pick) {
                return;
            }
            const auto position = sketch_.pointPosition(pick->ref);
            if (position) {
                pushPointToOverlay(position.value().x, position.value().y);
            }
            break;
        }
        case ToolId::Coincident: {
            std::optional<GeometryPickResult> pick;
            if (refs_.empty()) {
                pick = picker_.pickGeometryAtScreenLogical(e.x, e.y);
            } else {
                auto result = sketch_.entity(refs_[0].entity);
                if (!result) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
                auto& ent = result.value();
                if (std::holds_alternative<sketch::Point2>(ent.geometry) || refs_[0].sub != sketch::SubElement::Whole) {
                    pick = picker_.pickGeometryAtScreenLogical(e.x, e.y);
                } else {
                    pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                }
            }
            if (!pick) {
                return;
            }
            pushEntityToOverlay(pick.value().ref);
            break;
        }
        case ToolId::Horizontal:
        case ToolId::Vertical:
        case ToolId::Angle:
        case ToolId::Collinear:
        case ToolId::Parallel:
        case ToolId::Perpendicular: {
            auto pick = picker_.pickLineAtScreenLogical(e.x, e.y);
            if (!pick) {
                return;
            }
            pushEntityToOverlay(pick.value().ref);
            break;
        }
        case ToolId::Tangent: {
            if (refs_.empty()) {
                auto pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                }
                pushEntityToOverlay(pick.value().ref);
            } else if (refs_.size() == 1) {
                auto result = sketch_.entity(refs_[0].entity);
                if (!result) {
                    return;
                }
                auto& ent = result.value();

                bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent.geometry);
                bool firstIsLine = std::holds_alternative<sketch::Line2>(ent.geometry);

                if (firstIsLine) {
                    auto pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                } else if (firstIsCircle) {
                    auto pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                    pushEntityToOverlay(pick.value().ref);
                }
            }
            break;
        }
        case ToolId::Equal: {
            if (refs_.empty()) {
                auto pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                }
                pushEntityToOverlay(pick.value().ref);
            } else if (refs_.size() == 1) {
                auto result = sketch_.entity(refs_[0].entity);
                if (!result) {
                    return;
                }
                auto& ent = result.value();

                bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent.geometry);
                bool firstIsLine = std::holds_alternative<sketch::Line2>(ent.geometry);

                if (firstIsLine) {
                    auto pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                } else if (firstIsCircle) {
                    auto pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                }
            }
            break;
        }
        case ToolId::Fix: {
            auto pick = picker_.pickGeometryAtScreenLogical(e.x, e.y);
            if (!pick) {
                return;
            }
            pushEntityToOverlay(pick.value().ref);
            break;
        }
        case ToolId::Concentric: {
            if (refs_.empty()) {
                auto pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickArcAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                }
                pushEntityToOverlay(pick.value().ref);
            } else if (refs_.size() == 1) {
                auto result = sketch_.entity(refs_[0].entity);
                if (!result) {
                    return;
                }
                auto& ent = result.value();

                bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent.geometry);
                bool firstIsArc = std::holds_alternative<sketch::Arc2>(ent.geometry);

                if (firstIsCircle || firstIsArc) {
                    auto pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickArcAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                    pushEntityToOverlay(pick.value().ref);
                }
            }
            break;
        }
        case ToolId::Midpoint: {
            if (refs_.empty()) {
                auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickArcAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                }
                pushEntityToOverlay(pick.value().ref);
            } else if (refs_.size() == 1) {
                auto result = sketch_.entity(refs_[0].entity);
                if (!result) {
                    return;
                }
                auto& ent = result.value();

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent.geometry);
                bool firstIsArc  = std::holds_alternative<sketch::Arc2>(ent.geometry);

                // point arc
                // arc point
                // point line
                // line point
                // line line
                if (firstIsPoint) {
                    auto pick = picker_.pickArcAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                    pushEntityToOverlay(pick.value().ref);
                } else if (firstIsLine) {
                    auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                    pushEntityToOverlay(pick.value().ref);
                } else if (firstIsArc) {
                    auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                }
            }
            break;
        }
        case ToolId::Symmetric: {
            if (refs_.empty()) {
                auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                }
                pushEntityToOverlay(pick.value().ref);
            } else if (refs_.size() == 1) {
                auto result = sketch_.entity(refs_[0].entity);
                if (!result) {
                    return;
                }
                auto& ent = result.value();

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent.geometry);

                if (firstIsPoint) {
                    auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                    pushEntityToOverlay(pick.value().ref);
                } else if (firstIsLine) {
                    auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                }
            } else if (refs_.size() == 2) {
                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent1.geometry);

                bool secondIsPoint  = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);

                if (firstIsPoint && secondIsPoint) {
                    auto pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                } else if ((firstIsPoint && secondIsLine) || (firstIsLine && secondIsPoint)) {
                    auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                    pushEntityToOverlay(pick.value().ref);
                }
            }
            break;
        }
        case ToolId::Curvature: {
            return;
        }
        default: {
            return;
        }
    }
}

void ConstraintTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button != input::MouseButton::Left || e.action != input::MouseButtonAction::Press) {
        return;
    }

    using sketch::ConstraintType;
    std::vector<sketch::ConstraintDefinition> definitions;
    sketch::PointTarget pointTarget;
    sketch::SolveOptions options{};
    const auto selectedGeometry = overlay_.selection_.model.geometryItems();
    const std::optional<double> dimension = isValueSet_ ? std::optional{value_} : std::nullopt;

    switch (subTool_) {
        case ToolId::Distance: {
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }

                // Circle radius case or one line case
                if (refs_.size() == 1) {
                    auto result = sketch_.entity(refs_[0].entity);
                    if (!result) {
                        refs_.clear();
                        overlay_.selection_.model.clear();
                        return;
                    }
                    auto& ent = result.value();
                    if (std::holds_alternative<sketch::Circle2>(ent.geometry) && refs_[0].sub == sketch::SubElement::Whole) {
                        definitions.push_back({.type = ConstraintType::Radius, .refs = refs_, .value = dimension});
                        break;
                    }
                    if (std::holds_alternative<sketch::Line2>(ent.geometry) && refs_[0].sub == sketch::SubElement::Whole) {
                        definitions.push_back({.type = ConstraintType::Length, .refs = refs_, .value = dimension});
                        break;
                    }
                }

                // Lines and points cases
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);

                    auto result1 = sketch_.entity(refs_[0].entity);
                    if (!result1) {
                        refs_.clear();
                        overlay_.selection_.model.clear();
                        return;
                    }

                    auto result2 = sketch_.entity(refs_[1].entity);
                    if (!result2) {
                        refs_.clear();
                        overlay_.selection_.model.clear();
                        return;
                    }

                    auto& ent1 = result1.value();
                    auto& ent2 = result2.value();

                    bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                    bool firstIsLine   = std::holds_alternative<sketch::Line2>(ent1.geometry);
                    bool secondIsPoint = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                    bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);

                    if (!firstIsPoint || !secondIsPoint) { // case: point line distance where point is subpoint this line
                        if (ent1.id == ent2.id) {
                            refs_.pop_back();
                            return;
                        }
                    }

                    if ((firstIsPoint && secondIsPoint) ||
                        (firstIsPoint && secondIsLine) ||
                        (firstIsLine && secondIsPoint)) {
                        definitions.push_back({.type = ConstraintType::Distance, .refs = refs_, .value = dimension});
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry; // should be two lines

                // Circle radius case
                if (refs_.size() == 1) {
                    auto result = sketch_.entity(refs_[0].entity);
                    if (!result) {
                        refs_.clear();
                        overlay_.selection_.model.clear();
                        return;
                    }
                    auto& ent = result.value();
                    if (std::holds_alternative<sketch::Circle2>(ent.geometry) && refs_[0].sub == sketch::SubElement::Whole) {
                        definitions.push_back({.type = ConstraintType::Radius, .refs = refs_, .value = dimension});
                        break;
                    }
                }

                // Points and lines
                if (refs_.size() != 2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool firstIsLine   = std::holds_alternative<sketch::Line2>(ent1.geometry);
                bool secondIsPoint = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);

                if ((firstIsPoint && secondIsPoint) ||
                    (firstIsPoint && secondIsLine) ||
                    (firstIsLine && secondIsPoint)) {
                    definitions.push_back({.type = ConstraintType::Distance, .refs = refs_, .value = dimension});
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
            }
            break;
        }
        case ToolId::HorizontalDistance:
        case ToolId::VerticalDistance: {
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                if (!pick) {
                    return;
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry;
                if (refs_.size() != 2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool secondIsPoint = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;

                if (!firstIsPoint || !secondIsPoint) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
            }

            const auto type = subTool_ == ToolId::HorizontalDistance ? ConstraintType::HorizontalDistance : ConstraintType::VerticalDistance;
            definitions.push_back({.type = type, .refs = refs_, .value = dimension});
            break;
        }
        case ToolId::Coincident: {
            const bool fromSelection = !selectedGeometry.empty();
            // Pick entities
            if (!fromSelection) {
                std::optional<GeometryPickResult> pick;
                if (refs_.empty()) { // pick any entity
                    pick = picker_.pickGeometryAtScreenLogical(e.x, e.y);
                } else {
                    auto result = sketch_.entity(refs_[0].entity);
                    if (!result) {
                        refs_.clear();
                        overlay_.selection_.model.clear();
                        return;
                    }
                    auto& ent = result.value();
                    if (std::holds_alternative<sketch::Point2>(ent.geometry) ||
                        refs_[0].sub != sketch::SubElement::Whole) { // first is a point -> pick any entity
                        pick = picker_.pickGeometryAtScreenLogical(e.x, e.y);
                    } else { // first entity isn't point -> pick point
                        pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                    }
                }

                if (!pick) {
                    return;
                }

                if (refs_.empty()) {
                    refs_.push_back(pick->ref);
                    return;
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                refs_.push_back(pick->ref);
            }
            // Selection entities
            else {
                refs_ = selectedGeometry;
                if (refs_.size() != 2 || refs_[0] == refs_[1]) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
            }

            const auto first = sketch_.pointPosition(refs_[0]);
            const auto second = sketch_.pointPosition(refs_[1]);
            if (first && second) {
                auto position = second.value();
                if (fromSelection) {
                    position = {std::midpoint(first.value().x, position.x), std::midpoint(first.value().y, position.y)};
                }
                pointTarget = {refs_[0], position};
                options = {std::span(&pointTarget, 1), sketch::TargetPolicy::Prefer};
            }
            definitions.push_back({.type = ConstraintType::Coincident, .refs = refs_});
            break;
        }
        case ToolId::Horizontal:
        case ToolId::Vertical: {
            std::vector<sketch::GeometryRef> refs; // should be lines
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                if (!pick) {
                    return;
                }
                refs.push_back(pick.value().ref); // add just one ref
            }
            // Selection entities
            else {
                refs = selectedGeometry; // can be more than 1 line
            }

            const auto type = subTool_ == ToolId::Horizontal ? ConstraintType::Horizontal : ConstraintType::Vertical;
            definitions.reserve(refs.size());
            for (const auto& ref : refs) {
                const auto result = sketch_.entity(ref.entity);
                if (!result || ref.sub != sketch::SubElement::Whole || !std::holds_alternative<sketch::Line2>(result.value().geometry)) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
                definitions.push_back({.type = type, .refs = {ref}});
            }
            break;
        }
        case ToolId::Angle:
        case ToolId::Collinear:
        case ToolId::Parallel:
        case ToolId::Perpendicular: {
            if (subTool_ == ToolId::Angle && !dimension) {
                return;
            }

            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                if (!pick) {
                    return;
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry; // should be two lines
                if (refs_.size() != 2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                if (!std::holds_alternative<sketch::Line2>(ent1.geometry) || !std::holds_alternative<sketch::Line2>(ent2.geometry)) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
            }

            switch (subTool_) {
                case ToolId::Angle:
                    definitions.push_back({.type = ConstraintType::Angle, .refs = refs_, .value = dimension});
                    break;
                case ToolId::Collinear:
                    definitions.push_back({.type = ConstraintType::Collinear, .refs = refs_});
                    break;
                case ToolId::Parallel:
                    definitions.push_back({.type = ConstraintType::Parallel, .refs = refs_});
                    break;
                case ToolId::Perpendicular:
                    definitions.push_back({.type = ConstraintType::Perpendicular, .refs = refs_});
                    break;
                default:
                    break;
            }
            break;
        }
        case ToolId::Tangent: {
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickArcAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);

                    auto result1 = sketch_.entity(refs_[0].entity);
                    if (!result1) {
                        return;
                    }

                    auto result2 = sketch_.entity(refs_[1].entity);
                    if (!result2) {
                        return;
                    }

                    auto& ent1 = result1.value();
                    auto& ent2 = result2.value();

                    bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent1.geometry) || std::holds_alternative<sketch::Arc2>(ent1.geometry);
                    bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent2.geometry) || std::holds_alternative<sketch::Arc2>(ent2.geometry);
                    bool firstIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                    bool secondIsLine = std::holds_alternative<sketch::Line2>(ent2.geometry);

                    if ((firstIsCircle && secondIsCircle) || (firstIsLine && secondIsCircle) || (firstIsCircle && secondIsLine)) {
                        definitions.push_back({.type = ConstraintType::Tangent, .refs = refs_});
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry; // should be two circles or line+circle
                if (refs_.size() != 2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent1.geometry) || std::holds_alternative<sketch::Arc2>(ent1.geometry);
                bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent2.geometry) || std::holds_alternative<sketch::Arc2>(ent2.geometry);
                bool firstIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                bool secondIsLine = std::holds_alternative<sketch::Line2>(ent2.geometry);

                if ((firstIsCircle && secondIsCircle) || (firstIsLine && secondIsCircle) || (firstIsCircle && secondIsLine)) {
                    definitions.push_back({.type = ConstraintType::Tangent, .refs = refs_});
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }
            break;
        }
        case ToolId::Equal: {
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);

                    auto result1 = sketch_.entity(refs_[0].entity);
                    if (!result1) {
                        return;
                    }

                    auto result2 = sketch_.entity(refs_[1].entity);
                    if (!result2) {
                        return;
                    }

                    auto& ent1 = result1.value();
                    auto& ent2 = result2.value();

                    bool firstIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                    bool secondIsLine = std::holds_alternative<sketch::Line2>(ent2.geometry);
                    bool firstIsCircle = std::holds_alternative<sketch::Circle2>(ent1.geometry);
                    bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent2.geometry);

                    if ((firstIsLine && secondIsLine) || (firstIsCircle && secondIsCircle)) {
                        definitions.push_back({.type = ConstraintType::Equal, .refs = refs_});
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry; // should be two lines
                if (refs_.size() != 2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                bool firstIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                bool secondIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                bool firstIsCircle = std::holds_alternative<sketch::Circle2>(ent1.geometry);
                bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent1.geometry);

                if ((firstIsLine && secondIsLine) || (firstIsCircle && secondIsCircle)) {
                    definitions.push_back({.type = ConstraintType::Equal, .refs = refs_});
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }
            break;
        }
        case ToolId::Fix: {
            std::vector<sketch::GeometryRef> refs;
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickGeometryAtScreenLogical(e.x, e.y);
                if (!pick) {
                    return;
                }
                refs.push_back(pick.value().ref); // add just one ref
            }
            // Selection entities
            else {
                refs = selectedGeometry; // can be more than 1 line
            }

            definitions.reserve(refs.size());
            for (const auto& ref : refs) {
                definitions.push_back({.type = ConstraintType::Fix, .refs = {ref}});
            }
            break;
        }
        case ToolId::Concentric: {
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickCircleAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickArcAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);

                    auto result1 = sketch_.entity(refs_[0].entity);
                    if (!result1) {
                        return;
                    }

                    auto result2 = sketch_.entity(refs_[1].entity);
                    if (!result2) {
                        return;
                    }

                    auto& ent1 = result1.value();
                    auto& ent2 = result2.value();

                    bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent1.geometry);
                    bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent2.geometry);
                    bool firstIsArc = std::holds_alternative<sketch::Arc2>(ent1.geometry);
                    bool secondIsArc = std::holds_alternative<sketch::Arc2>(ent2.geometry);

                    if ((firstIsCircle && secondIsCircle) || (firstIsArc && secondIsCircle) || (firstIsCircle && secondIsArc) || (firstIsArc && secondIsArc)) {
                        definitions.push_back({.type = ConstraintType::Concentric, .refs = refs_});
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry; // first is a circle/arc, second is a circle/arc
                if (refs_.size() != 2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent1.geometry);
                bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent2.geometry);
                bool firstIsArc = std::holds_alternative<sketch::Arc2>(ent1.geometry);
                bool secondIsArc = std::holds_alternative<sketch::Arc2>(ent2.geometry);

                if ((firstIsCircle && secondIsCircle) || (firstIsArc && secondIsCircle) || (firstIsCircle && secondIsArc) || (firstIsArc && secondIsArc)) {
                    definitions.push_back({.type = ConstraintType::Concentric, .refs = refs_});
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }
            break;
        }
        case ToolId::Midpoint: {
            // point arc
            // point line
            // line point
            // line line
            // arc point

            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        pick = picker_.pickArcAtScreenLogical(e.x, e.y);
                        if (!pick) {
                            return;
                        }
                    }
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);

                    auto result1 = sketch_.entity(refs_[0].entity);
                    if (!result1) {
                        return;
                    }

                    auto result2 = sketch_.entity(refs_[1].entity);
                    if (!result2) {
                        return;
                    }

                    auto& ent1 = result1.value();
                    auto& ent2 = result2.value();

                    bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                    bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent1.geometry);
                    bool firstIsArc  = std::holds_alternative<sketch::Arc2>(ent1.geometry);

                    bool secondIsPoint  = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                    bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);
                    bool secondIsArc  = std::holds_alternative<sketch::Arc2>(ent2.geometry);

                    if ((firstIsPoint && secondIsArc)   ||
                        (firstIsArc   && secondIsPoint) ||
                        (firstIsPoint && secondIsLine)  ||
                        (firstIsLine  && secondIsPoint) ||
                        (firstIsLine  && secondIsLine)) {
                        definitions.push_back({.type = ConstraintType::Midpoint, .refs = refs_});
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry;
                if (refs_.size() != 2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto& ent1 = result1.value();
                auto& ent2 = result2.value();

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent1.geometry);
                bool firstIsArc  = std::holds_alternative<sketch::Arc2>(ent1.geometry);

                bool secondIsPoint  = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);
                bool secondIsArc  = std::holds_alternative<sketch::Arc2>(ent2.geometry);

                if ((firstIsPoint && secondIsArc)   ||
                    (firstIsArc   && secondIsPoint) ||
                    (firstIsPoint && secondIsLine)  ||
                    (firstIsLine  && secondIsPoint) ||
                    (firstIsLine  && secondIsLine)) {
                    definitions.push_back({.type = ConstraintType::Midpoint, .refs = refs_});
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }
            break;
        }
        case ToolId::Symmetric: {
            // Pick entities
            if (selectedGeometry.empty()) {
                std::optional<GeometryPickResult> pick;
                pick = picker_.pickPointAtScreenLogical(e.x, e.y);
                if (!pick) {
                    pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                    if (!pick) {
                        return;
                    }
                }

                if (refs_.empty()) {
                    refs_.push_back(pick.value().ref);
                }
                if (pick->ref == refs_.front()) {
                    return;
                }
                if (refs_.size() == 1) {
                    refs_.push_back(pick.value().ref);

                    auto result1 = sketch_.entity(refs_[0].entity);
                    if (!result1) {
                        return;
                    }

                    auto result2 = sketch_.entity(refs_[1].entity);
                    if (!result2) {
                        return;
                    }

                    auto& ent1 = result1.value();
                    auto& ent2 = result2.value();

                    bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                    bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent1.geometry);

                    bool secondIsPoint  = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                    bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);

                    if ((firstIsPoint && secondIsLine)  ||
                        (firstIsLine  && secondIsPoint) ||
                        (firstIsPoint && secondIsPoint)) {
                    } else {
                        refs_.pop_back();
                    }
                    return;
                }

                if (refs_.size() == 2) {
                    refs_.push_back(pick.value().ref);

                    auto result1 = sketch_.entity(refs_[0].entity);
                    if (!result1) {
                        return;
                    }
                    auto result2 = sketch_.entity(refs_[1].entity);
                    if (!result2) {
                        return;
                    }
                    auto result3 = sketch_.entity(refs_[2].entity);
                    if (!result3) {
                        return;
                    }
                    auto& ent1 = result1.value();
                    auto& ent2 = result2.value();
                    auto& ent3 = result3.value();

                    bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                    bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent1.geometry);

                    bool secondIsPoint  = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                    bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);

                    bool thirdIsPoint  = std::holds_alternative<sketch::Point2>(ent3.geometry) || refs_[2].sub != sketch::SubElement::Whole;
                    bool thirdIsLine  = std::holds_alternative<sketch::Line2>(ent3.geometry);

                    if ((firstIsPoint && secondIsPoint && thirdIsPoint) ||
                        (firstIsPoint && secondIsLine && thirdIsPoint) ||
                        (firstIsPoint && secondIsPoint && thirdIsLine) ||
                        (firstIsLine && secondIsPoint && thirdIsPoint)) {
                        definitions.push_back({.type = ConstraintType::Symmetric, .refs = refs_});
                    }
                }
            }

            // Selection entities
            else {
                refs_ = selectedGeometry;
                if (refs_.size() != 3) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }

                auto result1 = sketch_.entity(refs_[0].entity);
                if (!result1) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
                auto result2 = sketch_.entity(refs_[1].entity);
                if (!result2) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
                auto result3 = sketch_.entity(refs_[2].entity);
                if (!result3) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
                auto& ent1 = result1.value();
                auto& ent2 = result2.value();
                auto& ent3 = result3.value();

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent1.geometry) || refs_[0].sub != sketch::SubElement::Whole;
                bool firstIsLine  = std::holds_alternative<sketch::Line2>(ent1.geometry);

                bool secondIsPoint  = std::holds_alternative<sketch::Point2>(ent2.geometry) || refs_[1].sub != sketch::SubElement::Whole;
                bool secondIsLine  = std::holds_alternative<sketch::Line2>(ent2.geometry);

                bool thirdIsPoint  = std::holds_alternative<sketch::Point2>(ent3.geometry) || refs_[2].sub != sketch::SubElement::Whole;
                bool thirdIsLine  = std::holds_alternative<sketch::Line2>(ent3.geometry);

                if ((firstIsPoint && secondIsPoint && thirdIsPoint) ||
                    (firstIsPoint && secondIsLine && thirdIsPoint) ||
                    (firstIsPoint && secondIsPoint && thirdIsLine) ||
                    (firstIsLine && secondIsPoint && thirdIsPoint)
                    ) {
                    definitions.push_back({.type = ConstraintType::Symmetric, .refs = refs_});
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }
            break;
        }
        case ToolId::Curvature: {
            return;
            break;
        }
        default: {
            return;
        }
    }

    const bool pushed = !definitions.empty() && pushConstraint(definitions, options);

    refs_.clear();
    overlay_.clearPreview();
    overlay_.selection_.model.clear();

    if (pushed) {
        sketch_.pushState();
    }
}

void ConstraintTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

bool ConstraintTool::cancel() {
    overlay_.clearPreview();
    isValueSet_ = false;
    refs_.clear();
    return false;
}

void ConstraintTool::setValue(double value) {
    cancel();
    value_ = value;
    isValueSet_ = true;
}

void ConstraintTool::setSubToolType(ToolId subTool) {
    overlay_.clearPreview();
    refs_.clear();

    subTool_ = subTool;

    if (!overlay_.selection_.model.geometryItems().empty()) {
        input::MouseButtonEvent mbe;
        mbe.button = input::MouseButton::Left;
        mbe.action = input::MouseButtonAction::Press;
        onMouseButton(mbe);
    }
}

bool ConstraintTool::pushConstraint(std::span<const sketch::ConstraintDefinition> definitions, const sketch::SolveOptions& options) {
    const auto result = sketch_.addConstraints(definitions, options);
    if (!result) {
        std::cout << "Sketch message: [Failure] " << result.error().message << '\n';
        return false;
    }

    const auto& outcome = result.value();
    if (outcome.diagnostics.solveStatus != sketch::SolveStatus::Converged || outcome.constraints.size() != definitions.size()) {
        std::cout << "Sketch message: [Failure] Not Converged" << '\n';
        return false;
    }

    if (outcome.diagnostics.targetsSatisfied == false) {
        std::cout << "Sketch message: [Warning] Requested placement could not be preserved" << '\n';
    }

    return true;
}

void ConstraintTool::pushEntityToOverlay(const sketch::GeometryRef& ref) const {
    const auto position = sketch_.pointPosition(ref);
    if (position) {
        pushPointToOverlay(position.value().x, position.value().y);
        return;
    }
    auto result = sketch_.entity(ref.entity);
    if (!result) {
        return;
    }
    if (const auto* point = std::get_if<sketch::Point2>(&result.value().geometry)) {
        pushPointToOverlay(point->position.x, point->position.y);
    } else if (const auto* line = std::get_if<sketch::Line2>(&result.value().geometry)) {
        if (ref.sub == sketch::SubElement::Whole) {
            pushLineToOverlay(line->start.x, line->start.y, line->end.x, line->end.y);
        } else if (ref.sub == sketch::SubElement::Start) {
            pushPointToOverlay(line->start.x, line->start.y);
        } else if (ref.sub == sketch::SubElement::End) {
            pushPointToOverlay(line->end.x, line->end.y);
        } else {
            return;
        }
    } else if (const auto* circle = std::get_if<sketch::Circle2>(&result.value().geometry)) {
        if (ref.sub == sketch::SubElement::Whole) {
            pushCircleToOverlay(circle->center.x, circle->center.y, circle->radius);
        } else if (ref.sub == sketch::SubElement::Center) {
            pushPointToOverlay(circle->center.x, circle->center.y);
        } else {
            return;
        }
    } else if (const auto* arc = std::get_if<sketch::Arc2>(&result.value().geometry)) {
        if (ref.sub == sketch::SubElement::Whole) {
            double sx = arc->start.x;
            double sy = arc->start.y;
            double ex = arc->end.x;
            double ey = arc->end.y;
            double cx = arc->center.x;
            double cy = arc->center.y;
            constexpr double TWO_PI = 2.0 * std::numbers::pi;
            double r = std::hypot(sx - cx, sy - cy);
            double a0 = std::atan2(sy - cy, sx - cx);
            double a1 = std::atan2(ey - cy, ex - cx);
            double delta = std::fmod(a1 - a0, TWO_PI);
            if (delta < 0) {
                delta += TWO_PI;
            }
            a1 = a0 + delta;
            pushArcToOverlay(cx, cy, r, a0, a1);
        } else if (ref.sub == sketch::SubElement::Start) {
            pushPointToOverlay(arc->start.x, arc->start.y);
        } else if (ref.sub == sketch::SubElement::End) {
            pushPointToOverlay(arc->end.x, arc->end.y);
        } else if (ref.sub == sketch::SubElement::Center) {
            pushPointToOverlay(arc->center.x, arc->center.y);
        } else {
            return;
        }
    }
}

void ConstraintTool::pushPointToOverlay(double x, double y) const {
    OverlayModel::Point overlayPoint;
    overlayPoint.type = OverlayModel::OverlayType::ConstraintHoverToObject;
    overlayPoint.x = x;
    overlayPoint.y = y;
    overlay_.points_.push_back(overlayPoint);
}

void ConstraintTool::pushLineToOverlay(double x1, double y1, double x2, double y2) const {
    OverlayModel::Line overlayLine;
    overlayLine.type = OverlayModel::OverlayType::ConstraintHoverToObject;
    overlayLine.x1 = x1;
    overlayLine.y1 = y1;
    overlayLine.x2 = x2;
    overlayLine.y2 = y2;
    overlay_.lines_.push_back(overlayLine);
}

void ConstraintTool::pushCircleToOverlay(double x, double y, double r) const {
    OverlayModel::Circle overlayCircle;
    overlayCircle.type = OverlayModel::OverlayType::ConstraintHoverToObject;
    overlayCircle.cx = x;
    overlayCircle.cy = y;
    overlayCircle.r = r;
    overlay_.circles_.push_back(overlayCircle);
}

void ConstraintTool::pushArcToOverlay(double x, double y, double r, double startAngle, double endAngle) const {
    OverlayModel::Arc overlayArc;
    overlayArc.type = OverlayModel::OverlayType::ConstraintHoverToObject;
    overlayArc.cx = x;
    overlayArc.cy = y;
    overlayArc.r = r;
    overlayArc.startAngle = startAngle;
    overlayArc.endAngle = endAngle;
    overlay_.arcs_.push_back(overlayArc);
}
