#ifndef OURPAINT_INTERSECTION_TYPES_H
#define OURPAINT_INTERSECTION_TYPES_H

#include <vector>

#include "geometry/GeometryTypes.h"

namespace geometry {
// A geometric intersection can contain zero, one (tangency), or two points.
// Coincident objects have infinitely many common points and return no finite list.
using IntersectionPoints = std::vector<Point2>;

struct IntersectionOptions {
    // true: Line2 is treated as a finite segment; false: as an infinite line.
    bool lineAsSegment = true;
};
}  // namespace geometry

#endif  // OURPAINT_INTERSECTION_TYPES_H
