#include "SnapSystem.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

#include "Algorithms.h"
#include "snapping/algorithms/SnapTangencyQuery.h"
#include "Camera2D.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"

namespace snap {
namespace {

using namespace core::sketch;

constexpr double kDefaultSnapRadiusPixels = 5.0;
constexpr double kCollinearSnapRadiusPixels = 15.0; // longitudinal endpoint influence, in screen pixels
constexpr double kTangentSnapRadiusPixels = 3.0;

using geometry::Circle2;
using geometry::GeometryRef;
using geometry::Line2;
using geometry::Point2;
using geometry::SubElement;
using geometry::Vec2;

using geometry::distance;
using geometry::detail::add;
using geometry::detail::closestPointOnCircle;
using geometry::detail::closestPointOnSegment;
using geometry::detail::cross;
using geometry::detail::distanceSquared;
using geometry::detail::dot;
using geometry::detail::isFinite;
using geometry::detail::lengthSquared;
using geometry::detail::midpoint;
using geometry::detail::multiply;
using geometry::detail::subtract;

// Snap rejects degenerate line segments even though the geometry layer keeps
// them representable for callers that need point-like segment behavior.
bool isUsableLine(const Line2& line) { return isFinite(line) && !geometry::detail::isDegenerate(line); }

SnapCandidate makeCandidate(const Vec2& point, SnapKind kind, double candidateDistance, int priority, std::vector<GeometryRef> objects = {},
                            std::vector<Line2> guideLines = {}) {
    SnapResult result;
    result.snapped = true;
    result.point = core::sketch::Point2{point};
    result.type = kind;
    result.objects = std::move(objects);
    result.guideLines = std::move(guideLines);

    return SnapCandidate{std::move(result), candidateDistance * candidateDistance, priority};
}

}  // namespace

bool SnapSystem::isBetterCandidate(const SnapCandidate& candidate, const SnapCandidate& current, double priorityRadius) {
    constexpr double epsilon = 1e-9;

    // Priority is meaningful inside the screen-space priority band. This lets a
    // real/virtual intersection beat a nearby line body or endpoint, without
    // allowing a much farther high-priority candidate to steal the snap.
    // priorityRadius is applied in parser() in world units (pixels / zoom).
    // SnapCandidate intentionally stores no rule, so the standard band is
    // enforced by candidate distance comparisons after parser() has filtered
    // candidates to the pixel hit radius.
    const double candidateDistance = std::sqrt(std::max(0.0, candidate.distanceSquared));
    const double currentDistance = std::sqrt(std::max(0.0, current.distanceSquared));
    if (std::abs(candidateDistance - currentDistance) <= priorityRadius + epsilon) {
        return candidate.priority > current.priority;
    }
    return candidateDistance < currentDistance;
}

SnapSystem::SnapSystem(const core::sketch::Sketch& sketch, const Camera2D& camera) : sketch_(sketch), camera_(camera) {}

SnapResult SnapSystem::getSnapCandidate(const SnapRequest& request) const { return parser(request); }

SnapRule SnapSystem::ruleFor(ActionType action, DrawState state) {
    SnapRule rule;
    rule.mask = snapMaskFor(action, state);
    // Values are screen pixels here; parser() converts them to world units using zoom.
    rule.maxRadius = kDefaultSnapRadiusPixels;
    rule.priorityRadius = 1.5;
    rule.collinearRadius = kCollinearSnapRadiusPixels;
    rule.tangentRadius = kTangentSnapRadiusPixels;
    return rule;
}

SnapResult SnapSystem::parser(const SnapRequest& request) const {
    SnapRule rule = ruleFor(request.action, request.drawState);

    // All distances produced by geometry queries are in world units. Convert the
    // pixel hit tolerance to world units so the snap target stays the same size
    // on screen at every zoom level.
    // A missing/invalid zoom must not disable every snap (especially endpoints).
    // SnapRequest defaults to 1.0; use the same safe fallback if a caller passes
    // zero, a negative value, NaN, or infinity. The view integration should still
    // pass its actual world-to-screen scale for exact pixel tolerance.
    const double currentZoom = camera_.zoom();
    const double zoom = (std::isfinite(currentZoom) && currentZoom > 1e-9) ? currentZoom : 1.0;
    rule.maxRadius /= zoom;
    rule.priorityRadius /= zoom;
    rule.collinearRadius /= zoom;
    rule.tangentRadius /= zoom;

    if (rule.mask == SnapMask::None || !isFinite(request.cursor) || !std::isfinite(rule.maxRadius) || rule.maxRadius <= 0.0) {
        return {};
    }

    std::optional<SnapCandidate> best;

    const auto consider = [&](std::optional<SnapCandidate> candidate) {
        if (!candidate) {
            return;
        }

        // Final, central hit-test: every snap kind must be within the same
        // on-screen tolerance. Candidate distances are stored in world units,
        // and rule.maxRadius was converted from 5 px using the current zoom.
        // Keep this guard even when individual finders also filter candidates,
        // so no snap type can accidentally bypass the distance limit.
        if (!std::isfinite(candidate->distanceSquared) || candidate->distanceSquared < 0.0 || candidate->distanceSquared > rule.maxRadius * rule.maxRadius) {
            return;
        }
        const double candidateDistance = std::sqrt(candidate->distanceSquared);
        if (candidate->result.type == SnapKind::Collinear && candidateDistance > rule.collinearRadius) return;
        if (candidate->result.type == SnapKind::Tangent && candidateDistance > rule.tangentRadius) return;

        if (!best || isBetterCandidate(*candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    };

    if (hasSnap(rule.mask, SnapMask::Coincident)) {
        consider(findBestStandalonePoint(request, rule));
    }

    // Tangency is queried before body snaps so a genuine body contact can be
    // classified as Tangent; endpoint and center coincidence still wins by priority.
    if (hasSnap(rule.mask, SnapMask::Tangent)) {
        consider(findBestTangentCandidate(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::Coincident) || hasSnap(rule.mask, SnapMask::Midpoint) || hasSnap(rule.mask, SnapMask::Tangent)) {
        consider(findBestLineCandidate(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::Tangent) || hasSnap(rule.mask, SnapMask::Coincident)) {
        consider(findBestCircleCandidate(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::Intersection) || hasSnap(rule.mask, SnapMask::ExtendedIntersection) || hasSnap(rule.mask, SnapMask::VirtualIntersection)) {
        consider(findBestIntersection(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::Tangent) || hasSnap(rule.mask, SnapMask::Coincident) || hasSnap(rule.mask, SnapMask::Midpoint)) {
        consider(findBestArcCandidate(request, rule));
    }

    if (hasSnap(rule.mask, Direction)) {
        consider(findBestDirectionalCandidate(request, rule));
    }

    return best ? best->result : SnapResult{};
}

bool SnapSystem::isAllowed(SnapKind kind, double candidateDistance, const SnapRule& rule) {
    if (!std::isfinite(candidateDistance) || candidateDistance < 0.0 || !std::isfinite(rule.maxRadius) || candidateDistance > rule.maxRadius) {
        return false;
    }
    const SnapMask required = maskForSnapKind(kind);
    return required != SnapMask::None && hasSnap(rule.mask, required);
}

std::optional<SnapCandidate> SnapSystem::findBestDirectionalCandidate(const SnapRequest& request, const SnapRule& rule) const {
    if (request.anchors.empty() || request.action != ActionType::Line || request.drawState != DrawState::End) {
        return std::nullopt;
    }
    const Vec2 base = request.anchors.back().position;
    if (!isFinite(base) || !isFinite(request.cursor)) {
        return std::nullopt;
    }
    auto entitiesResult = sketch_.entities();
    if (!entitiesResult) {
        return std::nullopt;
    }
    const auto& entities = entitiesResult.value();
    // Parallel/perpendicular relations are only meaningful for a line that shares
    // the committed anchor (by coordinates). Axis H/V directions remain global.
    // Filter line-based direction candidates to reference segments passing through
    // the anchor, using the current screen-space snap tolerance converted to world units.
    auto candidates = geometry::findDirectionSnaps(base, request.cursor, entities, 15.0);
    candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [&](const auto& candidate) {
        if (!candidate.reference) return false; // horizontal/vertical axes
        for (const auto& entity : entities) {
            if (entity.id != candidate.reference->entity) continue;
            const auto* line = std::get_if<Line2>(&entity.geometry);
            if (!line || !isUsableLine(*line)) return true;
            const Vec2 nearest = closestPointOnSegment(base, *line).value();
            return distanceSquared(base, nearest) > rule.maxRadius * rule.maxRadius;
        }
        return true;
    }), candidates.end());
    std::optional<SnapCandidate> best;
    for (const auto& direction : candidates) {
        SnapKind kind = SnapKind::Coincident;
        switch (direction.kind) {
            case geometry::DirectionSnapKind::AxisXParallel:
            case geometry::DirectionSnapKind::AxisYPerpendicular:
                kind = SnapKind::Horizontal;
                break;
            case geometry::DirectionSnapKind::AxisYParallel:
            case geometry::DirectionSnapKind::AxisXPerpendicular:
                kind = SnapKind::Vertical;
                break;
            case geometry::DirectionSnapKind::ParallelToLine:
                kind = SnapKind::Parallel;
                break;
            case geometry::DirectionSnapKind::PerpendicularToLine:
                kind = SnapKind::Perpendicular;
                break;
        }
        const double directionDistance = std::sqrt(std::max(0.0, direction.distanceSquared));
        if ((kind == SnapKind::Parallel || kind == SnapKind::Perpendicular) && direction.reference) {
            // A parallel direction through an anchor on the same infinite line
            // is collinearity, not a separate parallel relation.
            if (kind == SnapKind::Parallel) {
                for (const auto& entity : entities) {
                    if (entity.id != direction.reference->entity) continue;
                    const auto* line = std::get_if<Line2>(&entity.geometry);
                    if (!line || !isUsableLine(*line)) break;
                    const Vec2 v = subtract(line->end, line->start);
                    const double len2 = lengthSquared(v);
                    const double len = std::sqrt(len2);
                    const double offset = std::abs(cross(subtract(base, line->start), v)) / len;
                    const double anchorT = dot(subtract(base, line->start), v) / len2;
                    const double extensionDistance = (anchorT < 0.0) ? -anchorT * len :
                        ((anchorT > 1.0) ? (anchorT - 1.0) * len : 0.0);
                    if ((anchorT < 0.0 || anchorT > 1.0) && offset <= rule.maxRadius &&
                        extensionDistance <= rule.collinearRadius) {
                        kind = SnapKind::Collinear;
                    }
                    break;
                }
            }
        }
        if (kind == SnapKind::Collinear && directionDistance > rule.collinearRadius) continue;
        if (!isAllowed(kind, directionDistance, rule)) continue;
        SnapResult result;
        result.snapped = true;
        result.point = Point2{direction.endpoint};
        result.type = kind;
        result.angle = std::atan2(direction.unitDirection.y, direction.unitDirection.x);
        if (direction.reference) {
            result.objects.push_back(*direction.reference);
        }
        result.guideLines.push_back(Line2{base, direction.endpoint});
        const bool axisDirection = direction.kind == geometry::DirectionSnapKind::AxisXParallel ||
                                   direction.kind == geometry::DirectionSnapKind::AxisYParallel ||
                                   direction.kind == geometry::DirectionSnapKind::AxisXPerpendicular ||
                                   direction.kind == geometry::DirectionSnapKind::AxisYPerpendicular;
        // Parallel and perpendicular have equal priority. When both are valid,
        // candidate ranking uses geometric fit/distance rather than favoring
        // perpendicularity by kind. Axis H/V suggestions keep their own priority.
        int priority = axisDirection ? 700 : snapPriority(kind, request.action, request.drawState);
        if (kind == SnapKind::Collinear) priority = 760;
        SnapCandidate candidate{std::move(result), direction.distanceSquared, priority};
        if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    }
    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestStandalonePoint(const SnapRequest& request, const SnapRule& rule) const {
    auto entitiesResult = sketch_.entities();
    if (!entitiesResult) {
        return std::nullopt;
    }
    const auto nearest = geometry::findNearestStandalonePoint(request.cursor, entitiesResult.value());
    if (!nearest) {
        return std::nullopt;
    }
    const double d = std::sqrt(std::max(0.0, nearest->distanceSquared));
    if (!isAllowed(SnapKind::Coincident, d, rule)) {
        return std::nullopt;
    }
    return makeCandidate(nearest->point, SnapKind::Coincident, d, snapPriority(SnapKind::Coincident, request.action, request.drawState),
                         {nearest->source});
}

std::optional<SnapCandidate> SnapSystem::findBestLineCandidate(const SnapRequest& request, const SnapRule& rule) const {
    auto entitiesResult = sketch_.lines();
    if (!entitiesResult) {
        return std::nullopt;
    }

    std::optional<SnapCandidate> best;

    const auto consider = [&](SnapCandidate candidate) {
        if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    };

    // A line body is a positional coincidence, not a tangency. Keep its
    // priority below endpoints and the unified midpoint so it cannot steal
    // those more specific anchors when they are nearby.
    if (hasSnap(rule.mask, SnapMask::Coincident)) {
        if (auto allEntitiesResult = sketch_.entities()) {
            if (const auto nearest = geometry::findNearestLine(request.cursor, allEntitiesResult.value())) {
                const double d = std::sqrt(std::max(0.0, nearest->distanceSquared));
                if (isAllowed(SnapKind::Coincident, d, rule)) {
                    consider(makeCandidate(nearest->point, SnapKind::Coincident, d, 500, {nearest->source}));
                }
            }
        }
    }

    for (const auto& entity : entitiesResult.value()) {
        const auto* line = std::get_if<Line2>(&entity.geometry);
        if (!line || !isUsableLine(*line)) {
            continue;
        }

        const GeometryRef whole{entity.id, SubElement::Whole};
        const GeometryRef start{entity.id, SubElement::Start};
        const GeometryRef end{entity.id, SubElement::End};

        if (hasSnap(rule.mask, SnapMask::Coincident)) {
            const double startDistance = distance(request.cursor, line->start);

            if (isAllowed(SnapKind::Coincident, startDistance, rule)) {
                consider(makeCandidate(line->start, SnapKind::Coincident, startDistance,
                                       snapPriority(SnapKind::Coincident, request.action, request.drawState), {start}));
            }

            const double endDistance = distance(request.cursor, line->end);

            if (isAllowed(SnapKind::Coincident, endDistance, rule)) {
                consider(makeCandidate(line->end, SnapKind::Coincident, endDistance, snapPriority(SnapKind::Coincident, request.action, request.drawState),
                                       {end}));
            }
        }

        if (hasSnap(rule.mask, SnapMask::Midpoint)) {
            const Vec2 middle = midpoint(*line);
            const double middleDistance = distance(request.cursor, middle);

            if (isAllowed(SnapKind::Midpoint, middleDistance, rule)) {
                consider(makeCandidate(middle, SnapKind::Midpoint, middleDistance, snapPriority(SnapKind::Midpoint, request.action, request.drawState),
                                       {whole}));
            }
        }
    }

    // Initial placement: snap to the extension of an existing segment, but not
    // to its finite body (the body is Coincident) or endpoints (Coincident wins).
    if (hasSnap(rule.mask, SnapMask::Collinear) && request.drawState == DrawState::Start) {
        auto allEntitiesResult = sketch_.entities();
        if (allEntitiesResult) {
            for (const auto& entity : allEntitiesResult.value()) {
                const auto* line = std::get_if<Line2>(&entity.geometry);
                if (!line || !isUsableLine(*line)) continue;
                const Vec2 v = subtract(line->end, line->start);
                const double len2 = lengthSquared(v);
                const double t = dot(subtract(request.cursor, line->start), v) / len2;
                if (t >= 0.0 && t <= 1.0) continue;
                const double len = std::sqrt(len2);
                const double extensionDistance = (t < 0.0) ? -t * len : (t - 1.0) * len;
                // Collinearity is offered only within 15 screen pixels of either
                // finite endpoint, and within the normal lateral hit radius.
                if (extensionDistance > rule.collinearRadius) continue;
                const Vec2 projection{line->start.x + t * v.x, line->start.y + t * v.y};
                const double d = distance(request.cursor, projection);
                if (d > rule.maxRadius || !isAllowed(SnapKind::Collinear, d, rule)) continue;
                auto candidate = makeCandidate(projection, SnapKind::Collinear,
                    d, snapPriority(SnapKind::Collinear, request.action, request.drawState),
                    {{entity.id, SubElement::Whole}}, {Line2{line->start, projection}});
                consider(std::move(candidate));
            }
        }
    }

    // Continuation after the first point needs no metadata. If the committed
    // anchor lies on a line's extension, offer a point along that same direction
    // when the cursor is close to the parallel ray through the anchor.
    if (hasSnap(rule.mask, SnapMask::Collinear) && request.action == ActionType::Line &&
        request.drawState == DrawState::End && !request.anchors.empty() && isFinite(request.cursor)) {
        const Vec2 anchor = request.anchors.back().position;
        auto allEntitiesResult = sketch_.entities();
        if (isFinite(anchor) && allEntitiesResult) {
            for (const auto& entity : allEntitiesResult.value()) {
                const auto* line = std::get_if<Line2>(&entity.geometry);
                if (!line || !isUsableLine(*line)) continue;
                const Vec2 v = subtract(line->end, line->start);
                const double len2 = lengthSquared(v);
                const double anchorT = dot(subtract(anchor, line->start), v) / len2;
                const Vec2 anchorProjection{line->start.x + anchorT * v.x, line->start.y + anchorT * v.y};
                if (anchorT >= 0.0 && anchorT <= 1.0) continue;
                const double len = std::sqrt(len2);
                const double anchorExtensionDistance = (anchorT < 0.0) ? -anchorT * len : (anchorT - 1.0) * len;
                if (anchorExtensionDistance > rule.collinearRadius) continue;
                Vec2 unit{v.x / len, v.y / len};
                // Orient the ray away from the finite segment, whichever end
                // the anchor lies beyond.
                if (anchorT < 0.0) unit = multiply(unit, -1.0);
                const double t = dot(subtract(request.cursor, anchor), unit);
                if (t <= 0.0) continue;
                const Vec2 projection{anchor.x + t * unit.x, anchor.y + t * unit.y};
                const double d = distance(request.cursor, projection);
                if (d > rule.collinearRadius || !isAllowed(SnapKind::Collinear, d, rule)) continue;
                auto candidate = makeCandidate(projection, SnapKind::Collinear, d,
                    snapPriority(SnapKind::Collinear, request.action, request.drawState),
                    {{entity.id, SubElement::Whole}}, {Line2{anchor, projection}});
                consider(std::move(candidate));
            }
        }
    }

    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestCircleCandidate(const SnapRequest& request, const SnapRule& rule) const {
    auto entitiesResult = sketch_.entities();
    if (!entitiesResult) {
        return std::nullopt;
    }
    const auto& entities = entitiesResult.value();
    std::optional<SnapCandidate> best;
    const auto consider = [&](const std::optional<geometry::PointCandidate>& nearest, SnapKind kind) {
        if (!nearest) {
            return;
        }
        const double d = std::sqrt(std::max(0.0, nearest->distanceSquared));
        if (!isAllowed(kind, d, rule)) {
            return;
        }
        SnapCandidate candidate = makeCandidate(nearest->point, kind, d, snapPriority(kind, request.action, request.drawState), {nearest->source});
        if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    };
    if (hasSnap(rule.mask, SnapMask::Coincident)) {
        // Circle centers use the unified Coincident kind.
        consider(geometry::findNearestCircleCenter(request.cursor, entities), SnapKind::Coincident);
    }
    if (hasSnap(rule.mask, SnapMask::Coincident)) {
        consider(geometry::findNearestCircleBody(request.cursor, entities), SnapKind::Coincident);
    }
    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestIntersection(const SnapRequest& request, const SnapRule& rule) const {
    const bool allowReal = hasSnap(rule.mask, SnapMask::Intersection);
    const bool allowExtended = hasSnap(rule.mask, SnapMask::ExtendedIntersection) || hasSnap(rule.mask, SnapMask::VirtualIntersection);
    if (!allowReal && !allowExtended) {
        return std::nullopt;
    }
    auto entitiesResult = sketch_.entities();
    if (!entitiesResult) {
        return std::nullopt;
    }
    const auto& entities = entitiesResult.value();
    std::optional<SnapCandidate> best;
    const auto considerPoint = [&](const Vec2& point, SnapKind kind, double d2, GeometryRef first, GeometryRef second) {
        const double d = std::sqrt(std::max(0.0, d2));
        if (!isAllowed(kind, d, rule)) {
            return;
        }
        auto candidate = makeCandidate(point, kind, d, snapPriority(kind, request.action, request.drawState), {first, second});
        if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    };
    if (allowReal) {
        if (const auto real = geometry::findNearestIntersection(request.cursor, entities)) {
            considerPoint(real->point, SnapKind::Intersection, real->distanceSquared, real->first, real->second);
        }
    }
    if (allowExtended) {
        for (const auto& ext : geometry::findExtendedLineIntersections(request.cursor, entities)) {
            const bool bothExtended = ext.firstExtension.has_value() && ext.secondExtension.has_value();
            const auto kind = bothExtended ? SnapKind::VirtualIntersection : SnapKind::ExtendedIntersection;
            if (!hasSnap(rule.mask, maskForSnapKind(kind))) {
                continue;
            }
            const double extDistance = std::sqrt(std::max(0.0, ext.distanceSquared));
            if (!isAllowed(kind, extDistance, rule)) {
                continue;
            }
            auto candidate = makeCandidate(ext.point, kind, extDistance, snapPriority(kind, request.action, request.drawState), {ext.first, ext.second});
            if (ext.firstExtension) {
                candidate.result.guideLines.push_back(*ext.firstExtension);
            }
            if (ext.secondExtension) {
                candidate.result.guideLines.push_back(*ext.secondExtension);
            }
            if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) {
                best = std::move(candidate);
            }
        }
    }
    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestTangentCandidate(const SnapRequest& request, const SnapRule& rule) const {
    if (request.anchors.empty() || !isFinite(request.cursor)) {
        return std::nullopt;
    }
    // Arc's middle click uses the nearest point on an existing arc/circle/line
    // body (handled by findBestArcCandidate), not curve-placement tangency from
    // the first anchor. This query is for line and circle placement only.
    if (request.action != ActionType::Line && request.action != ActionType::Circle && request.action != ActionType::Arc) {
        return std::nullopt;
    }
    if (request.drawState != DrawState::End && request.drawState != DrawState::Body) {
        return std::nullopt;
    }

    const Vec2 anchor = request.anchors.back().position;
    if (!isFinite(anchor)) {
        return std::nullopt;
    }
    auto entitiesResult = sketch_.entities();
    if (!entitiesResult) {
        return std::nullopt;
    }

    std::optional<algorithms::TangencyCandidate> candidate;
    if (request.action == ActionType::Line && request.drawState == DrawState::End) {
        candidate = algorithms::findLinePlacementTangency(
            anchor, request.cursor, entitiesResult.value(), request.forbiddenObjects, rule.tangentRadius);
    } else if ((request.action == ActionType::Circle || request.action == ActionType::Arc) && request.drawState == DrawState::Body) {
        candidate = algorithms::findCurvePlacementTangency(
            anchor, request.cursor, entitiesResult.value(), request.forbiddenObjects, rule.tangentRadius);
    }
    if (!candidate) {
        return std::nullopt;
    }

    const double candidateDistance = std::sqrt(std::max(0.0, candidate->distanceSquared));
    if (candidateDistance > rule.tangentRadius || !isAllowed(SnapKind::Tangent, candidateDistance, rule)) {
        return std::nullopt;
    }

    std::vector<Line2> guides;
    if (request.action == ActionType::Line) {
        guides.push_back(Line2{anchor, candidate->point});
    }
    return makeCandidate(candidate->point, SnapKind::Tangent, candidateDistance,
        snapPriority(SnapKind::Tangent, request.action, request.drawState), {candidate->object}, std::move(guides));
}

std::optional<SnapCandidate> SnapSystem::findBestArcCandidate(const SnapRequest& request, const SnapRule& rule) const {
    const bool allowBody = hasSnap(rule.mask, SnapMask::Coincident);
    const bool allowEndpoint = hasSnap(rule.mask, SnapMask::Coincident);
    const bool allowCentre = hasSnap(rule.mask, SnapMask::Coincident);
    const bool allowMidpoint = hasSnap(rule.mask, SnapMask::Midpoint);

    if (!allowBody && !allowEndpoint && !allowCentre && !allowMidpoint) {
        return std::nullopt;
    }

    auto arcsResult = sketch_.arcs();
    if (!arcsResult) {
        return std::nullopt;
    }

    const auto& arcs = arcsResult.value();
    std::optional<SnapCandidate> best;

    const auto consider = [&](const auto& nearest, SnapKind kind, int priority) {
        if (!nearest) {
            return;
        }

        const double candidateDistance = distance(request.cursor, nearest->point);

        if (!isAllowed(kind, candidateDistance, rule)) {
            return;
        }

        auto candidate = makeCandidate(nearest->point, kind, candidateDistance, priority, {nearest->source});

        if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    };

    if (allowEndpoint) {
        consider(geometry::findNearestArcEndpoint(request.cursor, arcs), SnapKind::Coincident,
                 snapPriority(SnapKind::Coincident, request.action, request.drawState));
    }

    if (allowCentre) {
        // Arc centers also use Coincident, matching the unified snap taxonomy.
        consider(geometry::findNearestArcCenter(request.cursor, arcs), SnapKind::Coincident,
                 snapPriority(SnapKind::Coincident, request.action, request.drawState));
    }
    if (hasSnap(rule.mask, SnapMask::Midpoint)) {
        for (const auto& entity : arcs) {
            const auto* arc = std::get_if<geometry::Arc2>(&entity.geometry);
            if (!arc) continue;
            const auto middle = geometry::detail::arcMidpoint(*arc);
            if (!middle) continue;
            const double d = distance(request.cursor, *middle);
            if (isAllowed(SnapKind::Midpoint, d, rule)) {
                auto c = makeCandidate(*middle, SnapKind::Midpoint, d,
                    snapPriority(SnapKind::Midpoint, request.action, request.drawState), {{entity.id, SubElement::Whole}});
                if (!best || isBetterCandidate(c, *best, rule.priorityRadius)) best = std::move(c);
            }
        }
    }

    if (allowBody) {
        consider(geometry::findNearestArcBody(request.cursor, arcs), SnapKind::Coincident, 520);
    }

    return best;
}

}  // namespace snap