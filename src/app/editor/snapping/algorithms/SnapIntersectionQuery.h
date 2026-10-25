#ifndef OURPAINT_SNAP_INTERSECTION_QUERY_H
#define OURPAINT_SNAP_INTERSECTION_QUERY_H

#include <algorithm>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include "geometry/intersections/IntersectionDispatcher.h"
#include "snapping/algorithms/SnapQueryTypes.h"

namespace geometry::detail {
inline bool isBetter(const IntersectionCandidate& candidate, const IntersectionCandidate& current) {
    return candidate.distanceSquared < current.distanceSquared;
}
inline void consider(std::optional<IntersectionCandidate>& best, const Vec2& cursor, const Vec2& point, GeometryRef first, GeometryRef second) {
    if (!isFinite(cursor) || !isFinite(point)) {
        return;
    }
    IntersectionCandidate candidate;
    candidate.point = point;
    candidate.distanceSquared = distanceSquared(cursor, point);
    candidate.first = first;
    candidate.second = second;
    if (!best || isBetter(candidate, *best)) {
        best = candidate;
    }
}
}  // namespace geometry::detail

namespace geometry {
inline std::optional<IntersectionCandidate> findNearestIntersection(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<IntersectionCandidate> best;
    for (std::size_t i = 0; i < entities.size(); ++i) {
        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            const auto points = intersections::intersectGeometries(entities[i], entities[j]);
            const GeometryRef first{entities[i].id, SubElement::Whole};
            const GeometryRef second{entities[j].id, SubElement::Whole};
            for (const auto& p : points) {
                detail::consider(best, cursor, Vec2{p.position}, first, second);
            }
        }
    }
    return best;
}

// Returns every intersection that requires extending at least one Line2.
// Results are sorted by distance to the cursor. For line-circle and line-arc
// pairs, both boundary hits are kept when both lie beyond the segment.
inline std::vector<ExtendedIntersectionCandidate> findExtendedLineIntersections(const Vec2& cursor, std::span<const SketchEntity> entities) {
    using namespace intersections;
    std::vector<ExtendedIntersectionCandidate> result;
    if (!detail::isFinite(cursor)) {
        return result;
    }

    const auto parameterOnLine = [](const Line2& line, const Vec2& point) {
        const Vec2 d = detail::subtract(line.end, line.start);
        const double denominator = detail::lengthSquared(d);
        if (denominator <= EPSILON * EPSILON) {
            return 0.0;
        }
        return detail::dot(detail::subtract(point, line.start), d) / denominator;
    };
    const auto makeExtension = [](const Line2& line, double t, const Vec2& point) -> std::optional<Line2> {
        if (detail::onSegment(t)) {
            return std::nullopt;
        }
        // Extend from the endpoint on the side where the hit lies.
        return t < 0.0 ? std::optional<Line2>{Line2{line.start, point}} : std::optional<Line2>{Line2{line.end, point}};
    };
    const auto append = [&](const Vec2& point, const SketchEntity& firstEntity, const SketchEntity& secondEntity, std::optional<Line2> firstExtension,
                            std::optional<Line2> secondExtension) {
        if (!detail::isFinite(point) || (!firstExtension && !secondExtension)) {
            return;
        }
        const double d2 = detail::distanceSquared(cursor, point);
        // Guard against duplicates caused by tangency or numerically repeated hits.
        for (const auto& old : result) {
            if (detail::distanceSquared(old.point, point) <= EPSILON * EPSILON && old.first.entity == firstEntity.id && old.second.entity == secondEntity.id) {
                return;
            }
        }
        ExtendedIntersectionCandidate candidate;
        candidate.point = point;
        candidate.distanceSquared = d2;
        candidate.first = GeometryRef{firstEntity.id, SubElement::Whole};
        candidate.second = GeometryRef{secondEntity.id, SubElement::Whole};
        candidate.firstExtension = std::move(firstExtension);
        candidate.secondExtension = std::move(secondExtension);
        result.push_back(std::move(candidate));
    };

    for (std::size_t i = 0; i < entities.size(); ++i) {
        const auto* lineA = std::get_if<Line2>(&entities[i].geometry);
        if (!lineA || detail::isDegenerate(*lineA)) {
            continue;
        }

        for (std::size_t j = 0; j < entities.size(); ++j) {
            if (i == j) {
                continue;
            }
            const auto& other = entities[j];

            if (const auto* lineB = std::get_if<Line2>(&other.geometry)) {
                // Handle a line-line pair once. Either or both segments may need extension.
                if (j < i || detail::isDegenerate(*lineB)) {
                    continue;
                }
                const auto hit = intersectLines(*lineA, *lineB, false);
                if (!hit) {
                    continue;
                }
                auto extA = makeExtension(*lineA, hit->t, hit->point);
                auto extB = makeExtension(*lineB, hit->u, hit->point);
                append(hit->point, entities[i], other, std::move(extA), std::move(extB));
                continue;
            }

            IntersectionPoints points;
            if (const auto* circle = std::get_if<Circle2>(&other.geometry)) {
                points = lineCircle(*lineA, *circle, false);
            } else if (const auto* arc = std::get_if<Arc2>(&other.geometry)) {
                points = lineArc(*lineA, *arc, false);
            } else {
                continue;
            }

            for (const auto& p : points) {
                const Vec2 point{p.position};
                const double t = parameterOnLine(*lineA, point);
                auto extA = makeExtension(*lineA, t, point);
                append(point, entities[i], other, std::move(extA), std::nullopt);
            }
        }
    }

    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.distanceSquared < b.distanceSquared; });
    return result;
}

inline std::optional<IntersectionCandidate> findNearestExtendedLineIntersection(const Vec2& cursor, std::span<const SketchEntity> entities) {
    if (!detail::isFinite(cursor)) {
        return std::nullopt;
    }
    std::optional<IntersectionCandidate> best;
    for (std::size_t i = 0; i < entities.size(); ++i) {
        const auto* a = std::get_if<Line2>(&entities[i].geometry);
        if (!a) {
            continue;
        }
        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            const auto* b = std::get_if<Line2>(&entities[j].geometry);
            if (!b) {
                continue;
            }
            const auto hit = intersections::intersectLines(*a, *b, false);
            if (!hit || (detail::onSegment(hit->t) && detail::onSegment(hit->u))) {
                continue;
            }
            detail::consider(best, cursor, hit->point, GeometryRef{entities[i].id, SubElement::Whole}, GeometryRef{entities[j].id, SubElement::Whole});
        }
    }
    return best;
}
}  // namespace geometry

#endif  // OURPAINT_SNAP_INTERSECTION_QUERY_H
