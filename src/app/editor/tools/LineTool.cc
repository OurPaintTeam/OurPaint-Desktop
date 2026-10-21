#include "LineTool.h"

#include <iomanip>

#include "../../../core/Document.h"
#include "../../../core/DocumentManager.h"
#include "../../../core/Scene.h"
#include "objects/Objects.h"

LineTool::LineTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker)
    :IInteractionTool(document,camera,snap,overlay, picker) {}

std::optional<ActionReport> LineTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;

    snap::SnapRequest request;

    if (state_ == State::WaitingFirstPoint) {
         request = snap::SnapRequest{ core::sketch::Vec2{v.x, v.y}, snap::ToolType::Line, snap::DrawState::Idle};
    }else if (state_ == State::WaitingSecondPoint) {
        request = snap::SnapRequest {
            core::sketch::Vec2{v.x, v.y},
            snap::ToolType::Line,
            snap::DrawState::FirstPoint,
            { core::sketch::Point2{ core::sketch::Vec2{firstPoint_.x, firstPoint_.y} } }
        };
    }

    snapVisual(request);

    if (state_ == State::WaitingSecondPoint) {
        if (input::has_flag(e.modifiers, input::Modifiers::Shift)) {
            glm::dvec2 delta = v - firstPoint_;

            // Snap to the dominant axis
            if (std::abs(delta.x) > std::abs(delta.y)) {
                v.y = firstPoint_.y; // Lock to horizontal
            } else {
                v.x = firstPoint_.x; // Lock to vertical
            }
        }

        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }
    return std::nullopt;
}


std::optional<ActionReport> LineTool::onMouseButton(const input::MouseButtonEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;

    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {

        if (snap_result_.snapped) {
            v.x = snap_result_.point.position.x;
            v.y = snap_result_.point.position.y;
        }

        if (state_ == State::WaitingFirstPoint) {
            firstPoint_ = v;
            state_ = State::WaitingSecondPoint;

            firstSnap_ = snap_result_;

            overlay_.points_.push_back(OverlayModel::Point(v.x, v.y));
            overlay_.points_.push_back(OverlayModel::Point(v.x, v.y));
            overlay_.lines_.push_back(OverlayModel::Line(v.x, v.y, v.x, v.y));
        }
        else {
            if (input::has_flag(e.modifiers, input::Modifiers::Shift)) {
                glm::dvec2 delta = v - firstPoint_;

                // Snap to the dominant axis
                if (std::abs(delta.x) > std::abs(delta.y)) {
                    v.y = firstPoint_.y; // Lock to horizontal
                } else {
                    v.x = firstPoint_.x; // Lock to vertical
                }
            }

            auto addResult = document_.sketch().addLine({firstPoint_.x, firstPoint_.y}, {v.x, v.y});
            ActionReport report = ActionReport::creation(ActionKind::CreateLine, addResult);

            if (addResult) {
                const auto lineId = addResult.value();
                bool needsSolve = false;

                auto applyConstraint = [&](const snap::SnapResult& snap, core::sketch::SubElement sub) {
                    if (!snap.snapped || snap.objects.empty()) return;

                    core::sketch::ConstraintDefinition def;
                    bool validType = false;

                    if (sub == core::sketch::SubElement::Start || sub == core::sketch::SubElement::End) {
                        switch (snap.type) {
                            case snap::SnapKind::StandalonePoint:
                            case snap::SnapKind::LineEndpoint:
                            case snap::SnapKind::CircleCentre:
                            case snap::SnapKind::ArcEndpoint:
                            case snap::SnapKind::ArcCentre:
                            case snap::SnapKind::LineBody:
                            case snap::SnapKind::CircleBody:
                            case snap::SnapKind::ArcBody:
                                def.type = core::sketch::ConstraintType::Coincident;
                                def.refs = { { lineId, sub }, snap.objects[0] };
                                validType = true;
                                break;
                            case snap::SnapKind::LineMidpoint:
                                def.type = core::sketch::ConstraintType::Midpoint;
                                def.refs = { { lineId, sub }, snap.objects[0] };
                                validType = true;
                                break;
                            default:
                                break;
                        }
                    }
                    else if (sub == core::sketch::SubElement::Whole) {
                        switch (snap.type) {
                            case snap::SnapKind::Tangent:
                                def.type = core::sketch::ConstraintType::Tangent;
                                def.refs = { { lineId, sub }, snap.objects[0] };
                                validType = true;
                                break;
                            case snap::SnapKind::Horizontal:
                                def.type = core::sketch::ConstraintType::Horizontal;
                                def.refs = { { lineId, sub } };
                                validType = true;
                                break;
                            case snap::SnapKind::Vertical:
                                def.type = core::sketch::ConstraintType::Vertical;
                                def.refs = { { lineId, sub } };
                                validType = true;
                                break;
                            case snap::SnapKind::Parallel:
                                def.type = core::sketch::ConstraintType::Parallel;
                                def.refs = { { lineId, sub }, snap.objects[0] };
                                validType = true;
                                break;
                            case snap::SnapKind::Perpendicular:
                                def.type = core::sketch::ConstraintType::Perpendicular;
                                def.refs = { { lineId, sub }, snap.objects[0] };
                                validType = true;
                                break;
                            default:
                                break;
                        }
                    }

                    if (validType && document_.sketch().supportsConstraint(def)) {
                        auto c = document_.sketch().addConstraint(def);
                        if (c) {
                            report.constraintIds.push_back(c.value());
                            report.requestedConstraints++;
                            needsSolve = true;
                        } else {
                            report.operationError = c.error();
                        }
                    }
                };

                applyConstraint(firstSnap_, core::sketch::SubElement::Start);
                applyConstraint(snap_result_, core::sketch::SubElement::End);

                applyConstraint(firstSnap_, core::sketch::SubElement::Whole);
                applyConstraint(snap_result_, core::sketch::SubElement::Whole);

                if (needsSolve) {
                    auto solved = document_.sketch().solve();
                    report.recordSolve(solved);
                    if (!solved || solved.value().status != core::sketch::SolveStatus::Converged) {
                        std::cout << "solve err in LineTool" << std::endl;
                    }
                }
            }

            // std::ostringstream oss;
            // oss << std::fixed << std::setprecision(3)
            //     << std::setw(10) << v.x
            //     << " "
            //     << std::setw(10) << v.y;
            // int posX = 1.5 * e.x;
            // int posY = 1.5 * -e.y;
            // renderData_.linePrview = {oss.str(), posX, posY};

            state_ = State::WaitingFirstPoint;

            overlay_.clearPreview();
            return report;
        }
    }
    return std::nullopt;
}


std::optional<ActionReport> LineTool::onKey(const input::KeyEvent& e) {
    if (input::has_flag(e.modifiers, input::Modifiers::Shift) &&
        e.action == input::KeyAction::Press &&
        state_ == State::WaitingSecondPoint) {

        glm::dvec2 v = lastCursorWorldPos_;
        glm::dvec2 delta = v - firstPoint_;

        // Snap to the dominant axis
        if (std::abs(delta.x) > std::abs(delta.y)) {
            v.y = firstPoint_.y; // Lock to horizontal
        } else {
            v.x = firstPoint_.x; // Lock to vertical
        }

        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }
    else if (e.action == input::KeyAction::Release &&
             state_ == State::WaitingSecondPoint) {
        glm::dvec2 v = lastCursorWorldPos_;
        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }
    return std::nullopt;
}

ToolCancellation LineTool::cancel() {
    if (state_ == State::WaitingSecondPoint) {
        overlay_.clearPreview();
        state_ = State::WaitingFirstPoint;
        return {true, std::nullopt};
    }
    return {};
}

