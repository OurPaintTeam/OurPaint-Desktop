#include "PointTool.h"
#include "Document.h"
#include "objects/Objects.h"
#include "Scene.h"
#include "Transaction.h"
#include "ConsoleManager.h"
#include "UndoRedo.h"

PointTool::PointTool(DocumentManager& documentManager, Camera2D& camera) : documentManager_(documentManager), camera_(camera) {}

void PointTool::onMouseMove(const input::MouseMoveEvent& e) {

}

void PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        Document* document = documentManager_.getActiveDocument();
        glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
        UndoRedo::Transaction* txn = document->commandManager().invoke("POINT", { v.x, v.y });
        document->undoRedoManager().push(std::move(*txn));


    }
}

void PointTool::onKey(const input::KeyEvent& e) {

}

bool PointTool::cancel() {
    return false;
}


