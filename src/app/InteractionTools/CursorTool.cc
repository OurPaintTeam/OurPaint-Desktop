#include "CursorTool.h"

#include "Document.h"
#include "Scene.h"

CursorTool::CursorTool(DocumentManager& documentManager, Camera2D& camera, renderer::RenderData& renderData)
    : documentManager_(documentManager), camera_(camera), renderData_(renderData) {
    objs_ = {};
}

void CursorTool::onMouseMove(const input::MouseMoveEvent& e) {
    if ((e.buttons & input::MouseButton::Left) != input::MouseButton::None) {
        if (state_ == State::Selected) {
            glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
            Scene& scene = documentManager_.getActiveDocument()->scene();
            double dx = v.x - lastPos_.x;
            double dy = v.y - lastPos_.y;
            for (const auto& obj : objs_) {
                if (obj.et == ObjType::ET_POINT) {
                    scene.movePoint(obj.id, dx, dy);
                }
                else if (obj.et == ObjType::ET_LINE) {
                    scene.moveLine(obj.id, dx, dy);
                }
                else if (obj.et == ObjType::ET_CIRCLE) {
                    scene.moveCircle(obj.id, dx, dy);
                }
            }
            lastPos_.x = v.x;
            lastPos_.y = v.y;
        }
    }
}

void CursorTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        glm::dvec2 v = camera_.screenToWorld({e.x, e.y});
        double eps = 0.05 / (camera_.zoom() / 100.0);
        Scene& scene = documentManager_.getActiveDocument()->scene();

        ObjectData obj;
        std::vector<ObjectData> points = scene.getPoints();
        for (const auto& p : points) {
            const double& x = p.params[0];
            const double& y = p.params[1];
            if (std::abs(x - v.x) < eps && std::abs(y - v.y) < eps) {
                obj = p;
                state_ = State::Selected;
                scene.movePoint(obj.id, v.x - x, v.y - y);
                lastPos_ = v;
                if (!ids_.contains(obj.id)) {
                    objs_.push_back(obj);
                    ids_.insert(obj.id);
                }
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
                    obj = l;
                    state_ = State::Selected;
                    //scene.moveLine(obj.id, v.x - lastPos_.x, v.y - lastPos_.y);
                    lastPos_ = v;
                    if (!ids_.contains(obj.id)) {
                        objs_.push_back(obj);
                        ids_.insert(obj.id);
                    }
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
                    obj = l;
                    state_ = State::Selected;
                    //scene.moveLine(obj.id, v.x - lastPos_.x, v.y - lastPos_.y);
                    lastPos_ = v;
                    if (!ids_.contains(obj.id)) {
                        objs_.push_back(obj);
                        ids_.insert(obj.id);
                    }
                    return;
                }
            }
        }

        std::vector<ObjectData> circles = scene.getCircles();
        for (const auto& c : circles) {
            const double& x = c.params[0];
            const double& y = c.params[1];
            const double& r = c.params[2];
            double dx = v.x - x;
            double dy = v.y - y;
            double d = sqrt(dx*dx + dy*dy);
            if (d > r - eps && d < r + eps) {
                obj = c;
                state_ = State::Selected;

                if (d > 1e-12) {
                    double shiftX = dx * (1.0 - r / d);
                    double shiftY = dy * (1.0 - r / d);
                    scene.moveCircle(c.id, shiftX, shiftY);
                }

                lastPos_ = v;
                if (!ids_.contains(obj.id)) {
                    objs_.push_back(obj);
                    ids_.insert(obj.id);
                }
                return;
            }
        }

        state_ = State::Waiting;
        ids_.clear();
        objs_.clear();
    }
}

//#include <QDebug>

void CursorTool::onKey(const input::KeyEvent& e) {
    if (e.key == input::KeyCode::Delete && e.action == input::KeyAction::Press) {
        if (state_ == State::Selected) {
            Scene& scene = documentManager_.getActiveDocument()->scene();
            for (const auto& obj : objs_) {
                bool result = scene.deleteObject(obj.id);
                if (!result) {
                    //qDebug() << "Failed delete: " << obj_.id.get();
                }
                else {
                    //qDebug() << "Success delete: " << obj_.id.get();
                }
            }
            state_ = State::Waiting;
        }
    }
}

bool CursorTool::cancel() {
    return true;
}

