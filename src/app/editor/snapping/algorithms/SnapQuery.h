#ifndef OURPAINT_SNAP_QUERY_H
#define OURPAINT_SNAP_QUERY_H

#include <optional>
#include <span>
#include <variant>

#include "geometry/ArcAlgorithms.h"
#include "geometry/CircleAlgorithms.h"
#include "geometry/LineAlgorithms.h"
#include "snapping/algorithms/SnapQueryTypes.h"

namespace geometry::detail {
inline bool isBetter(const PointCandidate& candidate, const PointCandidate& current) { return candidate.distanceSquared < current.distanceSquared; }
inline void consider(std::optional<PointCandidate>& best, const Vec2& cursor, const Vec2& point, GeometryRef source) {
    if (!isFinite(cursor) || !isFinite(point)) {
        return;
    }
    PointCandidate candidate{point, distanceSquared(cursor, point), source};
    if (!best || isBetter(candidate, *best)) {
        best = candidate;
    }
}
inline void consider(std::optional<PointCandidate>& best, PointCandidate candidate) {
    if (!isFinite(candidate.point) || !std::isfinite(candidate.distanceSquared) || candidate.distanceSquared < 0.0) {
        return;
    }
    if (!best || isBetter(candidate, *best)) {
        best = candidate;
    }
}
}  // namespace geometry::detail

namespace geometry {
inline std::optional<PointCandidate> findNearestStandalonePoint(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* point = std::get_if<Point2>(&entity.geometry);
        if (point) {
            detail::consider(best, cursor, point->position, GeometryRef{entity.id, SubElement::Whole});
        }
    }
    return best;
}

inline std::optional<PointCandidate> findNearestLine(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* line = std::get_if<Line2>(&entity.geometry);
        if (!line) {
            continue;
        }
        const auto closest = detail::closestPointOnSegment(cursor, *line);
        if (!closest) {
            continue;
        }
        GeometryRef source{entity.id, SubElement::Whole};
        if (detail::distanceSquared(*closest, line->start) <= EPSILON * EPSILON) {
            source.sub = SubElement::Start;
        } else if (detail::distanceSquared(*closest, line->end) <= EPSILON * EPSILON) {
            source.sub = SubElement::End;
        }
        detail::consider(best, PointCandidate{*closest, detail::distanceSquared(cursor, *closest), source});
    }
    return best;
}

inline std::optional<PointCandidate> findNearestCircle(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* circle = std::get_if<Circle2>(&entity.geometry);
        if (!circle || !detail::isFinite(*circle) || circle->radius <= 0.0) {
            continue;
        }
        const auto boundary = detail::closestPointOnCircle(cursor, *circle);
        if (!boundary) {
            continue;
        }
        const double boundaryDistance = detail::distanceSquared(cursor, *boundary);
        const double centerDistance = detail::distanceSquared(cursor, circle->center);
        const bool useCenter = centerDistance < boundaryDistance;
        const Vec2 point = useCenter ? circle->center : *boundary;
        const GeometryRef source{entity.id, useCenter ? SubElement::Center : SubElement::Whole};
        detail::consider(best, PointCandidate{point, detail::distanceSquared(cursor, point), source});
    }
    return best;
}

inline std::optional<PointCandidate> findNearestCircleBody(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* circle = std::get_if<Circle2>(&entity.geometry);
        if (!circle || !detail::isFinite(*circle) || circle->radius <= 0.0) {
            continue;
        }
        const auto point = detail::closestPointOnCircle(cursor, *circle);
        if (point) {
            detail::consider(best, cursor, *point, GeometryRef{entity.id, SubElement::Whole});
        }
    }
    return best;
}

inline std::optional<PointCandidate> findNearestCircleCenter(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* circle = std::get_if<Circle2>(&entity.geometry);
        if (circle && detail::isFinite(*circle) && circle->radius >= 0.0) {
            detail::consider(best, cursor, circle->center, GeometryRef{entity.id, SubElement::Center});
        }
    }
    return best;
}

inline bool isLineCenter(const Vec2& cursor, const Line2& line, double tolerance) {
    if (!detail::isFinite(cursor) || !detail::isFinite(line) || !std::isfinite(tolerance) || tolerance < 0.0 || detail::isDegenerate(line)) {
        return false;
    }
    const Vec2 center{(line.start.x + line.end.x) * 0.5, (line.start.y + line.end.y) * 0.5};
    return detail::distanceSquared(cursor, center) <= tolerance * tolerance;
}

inline std::optional<PointCandidate> findNearestArcBody(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* arc = std::get_if<Arc2>(&entity.geometry);
        if (!arc) {
            continue;
        }
        const auto nearest = detail::closestPointOnArc(cursor, *arc);
        if (nearest) {
            detail::consider(best, PointCandidate{*nearest, detail::distanceSquared(cursor, *nearest), GeometryRef{entity.id, SubElement::Whole}});
        }
    }
    return best;
}

inline std::optional<PointCandidate> findNearestArcEndpoint(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* arc = std::get_if<Arc2>(&entity.geometry);
        if (!arc || !detail::isFinite(*arc)) {
            continue;
        }
        detail::consider(best, cursor, arc->start, GeometryRef{entity.id, SubElement::Start});
        detail::consider(best, cursor, arc->end, GeometryRef{entity.id, SubElement::End});
    }
    return best;
}

inline std::optional<PointCandidate> findNearestArcCenter(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<PointCandidate> best;
    for (const auto& entity : entities) {
        const auto* arc = std::get_if<Arc2>(&entity.geometry);
        if (arc && detail::isFinite(*arc)) {
            detail::consider(best, cursor, arc->center, GeometryRef{entity.id, SubElement::Center});
        }
    }
    return best;
}
}  // namespace geometry

#endif  // OURPAINT_SNAP_QUERY_H
