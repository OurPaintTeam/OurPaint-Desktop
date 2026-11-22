#ifndef OURPAINT_GEOMETRY_RELATIONS_POINT_CURVE_TANGENCY_H
#define OURPAINT_GEOMETRY_RELATIONS_POINT_CURVE_TANGENCY_H

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

#include "geometry/ArcAlgorithms.h"
#include "geometry/CircleAlgorithms.h"

namespace geometry::relations {

struct PointCurveTangentCandidate {
    Vec2 point;
    double cursorDistanceSquared = 0.0;
};

inline bool pointOnArcSweep(const Vec2& point, const Arc2& arc, double epsilon = 1e-8) {
    const double startAngle = std::atan2(arc.start.y - arc.center.y, arc.start.x - arc.center.x);
    const double pointAngle = std::atan2(point.y - arc.center.y, point.x - arc.center.x);
    const double sweep = detail::arcSweep(arc);
    return detail::normalizeAngle(pointAngle - startAngle) <= sweep + epsilon;
}

inline std::vector<PointCurveTangentCandidate> pointTangenciesToCircle(
    const Vec2& anchor, const Vec2& cursor, const Vec2& center, double radius) {
    std::vector<PointCurveTangentCandidate> result;
    if (!detail::isFinite(anchor) || !detail::isFinite(cursor) || !detail::isFinite(center) ||
        !std::isfinite(radius) || radius <= EPSILON) {
        return result;
    }

    const Vec2 offset = detail::subtract(center, anchor);
    const double distanceSquared = detail::lengthSquared(offset);
    const double radiusSquared = radius * radius;
    if (distanceSquared <= radiusSquared + EPSILON) {
        return result;
    }

    const double centerAngle = std::atan2(offset.y, offset.x);
    const double alpha = std::acos(std::clamp(radius / std::sqrt(distanceSquared), -1.0, 1.0));
    for (const double sign : {-1.0, 1.0}) {
        const double contactAngle = centerAngle + std::numbers::pi + sign * alpha;
        const Vec2 tangentPoint{center.x + radius * std::cos(contactAngle),
                                center.y + radius * std::sin(contactAngle)};
        result.push_back({tangentPoint, detail::distanceSquared(cursor, tangentPoint)});
    }
    return result;
}

inline std::vector<PointCurveTangentCandidate> pointTangenciesToArc(
    const Vec2& anchor, const Vec2& cursor, const Arc2& arc) {
    const double radius = std::sqrt(detail::distanceSquared(arc.center, arc.start));
    auto candidates = pointTangenciesToCircle(anchor, cursor, arc.center, radius);
    candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [&](const auto& candidate) {
        return !pointOnArcSweep(candidate.point, arc);
    }), candidates.end());
    return candidates;
}

}  // namespace geometry::relations

#endif  // OURPAINT_GEOMETRY_RELATIONS_POINT_CURVE_TANGENCY_H
