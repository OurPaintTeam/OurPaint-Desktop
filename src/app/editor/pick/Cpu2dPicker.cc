#include "Cpu2dPicker.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <numbers>

#include "../NurbsPolyline.h"
#include "../render/constraints/ConstraintLayout.h"

namespace {

bool isDimension(sketch::ConstraintType type) {
    switch (type) {
        case sketch::ConstraintType::Distance:
        case sketch::ConstraintType::HorizontalDistance:
        case sketch::ConstraintType::VerticalDistance:
        case sketch::ConstraintType::Length:
        case sketch::ConstraintType::Radius:
        case sketch::ConstraintType::Diameter:
        case sketch::ConstraintType::Angle:
            return true;
        default:
            return false;
    }
}

double segmentDistance(glm::dvec2 point, glm::dvec2 start, glm::dvec2 end) {
    const auto direction = end - start;
    const double lengthSquared = glm::dot(direction, direction);
    const double t = lengthSquared > 0.0 ? std::clamp(glm::dot(point - start, direction) / lengthSquared, 0.0, 1.0) : 0.0;
    return glm::length(point - (start + t * direction));
}

double arcDistance(glm::dvec2 point, const app::ConstraintArc& arc) {
    const double tau = 2.0 * std::numbers::pi;
    const auto positiveAngle = [tau](double angle) {
        const double result = std::fmod(angle, tau);
        return result < 0.0 ? result + tau : result;
    };
    if (!std::isfinite(arc.radiusPx) || arc.radiusPx <= 0.0 || !std::isfinite(arc.startAngle) || !std::isfinite(arc.endAngle)) {
        return std::numeric_limits<double>::infinity();
    }
    const auto delta = point - arc.center;
    const double sweep = positiveAngle(arc.endAngle - arc.startAngle);
    const double relative = positiveAngle(std::atan2(-delta.y, delta.x) - arc.startAngle);
    if (sweep < 1e-5 || relative <= sweep) {
        return std::abs(glm::length(delta) - arc.radiusPx);
    }
    const auto endpoint = [&](double angle) { return arc.center + arc.radiusPx * glm::dvec2(std::cos(angle), -std::sin(angle)); };
    return std::min(glm::length(point - endpoint(arc.startAngle)), glm::length(point - endpoint(arc.endAngle)));
}

double labelDistance(glm::dvec2 point, const app::ConstraintLabel& label) {
    // Undo the label's rotation before testing its local measured rectangle.
    const glm::dvec2 tangent{std::cos(label.rotationRad), std::sin(label.rotationRad)};
    const glm::dvec2 normal{-tangent.y, tangent.x};
    const auto delta = point - label.center;
    const double dx = std::max(0.0, std::abs(glm::dot(delta, tangent)) - label.sizePx.x * 0.5);
    const double dy = std::max(0.0, std::abs(glm::dot(delta, normal)) - label.sizePx.y * 0.5);
    return std::hypot(dx, dy);
}

bool pointInRect(sketch::Vec2 point, double xmin, double ymin, double xmax, double ymax) {
    return point.x >= xmin && point.x <= xmax && point.y >= ymin && point.y <= ymax;
}

bool circleIntersectsRect(const sketch::Circle2& circle, double xmin, double ymin, double xmax, double ymax) {
    const double nearX = std::clamp(circle.center.x, xmin, xmax) - circle.center.x;
    const double nearY = std::clamp(circle.center.y, ymin, ymax) - circle.center.y;
    const double farX = std::max(std::abs(xmin - circle.center.x), std::abs(xmax - circle.center.x));
    const double farY = std::max(std::abs(ymin - circle.center.y), std::abs(ymax - circle.center.y));
    return std::hypot(nearX, nearY) <= circle.radius && std::hypot(farX, farY) >= circle.radius;
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

bool arcInRect(const sketch::Arc2& arc, double xmin, double ymin, double xmax, double ymax) {
    if (!pointInRect(arc.start, xmin, ymin, xmax, ymax) || !pointInRect(arc.end, xmin, ymin, xmax, ymax)) {
        return false;
    }
    const double radius = std::hypot(arc.start.x - arc.center.x, arc.start.y - arc.center.y);
    if (!std::isfinite(radius) || radius <= 0.0) {
        return false;
    }

    // Only cardinal points on the arc's sweep can extend its bounds beyond the endpoints.
    const std::array<sketch::Vec2, 4> extrema{{
        {arc.center.x - radius, arc.center.y}, {arc.center.x + radius, arc.center.y},
        {arc.center.x, arc.center.y - radius}, {arc.center.x, arc.center.y + radius},
    }};
    for (const auto point : extrema) {
        if (!pointInRect(point, xmin, ymin, xmax, ymax) && angleInArc(arc, point)) {
            return false;
        }
    }
    return true;
}

}  // namespace

Cpu2dPicker::Cpu2dPicker(sketch::Sketch& sketch, render::Camera2D& camera) : sketch_(sketch), camera_(camera) {}

std::optional<DimensionPickResult> Cpu2dPicker::pickDimensionAtScreenLogical(double screenX, double screenY, const app::ConstraintLayout& layout) const {
    if (!std::isfinite(screenX) || !std::isfinite(screenY)) {
        return std::nullopt;
    }
    const glm::dvec2 point{screenX, screenY};
    std::optional<DimensionPickResult> best;
    for (const auto& marker : layout.markers()) {
        // Each dimension currently has one independently placed label and one owner.
        if (!isDimension(marker.type) || marker.constraints.size() != 1 || marker.labels.size() != 1 || !std::isfinite(marker.hitRadiusPx) ||
            marker.hitRadiusPx < 0.0) {
            continue;
        }
        const auto& label = marker.labels.front();
        if (!std::isfinite(label.center.x) || !std::isfinite(label.center.y) || !std::isfinite(label.rotationRad) || !std::isfinite(label.sizePx.x) ||
            !std::isfinite(label.sizePx.y) || label.sizePx.x <= 0.0 || label.sizePx.y <= 0.0) {
            continue;
        }
        auto part = DimensionPickResult::Part::Label;
        double distance = labelDistance(point, label);
        if (distance > marker.hitRadiusPx) {
            part = DimensionPickResult::Part::Body;
            distance = std::numeric_limits<double>::infinity();
            for (const auto& stroke : marker.strokes) {
                distance = std::min(distance, segmentDistance(point, stroke.start, stroke.end));
            }
            for (const auto& arc : marker.arcs) {
                distance = std::min(distance, arcDistance(point, arc));
            }
        }
        if (distance <= marker.hitRadiusPx &&
            (!best || part < best->part ||
             (part == best->part && (distance < best->distancePx || (distance == best->distancePx && marker.constraints.front() < best->constraint))))) {
            best = DimensionPickResult{marker.constraints.front(), part, label.center, distance};
        }
    }
    return best;
}

// Pick in rect

std::vector<sketch::GeometryRef> Cpu2dPicker::pickInRectAtScreenLogical(double x1, double y1, double x2, double y2, app::RectSelectionMode mode) const {
    const auto worldP1 = camera_.screenLogicalToWorld({x1, y1});
    const auto worldP2 = camera_.screenLogicalToWorld({x2, y2});
    return pickInRectAtWorld(worldP1.x, worldP1.y, worldP2.x, worldP2.y, mode);
}

std::vector<sketch::GeometryRef> Cpu2dPicker::pickInRectAtScreenFramebuffer(double x1, double y1, double x2, double y2, app::RectSelectionMode mode) const {
    const auto worldP1 = camera_.screenFramebufferToWorld({x1, y1});
    const auto worldP2 = camera_.screenFramebufferToWorld({x2, y2});
    return pickInRectAtWorld(worldP1.x, worldP1.y, worldP2.x, worldP2.y, mode);
}

// Pick at screen logical

std::optional<PickResult> Cpu2dPicker::pickAtScreenLogical(double screenX, double screenY, const app::ConstraintLayout& layout) const {
    if (const auto dimension = pickSelectableDimensionAtScreenLogical(screenX, screenY, layout)) {
        return *dimension;
    }
    return pickGeometryAtScreenLogical(screenX, screenY);
}

std::optional<DimensionPickResult> Cpu2dPicker::pickSelectableDimensionAtScreenLogical(double screenX, double screenY, const app::ConstraintLayout& layout) const {
    const auto dimension = pickDimensionAtScreenLogical(screenX, screenY, layout);
    if (dimension && dimension->part == DimensionPickResult::Part::Body && pickPointAtScreenLogical(screenX, screenY)) {
        return std::nullopt;
    }
    return dimension;
}

std::optional<GeometryPickResult> Cpu2dPicker::pickGeometryAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickPointAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickPointAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickCurveAtScreenLogical(double screenX, double screenY) const {
    const glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickCurveAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickLineAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickLineAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickCircleAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickCircleAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickArcAtScreenLogical(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenLogicalToWorld({screenX, screenY});
    return pickArcAt(world.x, world.y);
}

// Pick at screen framebuffer

std::optional<GeometryPickResult> Cpu2dPicker::pickAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickPointAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickPointAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickLineAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickLineAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickCircleAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickCircleAt(world.x, world.y);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickArcAtScreenFramebuffer(double screenX, double screenY) const {
    glm::dvec2 world = camera_.screenFramebufferToWorld({screenX, screenY});
    return pickArcAt(world.x, world.y);
}

// Pick at world

std::optional<GeometryPickResult> Cpu2dPicker::pickAtWorld(double worldX, double worldY) const {
    return pickAt(worldX, worldY);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickPointAtWorld(double worldX, double worldY) const {
    return pickPointAt(worldX, worldY);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickLineAtWorld(double worldX, double worldY) const {
    return pickLineAt(worldX, worldY);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickCircleAtWorld(double worldX, double worldY) const {
    return pickCircleAt(worldX, worldY);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickArcAtWorld(double worldX, double worldY) const {
    return pickArcAt(worldX, worldY);
}

std::vector<sketch::GeometryRef> Cpu2dPicker::pickInRectAtWorld(double x1, double y1, double x2, double y2, app::RectSelectionMode mode) const {
    const double rx1 = std::min(x1, x2);
    const double ry1 = std::min(y1, y2);
    const double rx2 = std::max(x1, x2);
    const double ry2 = std::max(y1, y2);
    const bool window = mode == app::RectSelectionMode::Window;

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
        bool picked = false;
        if (const auto* line = std::get_if<sketch::Line2>(&entity.geometry)) {
            picked = window ? pointInRect(line->start, rx1, ry1, rx2, ry2) && pointInRect(line->end, rx1, ry1, rx2, ry2)
                            : lineIntersectsRectFast(line->start.x, line->start.y, line->end.x, line->end.y, rx1, ry1, rx2, ry2);
        } else if (const auto* circle = std::get_if<sketch::Circle2>(&entity.geometry)) {
            const auto [center, radius] = *circle;
            picked = window ? center.x - radius >= rx1 && center.y - radius >= ry1 && center.x + radius <= rx2 && center.y + radius <= ry2
                            : circleIntersectsRect(*circle, rx1, ry1, rx2, ry2);
        } else if (const auto* arc = std::get_if<sketch::Arc2>(&entity.geometry)) {
            picked = window ? arcInRect(*arc, rx1, ry1, rx2, ry2) : arcIntersectsRect(*arc, rx1, ry1, rx2, ry2);
        } else if (const auto* nurbs = std::get_if<sketch::NurbsCurve2>(&entity.geometry)) {
            const auto samples = app::sampleNurbsPolyline(*nurbs);
            if (window) {
                picked = !samples.empty() && std::ranges::all_of(samples, [&](auto p) { return pointInRect(p, rx1, ry1, rx2, ry2); });
            } else {
                picked = std::ranges::adjacent_find(samples, [&](auto a, auto b) {
                    return lineIntersectsRectFast(a.x, a.y, b.x, b.y, rx1, ry1, rx2, ry2);
                }) != samples.end();
            }
        }
        if (picked) {
            selected.push_back({entity.id, sketch::SubElement::Whole});
        }
    }

    return selected;
}

// Private

std::optional<GeometryPickResult> Cpu2dPicker::pickAt(double worldX, double worldY) const {
    if (const auto point = pickPointAt(worldX, worldY)) {
        return point;
    }
    return pickCurveAt(worldX, worldY);
}

std::optional<GeometryPickResult> Cpu2dPicker::pickCurveAt(double worldX, double worldY) const {
    if (const auto line = pickLineAt(worldX, worldY)) {
        return line;
    }
    if (const auto circle = pickCircleAt(worldX, worldY)) {
        return circle;
    }
    if (const auto arc = pickArcAt(worldX, worldY)) {
        return arc;
    }
    const auto nurbs = sketch_.entities(sketch::EntityKind::Nurbs);
    if (!nurbs) {
        return std::nullopt;
    }
    const double tolerance = 5.0 / camera_.zoom();
    for (const auto& entity : nurbs.value()) {
        const auto samples = app::sampleNurbsPolyline(std::get<sketch::NurbsCurve2>(entity.geometry));
        const auto hit = std::ranges::adjacent_find(samples, [&](auto a, auto b) {
            return segmentDistance({worldX, worldY}, {a.x, a.y}, {b.x, b.y}) < tolerance;
        });
        if (hit != samples.end()) {
            return GeometryPickResult{{entity.id, sketch::SubElement::Whole}};
        }
    }
    return std::nullopt;
}

std::optional<GeometryPickResult> Cpu2dPicker::pickPointAt(double worldX, double worldY) const {
    const double eps = 0.05 / (camera_.zoom() / 100.0);
    auto result = sketch_.pointElements();
    if (!result) {
        return {};
    }
    for (const auto& point : result.value()) {
        if (std::abs(point.position.x - worldX) < eps && std::abs(point.position.y - worldY) < eps) {
            return GeometryPickResult{point.ref};
        }
    }
    return std::nullopt;
}

std::optional<GeometryPickResult> Cpu2dPicker::pickLineAt(double worldX, double worldY) const {
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
                return GeometryPickResult{{entity.id, sketch::SubElement::Whole}};
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
                return GeometryPickResult{{entity.id, sketch::SubElement::Whole}};
            }
        }
    }
    return std::nullopt;
}

std::optional<GeometryPickResult> Cpu2dPicker::pickCircleAt(double worldX, double worldY) const {
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
            return GeometryPickResult{{entity.id, sketch::SubElement::Whole}};
        }
    }
    return std::nullopt;
}

std::optional<GeometryPickResult> Cpu2dPicker::pickArcAt(double worldX, double worldY) const {
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
            return GeometryPickResult{{entity.id, sketch::SubElement::Whole}};
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

