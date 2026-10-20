#include "PointTool.h"

#include "../../../core/Document.h"
#include "objects/Objects.h"

PointTool::PointTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker) :
IInteractionTool(document,camera,snap,overlay, picker) {}

std::optional<ActionReport> PointTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    snap::SnapRequest request{ core::sketch::Vec2{v.x, v.y}, snap::ToolType::Point, snap::DrawState::Idle};
    snapVisual(request);
    return std::nullopt;
}

std::optional<ActionReport> PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
        if (snap_result_.snapped) {
            v.x = snap_result_.point.position.x;
            v.y = snap_result_.point.position.y;
        }
        return ActionReport::creation(ActionKind::CreatePoint, document_.sketch().addPoint({v.x, v.y}));
    }
    return std::nullopt;
}

std::optional<ActionReport> PointTool::onKey(const input::KeyEvent& e) {
    return std::nullopt;
}

ToolCancellation PointTool::cancel() {
    return {};
}


