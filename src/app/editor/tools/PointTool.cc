#include "PointTool.h"

#include "../../../core/Document.h"
#include "objects/Objects.h"

PointTool::PointTool(Document& document, Camera2D& camera) : document_(document), camera_(camera) {}

std::optional<ActionReport> PointTool::onMouseMove(const input::MouseMoveEvent& e) {
    return std::nullopt;
}

std::optional<ActionReport> PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
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


