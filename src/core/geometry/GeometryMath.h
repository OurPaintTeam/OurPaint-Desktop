#ifndef OURPAINT_GEOMETRY_MATH_H
#define OURPAINT_GEOMETRY_MATH_H

#include <algorithm>
#include <cmath>

#include "geometry/GeometryTypes.h"

namespace geometry {
inline constexpr double EPSILON = 1e-9;

namespace detail {
inline Vec2 add(const Vec2& a, const Vec2& b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 subtract(const Vec2& a, const Vec2& b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 multiply(const Vec2& v, double scalar) { return {v.x * scalar, v.y * scalar}; }
inline Vec2 midpoint(const Line2& line) { return {(line.start.x + line.end.x) * 0.5, (line.start.y + line.end.y) * 0.5}; }
inline double dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
inline double cross(const Vec2& a, const Vec2& b) { return a.x * b.y - a.y * b.x; }
inline double lengthSquared(const Vec2& v) { return dot(v, v); }
inline double distanceSquared(const Vec2& a, const Vec2& b) { return lengthSquared(subtract(a, b)); }
inline bool isFinite(const Vec2& p) { return std::isfinite(p.x) && std::isfinite(p.y); }
inline bool isFinite(const Line2& line) { return isFinite(line.start) && isFinite(line.end); }
inline bool isFinite(const Circle2& circle) { return isFinite(circle.center) && std::isfinite(circle.radius) && circle.radius >= 0.0; }
inline bool isFinite(const Arc2& arc) { return isFinite(arc.center) && isFinite(arc.start) && isFinite(arc.end); }
inline bool isDegenerate(const Vec2& start, const Vec2& end) { return distanceSquared(start, end) <= EPSILON * EPSILON; }
inline bool isDegenerate(const Line2& line) { return isDegenerate(line.start, line.end); }
inline bool onSegment(double t) { return t >= -EPSILON && t <= 1.0 + EPSILON; }
}  // namespace detail

inline double distance(const Vec2& a, const Vec2& b) { return std::sqrt(detail::distanceSquared(a, b)); }
}  // namespace geometry

#endif  // OURPAINT_GEOMETRY_MATH_H
