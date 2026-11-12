#ifndef OURPAINT_SNAP_QUERY_TYPES_H
#define OURPAINT_SNAP_QUERY_TYPES_H

#include <optional>

#include "geometry/GeometryTypes.h"

namespace geometry {
struct PointCandidate {
    Vec2 point;
    double distanceSquared = 0.0;
    GeometryRef source;
};
struct IntersectionCandidate {
    Vec2 point;
    double distanceSquared = 0.0;
    GeometryRef first;
    GeometryRef second;
};

// A possible extension snap. Extension segments are finite drawing hints from
// the original endpoint to the chosen intersection point, never infinite lines.
struct ExtendedIntersectionCandidate {
    Vec2 point;
    double distanceSquared = 0.0;
    GeometryRef first;
    GeometryRef second;
    std::optional<Line2> firstExtension;
    std::optional<Line2> secondExtension;
};
}  // namespace geometry

#endif  // OURPAINT_SNAP_QUERY_TYPES_H
