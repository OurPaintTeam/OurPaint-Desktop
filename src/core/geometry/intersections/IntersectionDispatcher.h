#ifndef OURPAINT_INTERSECTION_DISPATCHER_H
#define OURPAINT_INTERSECTION_DISPATCHER_H

#include <type_traits>
#include <variant>

#include "geometry/intersections/ArcIntersection.h"
#include "geometry/intersections/CircleCircleIntersection.h"
#include "geometry/intersections/LineCircleIntersection.h"
#include "geometry/intersections/LineLineIntersection.h"

namespace geometry::intersections {
inline IntersectionPoints intersect(const Line2& a, const Line2& b, IntersectionOptions options = {}) { return lineLine(a, b, options.lineAsSegment); }
inline IntersectionPoints intersect(const Line2& a, const Circle2& b, IntersectionOptions options = {}) { return lineCircle(a, b, options.lineAsSegment); }
inline IntersectionPoints intersect(const Circle2& a, const Line2& b, IntersectionOptions options = {}) { return lineCircle(b, a, options.lineAsSegment); }
inline IntersectionPoints intersect(const Circle2& a, const Circle2& b, IntersectionOptions = {}) { return circleCircle(a, b); }
inline IntersectionPoints intersect(const Line2& a, const Arc2& b, IntersectionOptions options = {}) { return lineArc(a, b, options.lineAsSegment); }
inline IntersectionPoints intersect(const Arc2& a, const Line2& b, IntersectionOptions options = {}) { return lineArc(b, a, options.lineAsSegment); }
inline IntersectionPoints intersect(const Circle2& a, const Arc2& b, IntersectionOptions = {}) { return circleArc(a, b); }
inline IntersectionPoints intersect(const Arc2& a, const Circle2& b, IntersectionOptions = {}) { return circleArc(b, a); }
inline IntersectionPoints intersect(const Arc2& a, const Arc2& b, IntersectionOptions = {}) { return arcArc(a, b); }

// Dispatcher for the geometry variants used by SketchEntity. Unsupported pairs
// (for example point-vs-line) return an empty result.
inline IntersectionPoints intersectGeometries(const SketchEntity& a, const SketchEntity& b, IntersectionOptions options = {}) {
    return std::visit(
        [&](const auto& ga, const auto& gb) -> IntersectionPoints {
            using A = std::decay_t<decltype(ga)>;
            using B = std::decay_t<decltype(gb)>;
            if constexpr (std::is_same_v<A, Line2> || std::is_same_v<A, Circle2> || std::is_same_v<A, Arc2>) {
                if constexpr (std::is_same_v<B, Line2> || std::is_same_v<B, Circle2> || std::is_same_v<B, Arc2>) {
                    return intersect(ga, gb, options);
                } else {
                    return {};
                }
            } else {
                return {};
            }
        },
        a.geometry, b.geometry);
}
}  // namespace geometry::intersections

#endif  // OURPAINT_INTERSECTION_DISPATCHER_H
