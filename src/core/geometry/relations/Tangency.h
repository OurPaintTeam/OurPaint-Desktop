#ifndef OURPAINT_GEOMETRY_RELATIONS_TANGENCY_H
#define OURPAINT_GEOMETRY_RELATIONS_TANGENCY_H

#include <algorithm>
#include <cmath>
#include <span>
#include <type_traits>
#include <variant>
#include <vector>

#include "geometry/intersections/IntersectionDispatcher.h"

namespace geometry::relations {

struct TangencyPoint {
    Vec2 point;
    GeometryRef first;
    GeometryRef second;
};

namespace detail_tangency {
inline Vec2 tangentAt(const Line2& line, const Vec2&) { return {line.end.x - line.start.x, line.end.y - line.start.y}; }
inline Vec2 tangentAt(const Circle2& circle, const Vec2& point) { return {-(point.y - circle.center.y), point.x - circle.center.x}; }
inline Vec2 tangentAt(const Arc2& arc, const Vec2& point) { return {-(point.y - arc.center.y), point.x - arc.center.x}; }

template <class T>
inline bool isCurve(const T&) {
    return !std::is_same_v<T, Line2>;
}

inline bool tangentPair(const Line2& a, const Line2& b, const Vec2& p) {
    (void)a;
    (void)b;
    (void)p;
    return false;  // line-line contact is handled as intersection/parallelism, not tangency.
}

template <class A, class B>
inline bool tangentPair(const A& a, const B& b, const Vec2& p) {
    if constexpr (std::is_same_v<A, Line2> && std::is_same_v<B, Line2>) {
        return false;
    } else {
        const Vec2 ta = tangentAt(a, p);
        const Vec2 tb = tangentAt(b, p);
        const double la = std::hypot(ta.x, ta.y);
        const double lb = std::hypot(tb.x, tb.y);
        if (la <= EPSILON || lb <= EPSILON) {
            return false;
        }
        const double normalizedCross = std::abs(ta.x * tb.y - ta.y * tb.x) / (la * lb);
        return normalizedCross <= 1e-7;
    }
}

template <class A, class B>
inline void collectPair(const A& a, const B& b, const SketchEntity& ea, const SketchEntity& eb, std::vector<TangencyPoint>& out) {
    if constexpr ((std::is_same_v<A, Line2> || std::is_same_v<A, Circle2> || std::is_same_v<A, Arc2>) &&
                  (std::is_same_v<B, Line2> || std::is_same_v<B, Circle2> || std::is_same_v<B, Arc2>)) {
        if constexpr (std::is_same_v<A, Line2> && std::is_same_v<B, Line2>) {
            return;
        }
        const auto points = intersections::intersect(a, b);
        for (const auto& p2 : points) {
            const Vec2 p{p2.position};
            if (!tangentPair(a, b, p)) {
                continue;
            }
            const auto duplicate = std::find_if(out.begin(), out.end(), [&](const TangencyPoint& old) {
                return old.first.entity == ea.id && old.second.entity == eb.id && detail::distanceSquared(old.point, p) <= EPSILON * EPSILON;
            });
            if (duplicate == out.end()) {
                out.push_back({p, GeometryRef{ea.id, SubElement::Whole}, GeometryRef{eb.id, SubElement::Whole}});
            }
        }
    }
}
}  // namespace detail_tangency

// Finds tangent contact points between existing sketch entities. Lines are finite
// segments by default; set includeLineExtensions=true to test their supporting lines.
inline std::vector<TangencyPoint> findTangencies(std::span<const SketchEntity> entities, bool includeLineExtensions = false) {
    std::vector<TangencyPoint> result;
    for (std::size_t i = 0; i < entities.size(); ++i) {
        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            const auto& a = entities[i];
            const auto& b = entities[j];
            if (includeLineExtensions) {
                const auto* la = std::get_if<Line2>(&a.geometry);
                const auto* lb = std::get_if<Line2>(&b.geometry);
                if (la && (std::holds_alternative<Circle2>(b.geometry) || std::holds_alternative<Arc2>(b.geometry))) {
                    std::vector<Point2> pts;
                    if (const auto* c = std::get_if<Circle2>(&b.geometry)) {
                        pts = intersections::lineCircle(*la, *c, false);
                    } else if (const auto* ar = std::get_if<Arc2>(&b.geometry)) {
                        pts = intersections::lineArc(*la, *ar, false);
                    }
                    for (const auto& pt : pts) {
                        const Vec2 p{pt.position};
                        const auto* c = std::get_if<Circle2>(&b.geometry);
                        const Vec2 center = c ? c->center : std::get<Arc2>(b.geometry).center;
                        const Vec2 radial{p.x - center.x, p.y - center.y};
                        const Vec2 direction{la->end.x - la->start.x, la->end.y - la->start.y};
                        const double den = std::hypot(radial.x, radial.y) * std::hypot(direction.x, direction.y);
                        if (den > EPSILON && std::abs(radial.x * direction.x + radial.y * direction.y) / den <= 1e-7) {
                            result.push_back({p, GeometryRef{a.id, SubElement::Whole}, GeometryRef{b.id, SubElement::Whole}});
                        }
                    }
                    continue;
                }
                if (lb && (std::holds_alternative<Circle2>(a.geometry) || std::holds_alternative<Arc2>(a.geometry))) {
                    // The reverse ordering is handled by the symmetric branch below.
                    std::vector<Point2> pts;
                    if (const auto* c = std::get_if<Circle2>(&a.geometry)) {
                        pts = intersections::lineCircle(*lb, *c, false);
                    } else if (const auto* ar = std::get_if<Arc2>(&a.geometry)) {
                        pts = intersections::lineArc(*lb, *ar, false);
                    }
                    for (const auto& pt : pts) {
                        const Vec2 p{pt.position};
                        const auto* c = std::get_if<Circle2>(&a.geometry);
                        const Vec2 center = c ? c->center : std::get<Arc2>(a.geometry).center;
                        const Vec2 radial{p.x - center.x, p.y - center.y};
                        const Vec2 direction{lb->end.x - lb->start.x, lb->end.y - lb->start.y};
                        const double den = std::hypot(radial.x, radial.y) * std::hypot(direction.x, direction.y);
                        if (den > EPSILON && std::abs(radial.x * direction.x + radial.y * direction.y) / den <= 1e-7) {
                            result.push_back({p, GeometryRef{a.id, SubElement::Whole}, GeometryRef{b.id, SubElement::Whole}});
                        }
                    }
                    continue;
                }
            }
            std::visit([&](const auto& ga, const auto& gb) { detail_tangency::collectPair(ga, gb, a, b, result); }, a.geometry, b.geometry);
        }
    }
    return result;
}

}  // namespace geometry::relations

#endif  // OURPAINT_GEOMETRY_RELATIONS_TANGENCY_H
