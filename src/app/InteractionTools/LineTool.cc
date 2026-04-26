#include "LineTool.h"

#include "Document.h"
#include "DocumentManager.h"
#include "objects/Objects.h"
#include "Scene.h"
#include "Transaction.h"
#include "ConsoleManager.h"
#include "UndoRedo.h"

#include <iomanip>

LineTool::LineTool(DocumentManager& documentManager, Camera2D& camera, OverlayModel& overlay)
    : documentManager_(documentManager), camera_(camera), overlay_(overlay) {}

void LineTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;

    // std::ostringstream oss;
    // oss << std::fixed << std::setprecision(3)
    //     << std::setw(10) << v.x
    //     << " "
    //     << std::setw(10) << v.y;
    // overlay_.pos = oss.str();
    // overlay_.posX = 1.5 * e.x - 150;
    // overlay_.posY = 1.5 * -e.y + 650;


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


        if (input::has_flag(e.modifiers, input::Modifiers::Shift)) {
            glm::dvec2 delta = v - firstPoint_;

            // Snap to the dominant axis
            if (std::abs(delta.x) > std::abs(delta.y)) {
                v.y = firstPoint_.y; // Lock to horizontal
            } else {
                v.x = firstPoint_.x; // Lock to vertical
            }
        }

        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }
}


void LineTool::onMouseButton(const input::MouseButtonEvent& e) {
    glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
    lastCursorWorldPos_ = v;
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        if (state_ == State::WaitingFirstPoint) {
            firstPoint_ = v;
            state_ = State::WaitingSecondPoint;


            overlay_.points_.push_back(OverlayModel::Point(v.x, v.y));
            overlay_.points_.push_back(OverlayModel::Point(v.x, v.y));
            overlay_.lines_.push_back(OverlayModel::Line(v.x, v.y, v.x, v.y));
        }
        else {
            if (input::has_flag(e.modifiers, input::Modifiers::Shift)) {
                glm::dvec2 delta = v - firstPoint_;

                // Snap to the dominant axis
                if (std::abs(delta.x) > std::abs(delta.y)) {
                    v.y = firstPoint_.y; // Lock to horizontal
                } else {
                    v.x = firstPoint_.x; // Lock to vertical
                }
            }
            Document* document = documentManager_.getActiveDocument();
            UndoRedo::Transaction* txn = document->commandManager().invoke("LINE", {firstPoint_.x, firstPoint_.y, v.x, v.y});
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

            overlay_.clear();
        }
    }
}

void LineTool::onKey(const input::KeyEvent& e) {
    if (input::has_flag(e.modifiers, input::Modifiers::Shift) &&
        e.action == input::KeyAction::Press &&
        state_ == State::WaitingSecondPoint) {

        glm::dvec2 v = lastCursorWorldPos_;
        glm::dvec2 delta = v - firstPoint_;

        // Snap to the dominant axis
        if (std::abs(delta.x) > std::abs(delta.y)) {
            v.y = firstPoint_.y; // Lock to horizontal
        } else {
            v.x = firstPoint_.x; // Lock to vertical
        }

        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }
    else if (e.action == input::KeyAction::Release &&
             state_ == State::WaitingSecondPoint) {
        glm::dvec2 v = lastCursorWorldPos_;
        overlay_.lines_[0].x2 = v.x;
        overlay_.lines_[0].y2 = v.y;
        overlay_.points_[0].x = v.x;
        overlay_.points_[0].y = v.y;
    }

}

bool LineTool::cancel() {
    if (state_ == State::WaitingSecondPoint) {
        overlay_.clear();
        state_ = State::WaitingFirstPoint;
        return true;
    }
    return false;
}

