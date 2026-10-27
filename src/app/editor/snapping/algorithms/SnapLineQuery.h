#ifndef OURPAINT_SNAP_LINE_QUERY_H
#define OURPAINT_SNAP_LINE_QUERY_H

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
#include <utility>

#include "geometry/LineAlgorithms.h"
#include "geometry/GeometryTypes.h"
#include "sketch/SketchTypes.h"

// This module owns enumeration of line features. SnapSystem only translates
// these geometric hits into ranked SnapCandidates and chooses the winner.
namespace snap::algorithms {

enum class LineFeature { Body, Start, End, Midpoint, CollinearExtension };

struct LineFeatureCandidate {
    geometry::Vec2 point;
    double distance = 0.0;
    LineFeature feature = LineFeature::Body;
    geometry::GeometryRef source;
    std::optional<geometry::Line2> guide;
};

inline bool usableLine(const geometry::Line2& line) {
    return geometry::detail::isFinite(line) && !geometry::detail::isDegenerate(line);
}

inline std::vector<LineFeatureCandidate> queryLineFeatures(
    const geometry::Vec2& cursor,
    const std::vector<geometry::SketchEntity>& entities,
    bool endpoints, bool midpoint, bool body,
    bool collinearStart, bool collinearContinuation,
    const std::vector<core::sketch::Point2>& anchors,
    double endpointRadius, double midpointRadius, double bodyRadius, double bodyStep, double collinearReach,
    double lateralRadius) {
    using namespace geometry;
    using namespace geometry::detail;
    std::vector<LineFeatureCandidate> out;
    if (!isFinite(cursor)) return out;

    auto append = [&](const Vec2& point, LineFeature feature, GeometryRef source,
                      std::optional<Line2> guide = std::nullopt) {
        const double d = distance(cursor, point);
        out.push_back({point, d, feature, source, std::move(guide)});
    };

    for (const auto& entity : entities) {
        const auto* line = std::get_if<Line2>(&entity.geometry);
        if (!line || !usableLine(*line)) continue;
        const GeometryRef whole{entity.id, SubElement::Whole};
        if (endpoints) {
            if (distance(cursor, line->start) <= endpointRadius)
                append(line->start, LineFeature::Start, {entity.id, SubElement::Start});
            if (distance(cursor, line->end) <= endpointRadius)
                append(line->end, LineFeature::End, {entity.id, SubElement::End});
        }
        if (midpoint) {
            const Vec2 middle{(line->start.x + line->end.x) * 0.5, (line->start.y + line->end.y) * 0.5};
            if (distance(cursor, middle) <= midpointRadius) append(middle, LineFeature::Midpoint, whole);
        }
        if (body) {
            const auto nearest = closestPointOnSegment(cursor, *line);
            if (nearest) {
                Vec2 target = *nearest;
                // Quantize body attachment by a zoom-dependent, human-friendly
                // world-space interval. Endpoints remain exact and are handled
                // by the endpoint query above.
                const Vec2 direction = subtract(line->end, line->start);
                const double len = std::sqrt(lengthSquared(direction));
                if (bodyStep > 1e-12 && len > 1e-12) {
                    const double t = std::clamp(dot(subtract(*nearest, line->start), direction) / (len * len), 0.0, 1.0);
                    const double along = t * len;
                    const double snappedAlong = std::clamp(std::round(along / bodyStep) * bodyStep, 0.0, len);
                    target = Vec2{line->start.x + direction.x * (snappedAlong / len),
                                  line->start.y + direction.y * (snappedAlong / len)};
                }
                const double d = distance(cursor, target);
                const bool nearAnchor = distance(cursor, line->start) <= endpointRadius ||
                    distance(cursor, line->end) <= endpointRadius;
                if (d <= bodyRadius && !nearAnchor) append(target, LineFeature::Body, whole);
            }
        }

        const Vec2 v = subtract(line->end, line->start);
        const double len2 = lengthSquared(v);
        const double len = std::sqrt(len2);
        auto queryExtension = [&](const Vec2& origin, double t, bool continuation) {
            if (t >= 0.0 && t <= 1.0) return;
            const double along = t < 0.0 ? -t * len : (t - 1.0) * len;
            if (!continuation && std::min(distance(cursor, line->start), distance(cursor, line->end)) <= endpointRadius) return;
            if (along > collinearReach) return;
            const Vec2 projection{line->start.x + t * v.x, line->start.y + t * v.y};
            if (distance(cursor, projection) > lateralRadius) return;
            append(projection, LineFeature::CollinearExtension, whole, Line2{origin, projection});
        };
        if (collinearStart) {
            const double t = dot(subtract(cursor, line->start), v) / len2;
            queryExtension(line->start, t, false);
        }
        if (collinearContinuation && !anchors.empty()) {
            const Vec2 anchor{anchors.back().position.x, anchors.back().position.y};
            const double anchorT = dot(subtract(anchor, line->start), v) / len2;
            if (anchorT < 0.0 || anchorT > 1.0) {
                const Vec2 anchorProjection{line->start.x + anchorT * v.x, line->start.y + anchorT * v.y};
                const double along = anchorT < 0.0 ? -anchorT * len : (anchorT - 1.0) * len;
                if (along <= collinearReach && std::min(distance(anchor, line->start), distance(anchor, line->end)) > endpointRadius) {
                    Vec2 unit{v.x / len, v.y / len};
                    if (anchorT < 0.0) unit = multiply(unit, -1.0);
                    const double t = dot(subtract(cursor, anchor), unit);
                    if (t > 0.0) {
                        const Vec2 projection{anchor.x + t * unit.x, anchor.y + t * unit.y};
                        if (distance(cursor, projection) <= lateralRadius)
                            append(projection, LineFeature::CollinearExtension, whole, Line2{anchor, projection});
                    }
                }
            }
        }
    }
    return out;
}

} // namespace snap::algorithms
#endif // OURPAINT_SNAP_LINE_QUERY_H
