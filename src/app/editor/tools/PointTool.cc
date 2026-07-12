#include "PointTool.h"

#include "ConsoleManager.h"
#include "Document.h"
#include "Scene.h"
#include "Transaction.h"
#include "UndoRedo.h"
#include "objects/Objects.h"

PointTool::PointTool(Document& document, Camera2D& camera) : document_(document), camera_(camera) {}

void PointTool::onMouseMove(const input::MouseMoveEvent& e) {

}

void PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
        UndoRedo::Transaction* txn = document_.commandManager().invoke("POINT", { v.x, v.y });
        document_.undoRedoManager().push(std::move(*txn));
    }
}

void PointTool::onKey(const input::KeyEvent& e) {

}

bool PointTool::cancel() {
    return false;
}


