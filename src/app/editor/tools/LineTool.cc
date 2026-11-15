#include "LineTool.h"

#include <iomanip>

#include "../../../core/Document.h"
#include "../../../core/DocumentManager.h"
#include "../../../core/Scene.h"
#include "objects/Objects.h"

LineTool::LineTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker)
    : IInteractionTool(document, camera, snap, overlay, picker) {}

void LineTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;

    snap::SnapRequest request{core::sketch::Vec2{v.x, v.y}, snap::ActionType::Draw, snap::PrimitiveType::Line, snap::DrawState::Start};

    if (state_ == State::WaitingSecondPoint) {
        request.drawState = snap::DrawState::End;
        request.anchors.push_back(core::sketch::Point2{core::sketch::Vec2{firstPoint_.x, firstPoint_.y}});

        if (input::has_flag(e.modifiers, input::Modifiers::Shift)) {
            glm::dvec2 delta = v - firstPoint_;

            // Snap to the dominant axis
            if (std::abs(delta.x) > std::abs(delta.y)) {
                v.y = firstPoint_.y;  // Lock to horizontal
            } else {
                v.x = firstPoint_.x;  // Lock to vertical
            }
        }

        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }

    snapVisual(request);
}

void LineTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button != input::MouseButton::Left || e.action != input::MouseButtonAction::Press) {
        return;
    }

    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;

    if (!snap_results_.empty()) {
        v.x = snap_results_[0].point.position.x;
        v.y = snap_results_[0].point.position.y;
    }

    if (state_ == State::WaitingFirstPoint) {
        firstPoint_ = v;
        firstSnap_ = snap_results_.empty() ? snap::SnapResult{} : snap_results_.front();
        state_ = State::WaitingSecondPoint;

        overlay_.points_.push_back(OverlayModel::Point(v.x, v.y));
        overlay_.points_.push_back(OverlayModel::Point(v.x, v.y));
        overlay_.lines_.push_back(OverlayModel::Line(v.x, v.y, v.x, v.y));
        return;
    }

    bool shiftLocked = false;
    if (input::has_flag(e.modifiers, input::Modifiers::Shift)) {
        glm::dvec2 delta = v - firstPoint_;
        if (std::abs(delta.x) > std::abs(delta.y)) {
            v.y = firstPoint_.y;
        } else {
            v.x = firstPoint_.x;
        }
        shiftLocked = true;
    }

    auto addResult = document_.sketch().addLine({firstPoint_.x, firstPoint_.y}, {v.x, v.y});
    if (!addResult) {
        state_ = State::WaitingFirstPoint;
        overlay_.clearPreview();
        return;
    }

    const auto lineId = addResult.value();
    const core::sketch::GeometryRef startRef{lineId, core::sketch::SubElement::Start};
    const core::sketch::GeometryRef endRef{lineId, core::sketch::SubElement::End};

    bool needsSolve = false;

    if (applySnapConstraint(firstSnap_, startRef)) {
        needsSolve = true;
    }

    if (!shiftLocked) {
        for (const auto& snap : snap_results_) {
            if (applySnapConstraint(snap, endRef)) {
                needsSolve = true;
            }
        }
    }

    if (needsSolve) {
        auto solved = document_.sketch().solve();
        if (!solved || solved.value().status != core::sketch::SolveStatus::Converged) {
            std::cout << "solve err in LineTool\n";
        }
    }

    state_ = State::WaitingFirstPoint;
    overlay_.clearPreview();
}

void LineTool::onKey(const input::KeyEvent& e) {
    if (input::has_flag(e.modifiers, input::Modifiers::Shift) && e.action == input::KeyAction::Press && state_ == State::WaitingSecondPoint) {
        glm::dvec2 v = lastCursorWorldPos_;
        glm::dvec2 delta = v - firstPoint_;

        // Snap to the dominant axis
        if (std::abs(delta.x) > std::abs(delta.y)) {
            v.y = firstPoint_.y;  // Lock to horizontal
        } else {
            v.x = firstPoint_.x;  // Lock to vertical
        }

        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    } else if (e.action == input::KeyAction::Release && state_ == State::WaitingSecondPoint) {
        glm::dvec2 v = lastCursorWorldPos_;
        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }
}

bool LineTool::cancel() {
    if (state_ == State::WaitingSecondPoint) {
        overlay_.clearPreview();
        state_ = State::WaitingFirstPoint;
        firstSnap_.type = snap::SnapKind::None;
        return true;
    }
    return false;
}