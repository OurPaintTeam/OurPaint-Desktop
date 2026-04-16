#include "Cpu2dPicker.h"

Cpu2dPicker::Cpu2dPicker(Scene& scene, Camera2D& camera) : scene_(scene), camera_(camera) {}

std::optional<PickResult> Cpu2dPicker::pickAt(double screenX, double screenY) const {
    glm::dvec2 v = camera_.screenToWorld({screenX, screenY});

    // Depends on zoom
    double eps = 0.05 / (camera_.zoom() / 100.0);

    std::vector<ObjectData> points = scene_.getPoints();
    for (const auto& p : points) {
        PickResult res;
        const double& x = p.params[0];
        const double& y = p.params[1];
        if (std::abs(x - v.x) < eps && std::abs(y - v.y) < eps) {
            res.type = ObjType::ET_POINT;
            res.id = p.id;
            return res;
        }
    }



    std::vector<ObjectData> lines_ = scene_.getLines();
    for (const auto& l : lines_) {
        PickResult res;

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
                res.type = ObjType::ET_LINE;
                res.id = l.id;
                return res;
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
                res.type = ObjType::ET_LINE;
                res.id = l.id;
                return res;
            }
        }
    }

    std::vector<ObjectData> circles_ = scene_.getCircles();
    for (const auto& c : circles_) {
        PickResult res;

        const double& x = c.params[0];
        const double& y = c.params[1];
        const double& r = c.params[2];
        double dx = v.x - x;
        double dy = v.y - y;
        double d = sqrt(dx*dx + dy*dy);
        if (d > r - eps && d < r + eps) {
            res.type = ObjType::ET_CIRCLE;
            res.id = c.id;
            return res;
        }
    }

    return std::nullopt;
}

bool lineIntersectsRectFast(double x1, double y1, double x2, double y2,
                            double xmin, double ymin, double xmax, double ymax) {
    // Быстрая проверка через ограничивающие прямоугольники
    if (std::max(x1, x2) < xmin || std::min(x1, x2) > xmax ||
        std::max(y1, y2) < ymin || std::min(y1, y2) > ymax) {
        return false;  // Тривиальное отклонение
        }

    // Проверка, что хотя бы один конец внутри
    if ((x1 >= xmin && x1 <= xmax && y1 >= ymin && y1 <= ymax) ||
        (x2 >= xmin && x2 <= xmax && y2 >= ymin && y2 <= ymax)) {
        return true;
        }

    // Проверка пересечения с каждой стороной
    auto intersect = [](double x1, double y1, double x2, double y2,
                        double x3, double y3, double x4, double y4) -> bool {
        double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (denom == 0) return false;

        double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
        double u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

        return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
    };

    // Проверка всех 4 сторон прямоугольника
    return intersect(x1, y1, x2, y2, xmin, ymin, xmax, ymin) ||  // bottom
           intersect(x1, y1, x2, y2, xmax, ymin, xmax, ymax) ||  // right
           intersect(x1, y1, x2, y2, xmax, ymax, xmin, ymax) ||  // top
           intersect(x1, y1, x2, y2, xmin, ymax, xmin, ymin);    // left
}

std::vector<SceneObjects::ID> Cpu2dPicker::pickInRect(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const {
    glm::dvec2 p1 = camera_.screenToWorld({screenMinX, screenMinY});
    glm::dvec2 p2 = camera_.screenToWorld({screenMaxX, screenMaxY});

    double rx1 = std::min(p1.x, p2.x);
    double ry1 = std::min(p1.y, p2.y);
    double rx2 = std::max(p1.x, p2.x);
    double ry2 = std::max(p1.y, p2.y);

    std::vector<SceneObjects::ID> res;

    // Points
    std::vector<ObjectData> points = scene_.getPoints();
    for (const auto& p : points) {
        const double& x = p.params[0];
        const double& y = p.params[1];
        if (x >= rx1 && x <= rx2 && y >= ry1 && y <= ry2) {
            res.push_back(p.id);
        }
    }

    // Lines
    std::vector<ObjectData> lines_ = scene_.getLines();
    for (const auto& l : lines_) {
        const double& x1 = l.params[0];
        const double& y1 = l.params[1];
        const double& x2 = l.params[2];
        const double& y2 = l.params[3];

        if (lineIntersectsRectFast(x1, y1, x2, y2, rx1, ry1, rx2, ry2)) {
            res.push_back(l.id);
        }
    }

    // Circles
    std::vector<ObjectData> circles_ = scene_.getCircles();
    for (const auto& c : circles_) {
        const double& cx = c.params[0];
        const double& cy = c.params[1];
        const double& radius = c.params[2];
        const double rSq = radius * radius;

        double closestX = std::max(rx1, std::min(cx, rx2));
        double closestY = std::max(ry1, std::min(cy, ry2));
        double dx = cx - closestX;
        double dy = cy - closestY;
        double distToRectSq = dx * dx + dy * dy;

        if (distToRectSq > rSq) {
            continue;
        }

        double farthestX = (std::abs(cx - rx1) > std::abs(cx - rx2)) ? rx1 : rx2;
        double farthestY = (std::abs(cy - ry1) > std::abs(cy - ry2)) ? ry1 : ry2;
        double dxFar = cx - farthestX;
        double dyFar = cy - farthestY;
        double distFarSq = dxFar * dxFar + dyFar * dyFar;

        if (distFarSq > rSq) {
            res.push_back(c.id);
        }
    }

    return res;
}

