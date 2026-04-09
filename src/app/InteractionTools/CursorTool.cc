#include "CursorTool.h"

#include "Document.h"
#include "Scene.h"

CursorTool::CursorTool(DocumentManager& documentManager, Camera2D& camera, renderer::RenderData& renderData)
    : documentManager_(documentManager), camera_(camera), renderData_(renderData) {
    obj_ = ObjectData();
}

void CursorTool::onMouseMove(const input::MouseMoveEvent& e) {
    if ((e.buttons & input::MouseButton::Left) != input::MouseButton::None) {
        if (state_ == State::Moving) {
            glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
            Scene& scene = documentManager_.getActiveDocument()->scene();
            if (obj_.et == ObjType::ET_POINT) {
                scene.movePoint(obj_.id, v.x, v.y);
            }
            else if (obj_.et == ObjType::ET_SECTION) {
                scene.moveSection(obj_.id, v.x - lastPos_.x, v.y - lastPos_.y);
            }
            else if (obj_.et == ObjType::ET_CIRCLE) {
                scene.moveCircle(obj_.id, v.x - lastPos_.x, v.y - lastPos_.y);
            }
            lastPos_.x = v.x;
            lastPos_.y = v.y;
            
        }
    }
}

void CursorTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        if (state_ == State::WaitingStartMoving) {
            glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
            double eps = 0.05 / (camera_.zoom() / 100.0);
            Scene& scene = documentManager_.getActiveDocument()->scene();
            std::vector<ObjectData> points = scene.getPoints();
            for (const auto& p : points) {
                const double& x = p.params[0];
                const double& y = p.params[1];
                if (std::abs(x - v.x) < eps && std::abs(y - v.y) < eps) {
                    obj_ = p;
                    state_ = State::Moving;
                    lastPos_ = v;
                    return;
                }
            }

            std::vector<ObjectData> lines = scene.getLines();
            for (const auto& l : lines) {
                const double& x1 = l.params[0];
                const double& y1 = l.params[1];
                const double& x2 = l.params[2];
                const double& y2 = l.params[3];

                double dx = x2 - x1;
                double dy = y2 - y1;

                double px = v.x - x1;
                double py = v.y - y1;

                double cross = std::abs(dx * py - dy * px);

                double len = std::sqrt(dx*dx + dy*dy);
                if (len < 1e-9) {
                    if (std::abs(v.x - x1) < eps && std::abs(v.y - y1) < eps) {
                        obj_ = l;
                        state_ = State::Moving;
                        lastPos_ = v;
                    }
                    continue;
                }

                double dist = cross / len;

                if (dist < eps) {
                    double minX = std::min(x1, x2) - eps;
                    double maxX = std::max(x1, x2) + eps;
                    double minY = std::min(y1, y2) - eps;
                    double maxY = std::max(y1, y2) + eps;

                    if (v.x >= minX && v.x <= maxX && v.y >= minY && v.y <= maxY) {
                        obj_ = l;
                        state_ = State::Moving;
                        lastPos_ = v;
                        return;
                    }
                }
            }

            std::vector<ObjectData> circles = scene.getCircles();
            for (const auto& c : circles) {
                const double& x = c.params[0];
                const double& y = c.params[1];
                const double& r = c.params[2];
                double d = sqrt(pow((v.x - x), 2) + pow((v.y - y), 2));
                //std::cout << "d: " << d << ", r: " << r << ", eps: " << eps << '\n';
                if (d > r - eps && d < r + eps) {
                    //std::cout << "GOT it" << '\n';
                    obj_ = c;
                    state_ = State::Moving;
                    lastPos_ = v;
                    return;
                }
            }

        }
    }
    else if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Release) {
        state_ = State::WaitingStartMoving;
    }
}

void CursorTool::onKey(const input::KeyEvent& e) {
    (void)e;
}

void CursorTool::onCancel() {

}

