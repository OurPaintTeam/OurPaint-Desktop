#ifndef OURPAINT_LINE_CIRCLE_INTERSECTION_H
#define OURPAINT_LINE_CIRCLE_INTERSECTION_H

#include <algorithm>
#include <cmath>

#include "geometry/GeometryMath.h"
#include "geometry/intersections/IntersectionTypes.h"

namespace geometry::intersections {
inline IntersectionPoints lineCircle(const Line2& line, const Circle2& circle, bool lineAsSegment = true) {
    using namespace detail;
    if (!isFinite(line) || !isFinite(circle) || isDegenerate(line) || circle.radius < 0.0) {
        return {};
    }
    const Vec2 d = subtract(line.end, line.start);
    const Vec2 f = subtract(line.start, circle.center);
    const double a = lengthSquared(d);
    const double b = 2.0 * dot(f, d);
    const double c = lengthSquared(f) - circle.radius * circle.radius;
    double discriminant = b * b - 4.0 * a * c;
    const double tolerance = EPSILON * std::max({1.0, b * b, std::abs(4.0 * a * c)});
    if (discriminant < -tolerance) {
        return {};
    }
    discriminant = std::max(0.0, discriminant);
    const double root = std::sqrt(discriminant);
    const double t1 = (-b - root) / (2.0 * a);
    const double t2 = (-b + root) / (2.0 * a);
    IntersectionPoints result;
    const auto add = [&](double t) {
        if (lineAsSegment && !onSegment(t)) {
            return;
        }
        const Vec2 p{line.start.x + t * d.x, line.start.y + t * d.y};
        if (result.empty() || distanceSquared(result.front().position, p) > EPSILON * EPSILON) {
            result.push_back(Point2{p});
        }
    };
    add(t1);
    if (std::abs(t2 - t1) > EPSILON) {
        add(t2);
    }
    return result;
}
}  // namespace geometry::intersections

#endif  // OURPAINT_LINE_CIRCLE_INTERSECTION_H
