#ifndef OURPAINT_ARC_ALGORITHMS_H
#define OURPAINT_ARC_ALGORITHMS_H

#include <numbers>
#include <optional>

#include "geometry/GeometryMath.h"

namespace geometry::detail {
inline double normalizeAngle(double angle) {
    constexpr double TWO_PI = 2.0 * std::numbers::pi;
    angle = std::fmod(angle, TWO_PI);
    if (angle < 0.0) {
        angle += TWO_PI;
    }
    return angle;
}
inline double arcSweep(const Arc2& arc) {
    const double startAngle = std::atan2(arc.start.y - arc.center.y, arc.start.x - arc.center.x);
    const double endAngle = std::atan2(arc.end.y - arc.center.y, arc.end.x - arc.center.x);
    return normalizeAngle(endAngle - startAngle);
}
inline std::optional<Vec2> closestPointOnArc(const Vec2& cursor, const Arc2& arc) {
    if (!isFinite(cursor) || !isFinite(arc)) {
        return std::nullopt;
    }
    const double startRadius = std::sqrt(distanceSquared(arc.center, arc.start));
    const double endRadius = std::sqrt(distanceSquared(arc.center, arc.end));
    if (startRadius <= EPSILON || endRadius <= EPSILON) {
        return std::nullopt;
    }
    const double radiusScale = std::max({1.0, startRadius, endRadius});
    if (std::abs(startRadius - endRadius) > EPSILON * radiusScale) {
        return std::nullopt;
    }
    const double sweep = arcSweep(arc);
    if (sweep <= EPSILON || sweep >= 2.0 * std::numbers::pi - EPSILON) {
        return std::nullopt;
    }
    const Vec2 offset = subtract(cursor, arc.center);
    if (lengthSquared(offset) <= EPSILON * EPSILON) {
        return arc.start;
    }
    const double startAngle = std::atan2(arc.start.y - arc.center.y, arc.start.x - arc.center.x);
    const double cursorAngle = std::atan2(offset.y, offset.x);
    if (normalizeAngle(cursorAngle - startAngle) <= sweep) {
        const double radius = (startRadius + endRadius) * 0.5;
        return Vec2{arc.center.x + radius * std::cos(cursorAngle), arc.center.y + radius * std::sin(cursorAngle)};
    }
    return distanceSquared(cursor, arc.start) <= distanceSquared(cursor, arc.end) ? std::optional<Vec2>{arc.start} : std::optional<Vec2>{arc.end};
}
}  // namespace geometry::detail

#endif  // OURPAINT_ARC_ALGORITHMS_H
