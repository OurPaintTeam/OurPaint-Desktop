#ifndef OURPAINT_LINE_LINE_INTERSECTION_H
#define OURPAINT_LINE_LINE_INTERSECTION_H

#include <optional>

#include "geometry/GeometryMath.h"
#include "geometry/intersections/IntersectionTypes.h"

namespace geometry::intersections {
struct LineHit {
    Vec2 point;
    double t = 0.0;
    double u = 0.0;
};

inline std::optional<LineHit> intersectLines(const Line2& a, const Line2& b, bool segmentsOnly = true) {
    using namespace detail;
    if (!isFinite(a) || !isFinite(b) || isDegenerate(a) || isDegenerate(b)) {
        return std::nullopt;
    }
    const Vec2 da = subtract(a.end, a.start);
    const Vec2 db = subtract(b.end, b.start);
    const Vec2 offset = subtract(b.start, a.start);
    const double denominator = cross(da, db);
    const double scale = std::sqrt(lengthSquared(da) * lengthSquared(db));
    if (std::abs(denominator) <= EPSILON * std::max(1.0, scale)) {
        return std::nullopt;
    }
    const double t = cross(offset, db) / denominator;
    const double u = cross(offset, da) / denominator;
    if (segmentsOnly && (!onSegment(t) || !onSegment(u))) {
        return std::nullopt;
    }
    return LineHit{{a.start.x + t * da.x, a.start.y + t * da.y}, t, u};
}

inline IntersectionPoints lineLine(const Line2& a, const Line2& b, bool segmentsOnly = true) {
    const auto hit = intersectLines(a, b, segmentsOnly);
    if (!hit) {
        return {};
    }
    return {Point2{hit->point}};
}
}  // namespace geometry::intersections

#endif  // OURPAINT_LINE_LINE_INTERSECTION_H
