#ifndef OURPAINT_CIRCLE_ALGORITHMS_H
#define OURPAINT_CIRCLE_ALGORITHMS_H

#include <optional>

#include "geometry/GeometryMath.h"

namespace geometry::detail {
inline std::optional<Vec2> closestPointOnCircle(const Vec2& cursor, const Circle2& circle) {
    if (!isFinite(cursor) || !isFinite(circle)) {
        return std::nullopt;
    }
    const Vec2 offset = subtract(cursor, circle.center);
    const double length2 = lengthSquared(offset);
    if (length2 <= EPSILON * EPSILON) {
        return Vec2{circle.center.x + circle.radius, circle.center.y};
    }
    const double scale = circle.radius / std::sqrt(length2);
    return Vec2{circle.center.x + offset.x * scale, circle.center.y + offset.y * scale};
}

}  // namespace geometry::detail

#endif  // OURPAINT_CIRCLE_ALGORITHMS_H
