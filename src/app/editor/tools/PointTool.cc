#include "PointTool.h"

#include "../../../core/Document.h"
#include "objects/Objects.h"

PointTool::PointTool(Document& document, Camera2D& camera) : document_(document), camera_(camera) {}

void PointTool::onMouseMove(const input::MouseMoveEvent& e) {

}

void PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
        document_.sketch().addPoint({ v.x, v.y });
    }
}

void PointTool::onKey(const input::KeyEvent& e) {

}

bool PointTool::cancel() {
    return false;
}


