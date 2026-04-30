#include "PointTool.h"

#include "ConsoleManager.h"
#include "Document.h"
#include "Scene.h"
#include "SnapEngine.h"
#include "Transaction.h"
#include "UndoRedo.h"
#include "objects/Objects.h"

PointTool::PointTool(DocumentManager& documentManager, Camera2D& camera, SnapEngine& snapEngine) : documentManager_(documentManager), camera_(camera), snapEngine_(snapEngine) {}

void PointTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::vec2 cursor = camera_.screenToWorld({e.x, e.y});
    const SnapEngine::SnapRequest req(
        SnapEngine::TypeObject::point,
        documentManager_.getActiveDocument()->scene(),
        {cursor.x, cursor.y}
        );

    if (const auto res = snapEngine_.getHint(req); res.action == SnapEngine::TypeAction::pointOnPoint) {
        std::cout<< res.candidate.first <<" "<<res.candidate.second << std::endl;
        if (!res.ids.empty()) {
        std::cout<<"ID "<< res.ids[0].id << std::endl;
        }
    }
    if (const auto res = snapEngine_.getHint(req); res.action == SnapEngine::TypeAction::pointOnAxis) {
        std::cout<<"CAND AXIS"<< res.candidate.first<<" "<< res.candidate.second << std::endl;
    }
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


