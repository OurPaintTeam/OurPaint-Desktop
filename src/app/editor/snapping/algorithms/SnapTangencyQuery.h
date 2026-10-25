#ifndef OURPAINT_SNAP_TANGENCY_QUERY_H
#define OURPAINT_SNAP_TANGENCY_QUERY_H

#include <algorithm>
#include <cmath>
#include <optional>
#include <span>
#include <variant>
#include <vector>

#include "geometry/LineAlgorithms.h"
#include "geometry/relations/PointCurveTangency.h"

namespace snap::algorithms {

struct TangencyCandidate {
    geometry::Vec2 point;
    geometry::GeometryRef object;
    double distanceSquared = 0.0;
};

inline std::optional<TangencyCandidate> findLinePlacementTangency(
    const geometry::Vec2& anchor,
    const geometry::Vec2& cursor,
    std::span<const geometry::SketchEntity> entities,
    std::span<const geometry::GeometryRef> forbiddenObjects,
    double maxDistance) {
    if (!geometry::detail::isFinite(anchor) || !geometry::detail::isFinite(cursor) ||
        !std::isfinite(maxDistance) || maxDistance < 0.0) {
        return std::nullopt;
    }

    std::optional<TangencyCandidate> best;
    const double maxDistanceSquared = maxDistance * maxDistance;
    for (const auto& entity : entities) {
        const bool forbidden = std::any_of(forbiddenObjects.begin(), forbiddenObjects.end(), [&](const auto& ref) {
            return ref.entity == entity.id;
        });
        if (forbidden) {
            continue;
        }

        std::vector<geometry::relations::PointCurveTangentCandidate> points;
        if (const auto* circle = std::get_if<geometry::Circle2>(&entity.geometry)) {
            const auto onBody = geometry::detail::closestPointOnCircle(cursor, *circle);
            if (onBody && geometry::detail::distanceSquared(cursor, *onBody) <= maxDistanceSquared) {
                continue;
            }
            points = geometry::relations::pointTangenciesToCircle(anchor, cursor, circle->center, circle->radius);
        } else {
            // Line placement tangency is intentionally supported for circles only.
            // Arc-body proximity is handled as an ordinary point snap, not tangency.
            continue;
        }

        for (const auto& point : points) {
            const geometry::Vec2 ray = geometry::detail::subtract(point.point, anchor);
            const double rayLength = std::hypot(ray.x, ray.y);
            if (rayLength <= geometry::EPSILON) {
                continue;
            }
            const geometry::Vec2 direction{ray.x / rayLength, ray.y / rayLength};
            const geometry::Vec2 cursorOffset = geometry::detail::subtract(cursor, anchor);
            const double along = geometry::detail::dot(cursorOffset, direction);
            if (along < 0.0) {
                continue;
            }
            const geometry::Vec2 projectedEndpoint{anchor.x + along * direction.x, anchor.y + along * direction.y};
            // The tangent contact must be reached by the finite segment being
            // drawn. Do not snap to a tangent whose contact lies beyond the cursor
            // endpoint (an infinite-line/continuation false positive).
            const double contactAlong = geometry::detail::dot(
                geometry::detail::subtract(point.point, anchor), direction);
            if (contactAlong < -geometry::EPSILON || contactAlong > along + geometry::EPSILON) {
                continue;
            }
            const double distanceSquared = geometry::detail::distanceSquared(cursor, projectedEndpoint);
            if (!std::isfinite(distanceSquared) || distanceSquared > maxDistanceSquared) {
                continue;
            }
            TangencyCandidate candidate{projectedEndpoint,
                geometry::GeometryRef{entity.id, geometry::SubElement::Whole}, distanceSquared};
            if (!best || candidate.distanceSquared < best->distanceSquared) {
                best = candidate;
            }
        }
    }
    return best;
}

// Finds circle/arc placement tangencies against existing line or arc bodies.
// The first committed anchor is the center; the returned point is the contact point.
inline std::optional<TangencyCandidate> findCurvePlacementTangency(
    const geometry::Vec2& center,
    const geometry::Vec2& cursor,
    std::span<const geometry::SketchEntity> entities,
    std::span<const geometry::GeometryRef> forbiddenObjects,
    double maxDistance) {
    using namespace geometry;
    if (!detail::isFinite(center) || !detail::isFinite(cursor) || !std::isfinite(maxDistance) || maxDistance < 0.0) {
        return std::nullopt;
    }

    std::optional<TangencyCandidate> best;
    const double maxDistanceSquared = maxDistance * maxDistance;
    for (const auto& entity : entities) {
        if (std::any_of(forbiddenObjects.begin(), forbiddenObjects.end(), [&](const auto& ref) { return ref.entity == entity.id; })) {
            continue;
        }

        std::optional<Vec2> contact;
        if (const auto* line = std::get_if<Line2>(&entity.geometry)) {
            const Vec2 direction = detail::subtract(line->end, line->start);
            const double lengthSquared = detail::lengthSquared(direction);
            if (lengthSquared <= EPSILON * EPSILON) {
                continue;
            }
            const double t = detail::dot(detail::subtract(center, line->start), direction) / lengthSquared;
            if (t <= EPSILON || t >= 1.0 - EPSILON) {
                continue;
            }
            contact = detail::closestPointOnSegment(center, *line);
        } else if (const auto* arc = std::get_if<Arc2>(&entity.geometry)) {
            const Vec2 radial = detail::subtract(center, arc->center);
            const double radialLength = std::hypot(radial.x, radial.y);
            if (radialLength > EPSILON) {
                const double arcRadius = std::sqrt(detail::distanceSquared(arc->center, arc->start));
                const Vec2 point{arc->center.x + radial.x * arcRadius / radialLength,
                                 arc->center.y + radial.y * arcRadius / radialLength};
                if (relations::pointOnArcSweep(point, *arc) &&
                    detail::distanceSquared(point, arc->start) > maxDistanceSquared &&
                    detail::distanceSquared(point, arc->end) > maxDistanceSquared) {
                    contact = point;
                }
            }
        } else if (const auto* circle = std::get_if<Circle2>(&entity.geometry)) {
            // Pairwise circle-body tangency: the contact lies on the center line.
            // Consider the near-side (external) and far-side (internal) contacts;
            // only the candidate close to the cursor will survive the hit test.
            const Vec2 radial = detail::subtract(center, circle->center);
            const double radialLength = std::hypot(radial.x, radial.y);
            if (radialLength > EPSILON) {
                const Vec2 unit{radial.x / radialLength, radial.y / radialLength};
                const Vec2 nearSide{circle->center.x + unit.x * circle->radius,
                                    circle->center.y + unit.y * circle->radius};
                const Vec2 farSide{circle->center.x - unit.x * circle->radius,
                                   circle->center.y - unit.y * circle->radius};
                const Vec2* chosen = detail::distanceSquared(cursor, nearSide) <= detail::distanceSquared(cursor, farSide)
                    ? &nearSide : &farSide;
                contact = *chosen;
            }
        } else {
            continue;
        }
        if (!contact || detail::distanceSquared(center, *contact) <= EPSILON * EPSILON) {
            continue;
        }
        const double distanceSquared = detail::distanceSquared(cursor, *contact);
        if (!std::isfinite(distanceSquared) || distanceSquared > maxDistanceSquared) {
            continue;
        }
        TangencyCandidate candidate{*contact, GeometryRef{entity.id, SubElement::Whole}, distanceSquared};
        if (!best || candidate.distanceSquared < best->distanceSquared) {
            best = candidate;
        }
    }
    return best;
}

}  // namespace snap::algorithms

#endif  // OURPAINT_SNAP_TANGENCY_QUERY_H
