#ifndef OURPAINT_CIRCLE_CIRCLE_INTERSECTION_H
#define OURPAINT_CIRCLE_CIRCLE_INTERSECTION_H

#include <algorithm>
#include <cmath>

#include "geometry/GeometryMath.h"
#include "geometry/intersections/IntersectionTypes.h"

namespace geometry::intersections {
inline IntersectionPoints circleCircle(const Circle2& a, const Circle2& b) {
    using namespace detail;
    if (!isFinite(a) || !isFinite(b) || a.radius < 0.0 || b.radius < 0.0) {
        return {};
    }
    const Vec2 delta = subtract(b.center, a.center);
    const double d2 = lengthSquared(delta);
    if (d2 <= EPSILON * EPSILON) {
        return {};  // concentric/coincident: no finite unique set
    }
    const double d = std::sqrt(d2);
    if (d > a.radius + b.radius + EPSILON || d < std::abs(a.radius - b.radius) - EPSILON) {
        return {};
    }
    const double x = (a.radius * a.radius - b.radius * b.radius + d2) / (2.0 * d);
    double h2 = a.radius * a.radius - x * x;
    if (h2 < -EPSILON * std::max(1.0, a.radius * a.radius)) {
        return {};
    }
    h2 = std::max(0.0, h2);
    const double h = std::sqrt(h2);
    const Vec2 base{a.center.x + x * delta.x / d, a.center.y + x * delta.y / d};
    const Vec2 offset{-delta.y * h / d, delta.x * h / d};
    IntersectionPoints result{Point2{Vec2{base.x + offset.x, base.y + offset.y}}};
    if (h > EPSILON) {
        result.push_back(Point2{Vec2{base.x - offset.x, base.y - offset.y}});
    }
    return result;
}
}  // namespace geometry::intersections

#endif  // OURPAINT_CIRCLE_CIRCLE_INTERSECTION_H
