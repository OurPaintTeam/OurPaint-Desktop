#include "ConstraintTool.h"

#include <iostream>
#include <numeric>

#include "../../viewport/OverlayModel.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "Document.h"
#include "DraggleButton.h"

ConstraintTool::ConstraintTool(Document& document,
                               Camera2D& camera,
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

                bool firstIsPoint  = std::holds_alternative<sketch::Point2>(ent.geometry);
                bool firstIsLine = std::holds_alternative<sketch::Line2>(ent.geometry);

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
        case ToolId::VerticalDistance:
        case ToolId::Coincident: {
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
            auto pick = picker_.pickAtScreenLogical(e.x, e.y);
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
        default: {
            return;
        }
    }
}

void ConstraintTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button != input::MouseButton::Left || e.action != input::MouseButtonAction::Press) {
        return;
    }

    switch (subTool_) {
        case ToolId::Distance: {
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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
                        constraintType_ = sketch::ConstraintType::Distance;
                    } else {
                        refs_.clear();
                        overlay_.selection_.model.clear();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = overlay_.selection_.model.items(); // should be two lines
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
                    constraintType_ = sketch::ConstraintType::Distance;
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
            }

            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        case ToolId::HorizontalDistance: {
            constraintType_ = sketch::ConstraintType::HorizontalDistance;
        }
        case ToolId::VerticalDistance: {
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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
                refs_ = overlay_.selection_.model.items();
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

            if (!constraintType_.has_value()) {
                constraintType_ = sketch::ConstraintType::VerticalDistance;
            }

            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        case ToolId::Coincident: {
            // An invalid preselection must not silently become a partial selection.
            if (refs_.empty() && !overlay_.selection_.model.empty()) {
                refs_.clear();
                overlay_.selection_.model.clear();
                return;
            }
            const auto pick = picker_.pickPointAtScreenLogical(e.x, e.y);
            if (!pick || !sketch_.pointPosition(pick->ref)) {
                return;
            }
            if (refs_.empty()) {
                refs_.push_back(pick->ref);
                return;
            }
            if (pick->ref == refs_.front()) {
                return;
            }
            if (refs_.size() == 1) {
                refs_.push_back(pick->ref);
                coincidentMidpoint_ = false;
            } else if (pick->ref != refs_[1]) {
                refs_[1] = pick->ref;
                coincidentMidpoint_ = false;
            }
            pushCoincident();
            break;
        }
        case ToolId::Horizontal: {
            constraintType_ = sketch::ConstraintType::Horizontal;
        }
        case ToolId::Vertical: {
            std::vector<sketch::GeometryRef> refs; // should be lines
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
                pick = picker_.pickLineAtScreenLogical(e.x, e.y);
                if (!pick) {
                    return;
                }
                refs.push_back(pick.value().ref); // add just one ref
            }
            // Selection entities
            else {
                refs = overlay_.selection_.model.items(); // can be more than 1 line
            }

            if (!constraintType_.has_value()) {
                constraintType_ = sketch::ConstraintType::Vertical;
            }

            bool allRefsIsLines = true;
            for (auto& ref : refs) {
                auto result = sketch_.entity(ref.entity);
                if (!result) {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                    return;
                }
                auto& ent = result.value();
                if (!std::holds_alternative<sketch::Line2>(ent.geometry)) {
                    allRefsIsLines = false; // if all entities is not lines -> do not apply
                    break;
                }
            }

            if (allRefsIsLines) {
                for (auto& ref : refs) {
                    refs_.push_back(ref);
                    pushConstraint();
                    refs_.clear();
                }
                sketch_.solve();
                overlay_.clearPreview();
                overlay_.selection_.model.clear();
                constraintType_.reset();
            } else {
                refs_.clear();
                overlay_.selection_.model.clear();
            }
            break;
        }
        case ToolId::Angle: {
            if (!isValueSet_) {
                return;
            }
            constraintType_ = sketch::ConstraintType::Angle;
        }
        case ToolId::Collinear: {
            if (!constraintType_.has_value()) {
                constraintType_ = sketch::ConstraintType::Collinear;
            }
        }
        case ToolId::Parallel: {
            if (!constraintType_.has_value()) {
                constraintType_ = sketch::ConstraintType::Parallel;
            }
        }
        case ToolId::Perpendicular: {
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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
                refs_ = overlay_.selection_.model.items(); // should be two lines
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

            if (!constraintType_.has_value()) {
                constraintType_ = sketch::ConstraintType::Perpendicular;
            }

            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        case ToolId::Tangent: {
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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

                    bool firstIsCircle  = std::holds_alternative<sketch::Circle2>(ent1.geometry);
                    bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent2.geometry);
                    bool firstIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                    bool secondIsLine = std::holds_alternative<sketch::Line2>(ent2.geometry);

                    if ((firstIsCircle && secondIsCircle) || (firstIsLine && secondIsCircle) || (firstIsCircle && secondIsLine)) {
                        constraintType_ = sketch::ConstraintType::Tangent;
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = overlay_.selection_.model.items(); // should be two circles or line+circle
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
                bool firstIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                bool secondIsLine = std::holds_alternative<sketch::Line2>(ent2.geometry);

                if ((firstIsCircle && secondIsCircle) || (firstIsLine && secondIsCircle) || (firstIsCircle && secondIsLine)) {
                    constraintType_ = sketch::ConstraintType::Tangent;
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }

            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        case ToolId::Equal: {
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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
                    bool secondIsLine = std::holds_alternative<sketch::Line2>(ent1.geometry);
                    bool firstIsCircle = std::holds_alternative<sketch::Circle2>(ent1.geometry);
                    bool secondIsCircle = std::holds_alternative<sketch::Circle2>(ent1.geometry);

                    if ((firstIsLine && secondIsLine) || (firstIsCircle && secondIsCircle)){
                        constraintType_ = sketch::ConstraintType::Equal;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = overlay_.selection_.model.items(); // should be two lines
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

                if ((firstIsLine && secondIsLine) || (firstIsCircle && secondIsCircle)){
                    constraintType_ = sketch::ConstraintType::Equal;
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }
            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        case ToolId::Fix: {
            std::vector<sketch::GeometryRef> refs;
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
                pick = picker_.pickAtScreenLogical(e.x, e.y);
                if (!pick) {
                    return;
                }
                refs.push_back(pick.value().ref); // add just one ref
            }
            // Selection entities
            else {
                refs = overlay_.selection_.model.items(); // can be more than 1 line
            }

            constraintType_ = sketch::ConstraintType::Fix;
            for (auto& ref : refs) {
                refs_.push_back(ref);
                pushConstraint();
                refs_.clear();
            }
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();
            break;
        }
        case ToolId::Concentric: {
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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
                        constraintType_ = sketch::ConstraintType::Concentric;
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = overlay_.selection_.model.items(); // first is a circle/arc, second is a circle/arc
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
                    constraintType_ = sketch::ConstraintType::Concentric;
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }

            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        case ToolId::Midpoint: {
            // point arc
            // point line
            // line point
            // line line
            // arc point

            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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
                        constraintType_ = sketch::ConstraintType::Midpoint;
                    } else {
                        refs_.pop_back();
                        return;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = overlay_.selection_.model.items();
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
                    constraintType_ = sketch::ConstraintType::Midpoint;
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }

            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        case ToolId::Symmetric: {
            // Pick entities
            if (overlay_.selection_.model.items().empty()) {
                std::optional<PickResult> pick;
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
                        constraintType_ = sketch::ConstraintType::Symmetric;
                    }
                }
            }

            // Selection entities
            else {
                refs_ = overlay_.selection_.model.items();
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
                    constraintType_ = sketch::ConstraintType::Symmetric;
                } else {
                    refs_.clear();
                    overlay_.selection_.model.clear();
                }
            }

            pushConstraint();

            refs_.clear();
            sketch_.solve();
            overlay_.clearPreview();
            overlay_.selection_.model.clear();
            constraintType_.reset();

            break;
        }
        default: {
            return;
        }
    }
}

void ConstraintTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

bool ConstraintTool::cancel() {
    overlay_.clearPreview();
    isValueSet_ = false;
    isConstraintTypeSet_ = false;
    refs_.clear();
    coincidentMidpoint_ = false;
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
    coincidentMidpoint_ = false;

    subTool_ = subTool;
    isConstraintTypeSet_ = true;

    if (subTool_ == ToolId::Coincident) {
        const auto selected = overlay_.selection_.model.items();
        if (selected.size() > 2) {
            std::cout << "Coincident requires at most two selected points" << '\n';
            refs_.clear();
            overlay_.selection_.model.clear();
            return;
        }
        for (const auto& ref : selected) {
            if (!sketch_.pointPosition(ref)) {
                std::cout << "Coincident requires point elements" << '\n';
                refs_.clear();
                overlay_.selection_.model.clear();
                return;
            }
        }
        if (selected.size() == 2 && selected[0] == selected[1]) {
            return;
        }
        refs_ = selected;
        coincidentMidpoint_ = refs_.size() == 2;
        if (coincidentMidpoint_) {
            pushCoincident();
        }
        return;
    }

    if (!overlay_.selection_.model.empty()) {
        input::MouseButtonEvent mbe;
        mbe.button = input::MouseButton::Left;
        mbe.action = input::MouseButtonAction::Press;
        onMouseButton(mbe);
    }
}

void ConstraintTool::pushConstraint() {
    sketch::ConstraintDefinition def;

    if (!constraintType_) {
        return;
    }
    def.type = constraintType_.value();

    if (refs_.empty()) {
        return;
    }
    def.refs = refs_;

    if (isValueSet_) {
        def.value = value_;
    }

    auto result = sketch_.addConstraint(def);
    if (!result) {
        std::cout << "Constraint not added" << '\n';
    }
}

void ConstraintTool::pushCoincident() {
    if (refs_.size() != 2 || refs_[0] == refs_[1]) {
        refs_.clear();
        overlay_.selection_.model.clear();
        return;
    }
    const auto first = sketch_.pointPosition(refs_[0]);
    const auto second = sketch_.pointPosition(refs_[1]);
    if (!first || !second) {
        std::cout << "Coincident requires point elements" << '\n';
        refs_.clear();
        overlay_.selection_.model.clear();
        return;
    }
    const sketch::Vec2 position =
        coincidentMidpoint_ ?
        sketch::Vec2{std::midpoint(first.value().x, second.value().x), std::midpoint(first.value().y, second.value().y)} :
        second.value();

    const sketch::PointTarget target{refs_[0], position};
    const sketch::SolveOptions options{std::span(&target, 1), sketch::TargetPolicy::Prefer};
    sketch::ConstraintDefinition definition;
    definition.type = sketch::ConstraintType::Coincident;
    definition.refs = refs_;
    const auto result = sketch_.addConstraint(definition, options);
    if (!result) {
        std::cout << "Coincident not added: " << result.error().message << '\n';
        refs_.clear();
        overlay_.selection_.model.clear();
        return;
    }
    const auto& outcome = result.value();
    if (!outcome.constraint || outcome.diagnostics.status != sketch::SolveStatus::Converged) {
        std::cout << "Coincident not added: solve did not converge" << '\n';
        refs_.clear();
        overlay_.selection_.model.clear();
        return;
    }
    if (outcome.diagnostics.targetsSatisfied == false) {
        std::cout << "Coincident added; requested placement could not be preserved" << '\n';
    }
    refs_.clear();
    coincidentMidpoint_ = false;
    overlay_.clearPreview();
    overlay_.selection_.model.clear();
    constraintType_.reset();
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
