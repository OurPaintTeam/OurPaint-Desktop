#ifndef OURPAINT_ARC_INTERSECTION_H
#define OURPAINT_ARC_INTERSECTION_H

#include <cmath>

#include "geometry/ArcAlgorithms.h"
#include "geometry/intersections/CircleCircleIntersection.h"
#include "geometry/intersections/LineCircleIntersection.h"

namespace geometry::intersections {
inline bool pointOnArc(const Vec2& point, const Arc2& arc) {
    using namespace detail;
    if (!isFinite(point) || !isFinite(arc)) {
        return false;
    }
    const double r0 = std::sqrt(distanceSquared(arc.center, arc.start));
    const double r1 = std::sqrt(distanceSquared(arc.center, arc.end));
    const double rp = std::sqrt(distanceSquared(arc.center, point));
    if (r0 <= EPSILON || std::abs(r0 - r1) > EPSILON * std::max(1.0, r0) || std::abs(r0 - rp) > EPSILON * std::max(1.0, r0)) {
        return false;
    }
    const double start = std::atan2(arc.start.y - arc.center.y, arc.start.x - arc.center.x);
    const double angle = std::atan2(point.y - arc.center.y, point.x - arc.center.x);
    const double sweep = arcSweep(arc);
    return sweep > EPSILON && sweep < 2.0 * std::numbers::pi - EPSILON && normalizeAngle(angle - start) <= sweep + EPSILON;
}

inline Circle2 supportingCircle(const Arc2& arc) { return Circle2{arc.center, std::sqrt(detail::distanceSquared(arc.center, arc.start))}; }

inline IntersectionPoints lineArc(const Line2& line, const Arc2& arc, bool lineAsSegment = true) {
    auto candidates = lineCircle(line, supportingCircle(arc), lineAsSegment);
    IntersectionPoints result;
    for (const auto& p : candidates) {
        if (pointOnArc(Vec2{p.position}, arc)) {
            result.push_back(p);
        }
    }
    return result;
}

inline IntersectionPoints circleArc(const Circle2& circle, const Arc2& arc) {
    auto candidates = circleCircle(circle, supportingCircle(arc));
    IntersectionPoints result;
    for (const auto& p : candidates) {
        if (pointOnArc(Vec2{p.position}, arc)) {
            result.push_back(p);
        }
    }
    return result;
}

inline IntersectionPoints arcArc(const Arc2& a, const Arc2& b) {
    auto candidates = circleCircle(supportingCircle(a), supportingCircle(b));
    IntersectionPoints result;
    for (const auto& p : candidates) {
        const Vec2 v{p.position};
        if (pointOnArc(v, a) && pointOnArc(v, b)) {
            result.push_back(p);
        }
    }
    return result;
}
}  // namespace geometry::intersections

#endif  // OURPAINT_ARC_INTERSECTION_H
