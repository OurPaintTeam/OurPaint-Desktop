#include "LineTool.h"

#include <cmath>
#include <iostream>

#include "Document.h"
#include "DocumentManager.h"
#include "objects/Objects.h"

LineTool::LineTool(core::Document& document, render::Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker)
    :IInteractionTool(document,camera,snap,overlay, picker) {}

void LineTool::beginSegmentPreview(glm::dvec2 start) {
    firstPoint_ = start;
    state_ = State::WaitingSecondPoint;
    snapVisualClear();
    overlay_.clearPreview();
    overlay_.points_.emplace_back(start.x, start.y);
    overlay_.points_.emplace_back(start.x, start.y);
    overlay_.lines_.emplace_back(start.x, start.y, start.x, start.y);
}

glm::dvec2 LineTool::updateCursor(glm::dvec2 cursor, input::Modifiers modifiers) {
    snap::SnapRequest request{{cursor.x, cursor.y}, snap::ActionType::Draw, snap::PrimitiveType::Line, snap::DrawState::Start};

    if (state_ == State::WaitingSecondPoint) {
        request.drawState = snap::DrawState::End;
        request.anchors.push_back({{firstPoint_.x, firstPoint_.y}});
    }
    snapVisual(request);

    if (!snap_results_.empty()) {
        const auto& position = snap_results_.front().point.position;
        cursor = {position.x, position.y};
    }
    cursorPoint_ = cursor;
    return updatePreview(modifiers);
}

glm::dvec2 LineTool::updatePreview(input::Modifiers modifiers) {
    glm::dvec2 cursor = cursorPoint_;
    if (state_ == State::WaitingSecondPoint) {
        if (input::has_flag(modifiers, input::Modifiers::Shift)) {
            const glm::dvec2 delta = cursor - firstPoint_;
            if (std::abs(delta.x) > std::abs(delta.y)) {
                cursor.y = firstPoint_.y;
            } else {
                cursor.x = firstPoint_.x;
            }
        }

        overlay_.lines_[0].x2 = cursor.x;
        overlay_.lines_[0].y2 = cursor.y;
        overlay_.points_[0].x = cursor.x;
        overlay_.points_[0].y = cursor.y;
    }
    return cursor;
}

void LineTool::onMouseMove(const input::MouseMoveEvent& e) {
    updateCursor(camera_.screenLogicalToWorld({e.x, e.y}), e.modifiers);
}

void LineTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button != input::MouseButton::Left || e.action != input::MouseButtonAction::Press) {
        return;
    }

    const glm::dvec2 point = updateCursor(camera_.screenLogicalToWorld({e.x, e.y}), e.modifiers);

    if (state_ == State::WaitingFirstPoint) {
        firstSnap_ = snap_results_.empty() ? snap::SnapResult{} : snap_results_.front();
        beginSegmentPreview(point);
        return;
    }

    if (subTool_ == ToolId::Polyline && point == firstPoint_) {
        return;
    }

    auto& sketch = document_.sketch();
    const auto addResult = sketch.addLine({firstPoint_.x, firstPoint_.y}, {point.x, point.y});
    if (!addResult) {
        cancel();
        return;
    }

    const auto lineId = addResult.value();
    const core::sketch::GeometryRef startRef{lineId, core::sketch::SubElement::Start};
    const core::sketch::GeometryRef endRef{lineId, core::sketch::SubElement::End};

    if (previousEnd_) {
        const auto joint = sketch.addConstraint({.type = core::sketch::ConstraintType::Coincident, .refs = {*previousEnd_, startRef}});
        if (!joint || !joint.value().constraint) {
            const auto removed = sketch.removeEntity(lineId);
            if (!removed) {
                std::cerr << "Could not roll back polyline segment: " << removed.error().message << '\n';
                cancel();
            }
            return;
        }
    } else {
        applySnapConstraint(firstSnap_, startRef);
    }

    if (!input::has_flag(e.modifiers, input::Modifiers::Shift)) {
        for (const auto& snap : snap_results_) {
            applySnapConstraint(snap, endRef);
        }
    }

    sketch.pushState();
    if (subTool_ == ToolId::Polyline) {
        const auto end = sketch.pointPosition(endRef);
        if (end) {
            previousEnd_ = endRef;
            firstSnap_ = {};
            beginSegmentPreview({end.value().x, end.value().y});
            return;
        }
    }
    cancel();
}

void LineTool::onKey(const input::KeyEvent& e) {
    if (state_ == State::WaitingSecondPoint && e.action != input::KeyAction::Repeat) {
        updatePreview(e.modifiers);
    }
}

bool LineTool::cancel() {
    snapVisualClear();
    if (state_ == State::WaitingSecondPoint) {
        overlay_.clearPreview();
        state_ = State::WaitingFirstPoint;
        firstSnap_ = {};
        previousEnd_.reset();
        return true;
    }
    return false;
}

void LineTool::setSubToolType(ToolId subTool) {
    subTool_ = subTool;
}
