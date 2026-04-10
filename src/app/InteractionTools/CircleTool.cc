#include "CircleTool.h"
#include "Document.h"
#include "DocumentManager.h"
#include "Objects.h"
#include "Scene.h"
#include "RenderData.h"

CircleTool::CircleTool(DocumentManager& documentManager_, Camera2D& camera, renderer::RenderData& renderData)
    : firstPoint_X(0), firstPoint_Y(0), documentManager_(documentManager_), camera_(camera), renderData_(renderData) {}

void CircleTool::onMouseMove(const input::MouseMoveEvent& e) {
    if (state_ == State::WaitingSecondPoint) {
        glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
        double r = sqrt(pow(v.x - firstPoint_X, 2) + pow(v.y - firstPoint_Y, 2));
        if (r <= 0.0) {
            return;
        }
        renderData_.overlay.circles[0].r = r;
    }
}

void CircleTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        if (state_ == State::WaitingFirstPoint) {
            glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
            firstPoint_X = v.x;
            firstPoint_Y = v.y;
            state_ = State::WaitingSecondPoint;
            renderData_.overlay.circles.push_back(renderer::Circle(firstPoint_X, firstPoint_Y, 0.0));
            renderData_.overlay.points.push_back(renderer::Point(firstPoint_X, firstPoint_Y));
        }
        else {
            glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
            double r = sqrt(pow(v.x - firstPoint_X, 2) + pow(v.y - firstPoint_Y, 2));
            if (r <= 0) {
                return;
            }
            Document* document = documentManager_.getActiveDocument();
            Scene& scene = document->scene();

            ObjectData od;
            od.et = ObjType::ET_CIRCLE;
            od.params = {firstPoint_X , firstPoint_Y, r};
            scene.addObject(od);

            state_ = State::WaitingFirstPoint;

            renderData_.overlay.clear();
        }
    }
}

void CircleTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

bool CircleTool::cancel() {
    if (state_ == State::WaitingSecondPoint) {
        renderData_.overlay.clear();
        state_ = State::WaitingFirstPoint;
        return true;
    }
    return false;
}

