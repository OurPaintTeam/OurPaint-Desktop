#include "LineTool.h"

#include "Document.h"
#include "DocumentManager.h"
#include "objects/Objects.h"
#include "Scene.h"
#include "Transaction.h"
#include "ConsoleManager.h"
#include "UndoRedo.h"

#include <iomanip>

LineTool::LineTool(DocumentManager& documentManager, Camera2D& camera, renderer::RenderData& renderData, OverlayModel& overlay)
    : firstPoint_X(0), firstPoint_Y(0), documentManager_(documentManager), camera_(camera), renderData_(renderData), overlay_(overlay) {}

void LineTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenToWorld({e.x, e.y});

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3)
        << std::setw(10) << v.x
        << " "
        << std::setw(10) << v.y;
    overlay_.pos = oss.str();
    overlay_.posX = 1.5 * e.x - 150;
    overlay_.posY = 1.5 * -e.y + 650;


    if (state_ == State::WaitingSecondPoint) {
        // glm::dvec2 startPoint = {renderData_.overlay.points[0].x, renderData_.overlay.points[0].y};
        // glm::dvec2 endPoint = v;
        // glm::dvec2 midPoint = (startPoint + endPoint) / 2.0;
        // glm::dvec2 midScreen = camera_.worldToScreen(midPoint);
        // int posX = midScreen.x;
        // int posY = midScreen.y;
        // double length = glm::distance(startPoint, endPoint);
        // std::ostringstream oss2;
        // oss2 << std::fixed << std::setprecision(3) << length;
        // overlay_.lp = oss2.str();
        // overlay_.lpX = 1.5 * posX - 150;
        // overlay_.lpY = 1.5 * -posY + 650;


        //glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
        renderData_.overlay.lines[0].x2 = v.x;
        renderData_.overlay.lines[0].y2 = v.y;
        renderData_.overlay.points[1].x = v.x;
        renderData_.overlay.points[1].y = v.y;
    }
}


void LineTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        if (state_ == State::WaitingFirstPoint) {
            glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
            firstPoint_X = v.x;
            firstPoint_Y = v.y;
            state_ = State::WaitingSecondPoint;

            renderData_.overlay.points.push_back(renderer::Point(firstPoint_X, firstPoint_Y));
            renderData_.overlay.points.push_back(renderer::Point(firstPoint_X, firstPoint_Y));
            renderData_.overlay.lines.push_back(renderer::Line(firstPoint_X, firstPoint_Y, firstPoint_X, firstPoint_Y));
        }
        else {
            Document* document = documentManager_.getActiveDocument();
            glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
            UndoRedo::Transaction* txn = document->commandManager().invoke("LINE", {firstPoint_X , firstPoint_Y, v.x, v.y});
            document->undoRedoManager().push(std::move(*txn));

            // std::ostringstream oss;
            // oss << std::fixed << std::setprecision(3)
            //     << std::setw(10) << v.x
            //     << " "
            //     << std::setw(10) << v.y;
            // int posX = 1.5 * e.x;
            // int posY = 1.5 * -e.y;
            // renderData_.linePrview = {oss.str(), posX, posY};

            state_ = State::WaitingFirstPoint;

            renderData_.overlay.clear();
        }
    }
}

void LineTool::onKey(const input::KeyEvent& e) {

}

bool LineTool::cancel() {
    if (state_ == State::WaitingSecondPoint) {
        renderData_.overlay.clear();
        state_ = State::WaitingFirstPoint;
        return true;
    }
    return false;
}

