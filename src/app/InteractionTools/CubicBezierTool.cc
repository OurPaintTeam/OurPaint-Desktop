#include "CubicBezierTool.h"

#include "Document.h"
#include "DocumentManager.h"
#include "objects/Objects.h"
#include "Scene.h"
#include "Transaction.h"
#include "ConsoleManager.h"
#include "UndoRedo.h"

CubicBezierTool::CubicBezierTool(DocumentManager& documentManager, Camera2D& camera, render::RenderScene& renderScene)
    : firstPoint_X(0), firstPoint_Y(0), documentManager_(documentManager), camera_(camera), renderData_(renderScene) {}

void CubicBezierTool::onMouseMove(const input::MouseMoveEvent& e) {
    if (state_ == State::WaitingSecondPoint) {
        glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
        // renderData_.overlay.lines[0].x2 = v.x;
        // renderData_.overlay.lines[0].y2 = v.y;
        // renderData_.overlay.points[1].x = v.x;
        // renderData_.overlay.points[1].y = v.y;
        //
        // const double& x0 = renderData_.overlay.points[0].x;
        // const double& y0 = renderData_.overlay.points[0].y;
        //
        // renderData_.overlay.points[2].x = x0 + (v.x - x0) / 3.0;
        // renderData_.overlay.points[2].y = y0 + (v.y - y0) / 3.0;
        //
        // renderData_.overlay.points[3].x = x0 + 2.0 * (v.x - x0) / 3.0;
        // renderData_.overlay.points[3].y = y0 + 2.0 * (v.y - y0) / 3.0;

    }
}

void CubicBezierTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        if (state_ == State::WaitingFirstPoint) {
            glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
            firstPoint_X = v.x;
            firstPoint_Y = v.y;
            state_ = State::WaitingSecondPoint;

            // renderData_.overlay.points.push_back(renderer::Point(firstPoint_X, firstPoint_Y));
            // renderData_.overlay.points.push_back(renderer::Point(firstPoint_X, firstPoint_Y));
            // renderData_.overlay.points.push_back(renderer::Point(firstPoint_X, firstPoint_Y));
            // renderData_.overlay.points.push_back(renderer::Point(firstPoint_X, firstPoint_Y));
            // renderData_.overlay.lines.push_back(renderer::Line(firstPoint_X, firstPoint_Y, firstPoint_X, firstPoint_Y));
        }
        else {
            Document* document = documentManager_.getActiveDocument();
            glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
            UndoRedo::Transaction* txn = document->commandManager().invoke("BEZIER", {firstPoint_X , firstPoint_Y, v.x, v.y});
            document->undoRedoManager().push(std::move(*txn));

            state_ = State::WaitingFirstPoint;

            // renderData_.overlay.clear();
        }
    }
}

void CubicBezierTool::onKey(const input::KeyEvent& e) {

}

bool CubicBezierTool::cancel() {
    if (state_ == State::WaitingSecondPoint) {
        // renderData_.overlay.clear();
        state_ = State::WaitingFirstPoint;
        return true;
    }
    return false;
}

