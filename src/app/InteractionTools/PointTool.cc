#include "PointTool.h"

#include "Document.h"
#include "Objects.h"
#include "Scene.h"

PointTool::PointTool(DocumentManager& documentManager, Camera2D& camera) : documentManager_(documentManager), camera_(camera) {}

void PointTool::onMouseMove(const input::MouseMoveEvent& e) {

}

void PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        Scene& scene = documentManager_.getActiveDocument()->scene();
        ObjectData od;
        od.et = ObjType::ET_POINT;
        glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
        od.params = {v.x, v.y};
        scene.addObject(od);
    }
}

void PointTool::onKey(const input::KeyEvent& e) {

}

bool PointTool::cancel() {
    return false;
}


