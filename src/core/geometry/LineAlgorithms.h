#ifndef OURPAINT_LINE_ALGORITHMS_H
#define OURPAINT_LINE_ALGORITHMS_H

#include <optional>

#include "geometry/GeometryMath.h"

namespace geometry::detail {
inline std::optional<Vec2> closestPointOnSegment(const Vec2& cursor, const Line2& line) {
    if (!isFinite(cursor) || !isFinite(line)) {
        return std::nullopt;
    }
    const Vec2 direction = subtract(line.end, line.start);
    const double length2 = lengthSquared(direction);
    if (length2 <= EPSILON * EPSILON) {
        return line.start;
    }
    const Vec2 offset = subtract(cursor, line.start);
    const double t = std::clamp(dot(offset, direction) / length2, 0.0, 1.0);
    return Vec2{line.start.x + t * direction.x, line.start.y + t * direction.y};
}

struct LineIntersection {
    Vec2 point;
    double t = 0.0;
    double u = 0.0;
};
inline std::optional<LineIntersection> intersectLines(const Line2& a, const Line2& b) {
    if (!isFinite(a) || !isFinite(b) || isDegenerate(a) || isDegenerate(b)) {
        return std::nullopt;
    }
    const Vec2 da = subtract(a.end, a.start), db = subtract(b.end, b.start);
    const Vec2 offset = subtract(b.start, a.start);
    const double denominator = cross(da, db);
    if (std::abs(denominator) <= EPSILON) {
        return std::nullopt;
    }
    const double t = cross(offset, db) / denominator;
    const double u = cross(offset, da) / denominator;
    return LineIntersection{{a.start.x + t * da.x, a.start.y + t * da.y}, t, u};
}
}  // namespace geometry::detail

#endif  // OURPAINT_LINE_ALGORITHMS_H
