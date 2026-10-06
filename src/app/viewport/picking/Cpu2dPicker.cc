#include "Cpu2dPicker.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace {

bool pointInRect(sketch::Vec2 point, double xmin, double ymin, double xmax, double ymax) {
    return point.x >= xmin && point.x <= xmax && point.y >= ymin && point.y <= ymax;
}

bool angleInArc(const sketch::Arc2& arc, sketch::Vec2 point, double tolerance = 1e-12) {
    const double tau = 2 * std::numbers::pi;
    const auto positiveAngle = [tau](double angle) {
        angle = std::fmod(angle, tau);
        return angle < 0 ? angle + tau : angle;
    };
    const double start = std::atan2(arc.start.y - arc.center.y, arc.start.x - arc.center.x);
    const double end = std::atan2(arc.end.y - arc.center.y, arc.end.x - arc.center.x);
    const double angle = std::atan2(point.y - arc.center.y, point.x - arc.center.x);
    const double relative = positiveAngle(angle - start);
    return relative <= positiveAngle(end - start) + tolerance || relative >= tau - tolerance;
}

bool arcIntersectsRect(const sketch::Arc2& arc, double xmin, double ymin, double xmax, double ymax) {
    if (pointInRect(arc.start, xmin, ymin, xmax, ymax) || pointInRect(arc.end, xmin, ymin, xmax, ymax)) {
        return true;
    }
    const double radius = std::hypot(arc.start.x - arc.center.x, arc.start.y - arc.center.y);
    if (!std::isfinite(radius) || radius <= 0) {
        return false;
    }
    const auto hits = [&](sketch::Vec2 point) { return pointInRect(point, xmin, ymin, xmax, ymax) && angleInArc(arc, point); };
    for (double x : {xmin, xmax}) {
        const double dx = x - arc.center.x;
        if (std::abs(dx) <= radius) {
            const double ratio = dx / radius;
            const double dy = radius * std::sqrt(std::max(0.0, 1 - ratio * ratio));
            if (hits({x, arc.center.y - dy}) || hits({x, arc.center.y + dy})) {
                return true;
            }
        }
    }
    for (double y : {ymin, ymax}) {
        const double dy = y - arc.center.y;
        if (std::abs(dy) <= radius) {
            const double ratio = dy / radius;
            const double dx = radius * std::sqrt(std::max(0.0, 1 - ratio * ratio));
            if (hits({arc.center.x - dx, y}) || hits({arc.center.x + dx, y})) {
                return true;
            }
        }
    }
    return false;
}

}  // namespace

Cpu2dPicker::Cpu2dPicker(sketch::Sketch& sketch, Camera2D& camera) : sketch_(sketch), camera_(camera) {}

// Pick in rect

std::vector<sketch::GeometryRef> Cpu2dPicker::pickInRectAtScreenLogical(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const {
    glm::dvec2 worldP1 = camera_.screenLogicalToWorld({screenMinX, screenMinY});
    glm::dvec2 worldP2 = camera_.screenLogicalToWorld({screenMaxX, screenMaxY});
    return pickInRect(worldP1.x, worldP1.y, worldP2.x, worldP2.y);
}

std::vector<sketch::GeometryRef> Cpu2dPicker::pickInRectAtScreenFramebuffer(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const {
    glm::dvec2 worldP1 = camera_.screenFramebufferToWorld({screenMinX, screenMinY});
    glm::dvec2 worldP2 = camera_.screenFramebufferToWorld({screenMaxX, screenMaxY});
    return pickInRect(worldP1.x, worldP1.y, worldP2.x, worldP2.y);
}

std::vector<sketch::GeometryRef> Cpu2dPicker::pickInRectAtWorld(double screenMinX, double screenMinY, double screenMaxX, double screenMaxY) const {
    return pickInRect(screenMinX, screenMinY, screenMaxX, screenMaxY);
}

// Pick at screen logical

std::optional<PickResult> Cpu2dPicker::pickAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickAt(world.x, world.y);
}

std::optional<PickResult> Cpu2dPicker::pickPointAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickPointAt(world.x, world.y);
}

std::optional<PickResult> Cpu2dPicker::pickCurveAtScreenLogical(double screenX, double screenY) const {
    const glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    if (const auto line = pickLineAt(world.x, world.y)) {
        return line;
    }
    if (const auto circle = pickCircleAt(world.x, world.y)) {
        return circle;
    }
    return pickArcAt(world.x, world.y);
}

std::optional<PickResult> Cpu2dPicker::pickLineAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickLineAt(world.x, world.y);
}

std::optional<PickResult> Cpu2dPicker::pickCircleAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickCircleAt(world.x, world.y);
}

// Pick at screen framebuffer

std::optional<PickResult> Cpu2dPicker::pickAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickAt(world.x, world.y);
}

std::optional<PickResult> Cpu2dPicker::pickPointAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickPointAt(world.x, world.y);
}

std::optional<PickResult> Cpu2dPicker::pickLineAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickLineAt(world.x, world.y);
}

std::optional<PickResult> Cpu2dPicker::pickCircleAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickCircleAt(world.x, world.y);
}

// Pick at world

std::optional<PickResult> Cpu2dPicker::pickAtWorld(double worldX, double worldY) const {
    return pickAt(worldX, worldY);
}

std::optional<PickResult> Cpu2dPicker::pickPointAtWorld(double worldX, double worldY) const {
    return pickPointAt(worldX, worldY);
}

std::optional<PickResult> Cpu2dPicker::pickLineAtWorld(double worldX, double worldY) const {
    return pickLineAt(worldX, worldY);
}

std::optional<PickResult> Cpu2dPicker::pickCircleAtWorld(double worldX, double worldY) const {
    return pickCircleAt(worldX, worldY);
}



// Private

std::vector<sketch::GeometryRef> Cpu2dPicker::pickInRect(double worldMinX, double worldMinY, double worldMaxX, double worldMaxY) const {
    const double rx1 = std::min(worldMinX, worldMaxX);
    const double ry1 = std::min(worldMinY, worldMaxY);
    const double rx2 = std::max(worldMinX, worldMaxX);
    const double ry2 = std::max(worldMinY, worldMaxY);

    auto result = sketch_.entities();
    auto points = sketch_.pointElements();
    if (!result || !points) {
        return {};
    }

    std::vector<sketch::GeometryRef> selected;
    for (const auto& point : points.value()) {
        if (pointInRect(point.position, rx1, ry1, rx2, ry2)) {
            selected.push_back(point.ref);
        }
    }
    for (const auto& entity : result.value()) {
        if (const auto* line = std::get_if<sketch::Line2>(&entity.geometry)) {
            if (lineIntersectsRectFast(line->start.x, line->start.y, line->end.x, line->end.y, rx1, ry1, rx2, ry2)) {
                selected.push_back({entity.id, sketch::SubElement::Whole});
            }
        } else if (const auto* circle = std::get_if<sketch::Circle2>(&entity.geometry)) {
            const double rSq = circle->radius * circle->radius;
            const double closestX = std::clamp(circle->center.x, rx1, rx2);
            const double closestY = std::clamp(circle->center.y, ry1, ry2);
            const double dx = circle->center.x - closestX;
            const double dy = circle->center.y - closestY;
            if (dx * dx + dy * dy > rSq) {
                continue;
            }

            const double farthestX = std::abs(circle->center.x - rx1) > std::abs(circle->center.x - rx2) ? rx1 : rx2;
            const double farthestY = std::abs(circle->center.y - ry1) > std::abs(circle->center.y - ry2) ? ry1 : ry2;
            const double dxFar = circle->center.x - farthestX;
            const double dyFar = circle->center.y - farthestY;
            if (dxFar * dxFar + dyFar * dyFar >= rSq) {
                selected.push_back({entity.id, sketch::SubElement::Whole});
            }
        } else if (const auto* arc = std::get_if<sketch::Arc2>(&entity.geometry)) {
            if (arcIntersectsRect(*arc, rx1, ry1, rx2, ry2)) {
                selected.push_back({entity.id, sketch::SubElement::Whole});
            }
        }
    }

    return selected;
}

std::optional<PickResult> Cpu2dPicker::pickAt(double worldX, double worldY) const {
    std::optional<PickResult> p = pickPointAt(worldX, worldY);
    if (p.has_value()) {
        return p;
    }

    std::optional<PickResult> l = pickLineAt(worldX, worldY);
    if (l.has_value()) {
        return l;
    }

    std::optional<PickResult> c = pickCircleAt(worldX, worldY);
    if (c.has_value()) {
        return c;
    }

    return pickArcAt(worldX, worldY);
}

std::optional<PickResult> Cpu2dPicker::pickPointAt(double worldX, double worldY) const {
    const double eps = 0.05 / (camera_.zoom() / 100.0);
    auto result = sketch_.pointElements();
    if (!result) {
        return {};
    }
    for (const auto& point : result.value()) {
        if (std::abs(point.position.x - worldX) < eps && std::abs(point.position.y - worldY) < eps) {
            return PickResult{point.ref};
        }
    }
    return std::nullopt;
}

std::optional<PickResult> Cpu2dPicker::pickLineAt(double worldX, double worldY) const {
    const double eps = 0.05 / (camera_.zoom() / 100.0);
    auto result = sketch_.lines();
    if (!result) {
        return {};
    }
    for (const auto& entity : result.value()) {
        const auto* line = std::get_if<sketch::Line2>(&entity.geometry);
        if (!line) {
            continue;
        }

        const double dx = line->end.x - line->start.x;
        const double dy = line->end.y - line->start.y;
        const double px = worldX - line->start.x;
        const double py = worldY - line->start.y;
        const double cross = std::abs(dx * py - dy * px);
        const double len = std::hypot(dx, dy);
        if (len < 1e-9) {
            if (std::abs(worldX - line->start.x) < eps && std::abs(worldY - line->start.y) < eps) {
                return PickResult{{entity.id, sketch::SubElement::Whole}};
            }
            continue;
        }

        const double dist = cross / len;
        if (dist < eps) {
            const double minX = std::min(line->start.x, line->end.x) - eps;
            const double maxX = std::max(line->start.x, line->end.x) + eps;
            const double minY = std::min(line->start.y, line->end.y) - eps;
            const double maxY = std::max(line->start.y, line->end.y) + eps;
            if (worldX >= minX && worldX <= maxX && worldY >= minY && worldY <= maxY) {
                return PickResult{{entity.id, sketch::SubElement::Whole}};
            }
        }
    }
    return std::nullopt;
}

std::optional<PickResult> Cpu2dPicker::pickCircleAt(double worldX, double worldY) const {
    const double eps = 0.05 / (camera_.zoom() / 100.0);
    auto result = sketch_.circles();
    if (!result) {
        return {};
    }
    for (const auto& entity : result.value()) {
        const auto* circle = std::get_if<sketch::Circle2>(&entity.geometry);
        if (!circle) {
            continue;
        }

        const double dx = worldX - circle->center.x;
        const double dy = worldY - circle->center.y;
        const double distance = std::hypot(dx, dy);
        if (distance > circle->radius - eps && distance < circle->radius + eps) {
            return PickResult{{entity.id, sketch::SubElement::Whole}};
        }
    }
    return std::nullopt;
}

std::optional<PickResult> Cpu2dPicker::pickArcAt(double worldX, double worldY) const {
    const double eps = 0.05 / (camera_.zoom() / 100.0);
    auto result = sketch_.arcs();
    if (!result) {
        return std::nullopt;
    }
    for (const auto& entity : result.value()) {
        const auto& arc = std::get<sketch::Arc2>(entity.geometry);
        const double radius = std::hypot(arc.start.x - arc.center.x, arc.start.y - arc.center.y);
        const double distance = std::hypot(worldX - arc.center.x, worldY - arc.center.y);
        if (radius > 0 && std::abs(distance - radius) < eps && angleInArc(arc, {worldX, worldY}, eps / radius)) {
            return PickResult{{entity.id, sketch::SubElement::Whole}};
        }
    }
    return std::nullopt;
}

bool Cpu2dPicker::lineIntersectsRectFast(double x1, double y1, double x2, double y2,
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

