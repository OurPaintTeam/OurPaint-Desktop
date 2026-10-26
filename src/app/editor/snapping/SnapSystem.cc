#include "SnapSystem.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

#include "Algorithms.h"
#include "Camera2D.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"
#include "snapping/algorithms/SnapEntityQuery.h"
#include "snapping/algorithms/SnapTangencyQuery.h"

namespace snap {
namespace {

using namespace core::sketch;

constexpr double kDefaultSnapRadiusPixels = 5.0;
constexpr double kEndpointSnapRadiusPixels = 11.0;
constexpr double kMidpointSnapRadiusPixels = 11.0;
constexpr double kCollinearSnapRadiusPixels = 20.0; // distance along the extension, in screen pixels
constexpr double kCollinearLateralRadiusPixels = 4.5; // lateral tolerance stays narrow to avoid stealing endpoint/midpoint snaps
constexpr double kLineBodySnapRadiusPixels = 3.5;
constexpr double kParallelSegmentProximityPixels = 50.0;
constexpr double kTangentSnapRadiusPixels = 7.0;

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

bool sameRefs(const std::vector<GeometryRef>& a, const std::vector<GeometryRef>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].entity != b[i].entity || a[i].sub != b[i].sub) return false;
    }
    return true;
}

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
    rule.maxRadius = kEndpointSnapRadiusPixels; // broad query radius; final hit-testing uses per-kind radii
    rule.regularRadius = kDefaultSnapRadiusPixels;
    rule.priorityRadius = 1.5;
    rule.collinearRadius = kCollinearSnapRadiusPixels;
    rule.tangentRadius = kTangentSnapRadiusPixels;
    rule.endpointRadius = kEndpointSnapRadiusPixels;
    rule.midpointRadius = kMidpointSnapRadiusPixels;
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
    rule.regularRadius /= zoom;
    rule.priorityRadius /= zoom;
    rule.collinearRadius /= zoom;
    rule.tangentRadius /= zoom;
    rule.endpointRadius /= zoom;
    rule.midpointRadius /= zoom;

    if (rule.mask == SnapMask::None || !isFinite(request.cursor) || !std::isfinite(rule.maxRadius) || rule.maxRadius <= 0.0) {
        return {};
    }

    std::vector<SnapCandidate> candidates;
    const auto consider = [&](std::optional<SnapCandidate> candidate) {
        if (!candidate) return;
        if (!std::isfinite(candidate->distanceSquared) || candidate->distanceSquared < 0.0) return;
        const double candidateDistance = std::sqrt(candidate->distanceSquared);
        double allowedRadius = rule.regularRadius;
        if (candidate->result.type == SnapKind::Midpoint) {
            allowedRadius = rule.midpointRadius;
        } else if (candidate->result.type == SnapKind::Tangent) {
            allowedRadius = rule.tangentRadius;
        } else if (candidate->result.type == SnapKind::Collinear) {
            allowedRadius = kCollinearLateralRadiusPixels / zoom;
        } else if (candidate->result.type == SnapKind::Coincident) {
            const bool endpointTarget = std::any_of(candidate->result.objects.begin(), candidate->result.objects.end(), [](const GeometryRef& ref) {
                return ref.sub == SubElement::Start || ref.sub == SubElement::End;
            });
            const bool wholeGeometryTarget = !candidate->result.objects.empty() && std::all_of(
                candidate->result.objects.begin(), candidate->result.objects.end(), [](const GeometryRef& ref) { return ref.sub == SubElement::Whole; });
            if (endpointTarget) allowedRadius = rule.endpointRadius;
            else if (wholeGeometryTarget) allowedRadius = kLineBodySnapRadiusPixels / zoom;
        }
        if (candidateDistance > allowedRadius) return;
        // A forbidden object must never leak through a candidate path, even if
        // a geometry query forgot to apply the exclusion itself.
        if (std::any_of(candidate->result.objects.begin(), candidate->result.objects.end(), [&](const GeometryRef& ref) {
            return algorithms::isForbiddenEntity(ref.entity, request.forbiddenObjects);
        })) return;
        candidates.push_back(std::move(*candidate));
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

    if (candidates.empty()) {
        hasPreviousQuery_ = false;
        previousCycleIndex_ = 0;
        return {};
    }

    // Collapse duplicate results from different query paths, then sort by the
    // normal snap ranking. Endpoint coincidences suppress intersections and
    // collinearity within the endpoint capture area; the endpoint is the useful
    // semantic target when two segments share a vertex.
    const auto candidatesBeforeEndpointFiltering = candidates;
    candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [&](const SnapCandidate& candidate) {
        const bool intersection = candidate.result.type == SnapKind::Intersection ||
            candidate.result.type == SnapKind::ExtendedIntersection || candidate.result.type == SnapKind::VirtualIntersection;
        const bool collinear = candidate.result.type == SnapKind::Collinear;
        if (!intersection && !collinear) return false;
        return std::any_of(candidatesBeforeEndpointFiltering.begin(), candidatesBeforeEndpointFiltering.end(), [&](const SnapCandidate& other) {
            if (other.result.type != SnapKind::Coincident || other.result.objects.empty()) return false;
            const bool endpoint = std::any_of(other.result.objects.begin(), other.result.objects.end(), [](const GeometryRef& ref) {
                return ref.sub == SubElement::Start || ref.sub == SubElement::End;
            });
            if (!endpoint) return false;
            // Endpoint acquisition is based on cursor distance, not just the
            // distance between two projected candidate points.
            const double endpointCursorDistance = std::sqrt(std::max(0.0, other.distanceSquared));
            if (endpointCursorDistance > rule.endpointRadius) return false;
            return intersection ? distance(other.result.point.position, candidate.result.point.position) <= rule.endpointRadius : true;
        });
    }), candidates.end());

    std::stable_sort(candidates.begin(), candidates.end(), [](const SnapCandidate& a, const SnapCandidate& b) {
        if (a.priority != b.priority) return a.priority > b.priority;
        return a.distanceSquared < b.distanceSquared;
    });
    candidates.erase(std::unique(candidates.begin(), candidates.end(), [](const SnapCandidate& a, const SnapCandidate& b) {
        return a.result.type == b.result.type &&
               distance(a.result.point.position, b.result.point.position) <= 1e-8 &&
               sameRefs(a.result.objects, b.result.objects);
    }), candidates.end());

    bool sameContext = hasPreviousQuery_ && previousAction_ == request.action && previousDrawState_ == request.drawState &&
        previousAnchors_.size() == request.anchors.size() && sameRefs(previousForbiddenObjects_, request.forbiddenObjects);
    if (sameContext) {
        for (std::size_t i = 0; i < request.anchors.size(); ++i) {
            if (distance(previousAnchors_[i], request.anchors[i].position) > 1e-8) { sameContext = false; break; }
        }
    }
    const double cursorDriftPixels = distance(previousCursor_, request.cursor) * zoom;
    if (sameContext && cursorDriftPixels <= 3.0) {
        // Keep the cycle alive for tiny cursor movements. Only advance when the
        // cursor moved enough to indicate the user is asking for an alternative.
        if (cursorDriftPixels > 0.75) previousCycleIndex_++;
    } else {
        previousCycleIndex_ = 0;
    }
    hasPreviousQuery_ = true;
    previousAction_ = request.action;
    previousDrawState_ = request.drawState;
    previousCursor_ = request.cursor;
    previousAnchors_.clear();
    for (const auto& anchor : request.anchors) previousAnchors_.push_back(anchor.position);
    previousForbiddenObjects_ = request.forbiddenObjects;
    previousCycleIndex_ %= candidates.size();
    return candidates[previousCycleIndex_].result;
}

bool SnapSystem::isAllowed(SnapKind kind, double candidateDistance, const SnapRule& rule) {
    if (!std::isfinite(candidateDistance) || candidateDistance < 0.0 || !std::isfinite(rule.maxRadius) || candidateDistance > rule.maxRadius) {
        return false;
    }
    const SnapMask required = maskForSnapKind(kind);
    return required != SnapMask::None && hasSnap(rule.mask, required);
}

void SnapSystem::onAnchorPlaced(const geometry::Vec2& position) {
    directionSnapBaseAnchor_ = position;
    directionSnapReferenceZoom_ = camera_.zoom();
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
    if (!entitiesResult) return std::nullopt;
    const auto entities = algorithms::filterForbiddenEntities(entitiesResult.value(), request.forbiddenObjects);


    const double zoom = std::max(camera_.zoom(), 1e-9);

    if (!directionSnapBaseAnchor_ || distance(*directionSnapBaseAnchor_, base) > geometry::EPSILON) {
        directionSnapBaseAnchor_ = base;
        directionSnapReferenceZoom_ = zoom;
    }
    const double referenceZoom = std::max(directionSnapReferenceZoom_, 1e-9);
    const double ratio = zoom / referenceZoom;

    // Piecewise exponent: zoom-out uses 0.5 (gives 15/30/45/60 across a few
    // decades), zoom-in uses 0.368 so that ratio = 833333 lands exactly on
    // 0.1° — the full zoom-in range is useful.
    //   ratio = 0.1   -> ideal 60
    //   ratio = 1     -> ideal 15  (base, wherever the anchor was placed)
    //   ratio = 833333 -> ideal 0.1
    constexpr double kZoomInAlpha  = 0.3676;  // ln(150) / ln(833333)
    constexpr double kZoomOutAlpha = 0.5;
    const double alpha = (ratio >= 1.0) ? kZoomInAlpha : kZoomOutAlpha;
    const double idealStep = 15.0 * std::pow(ratio, -alpha);

    // Snap to a pretty value so the user never sees 10.4167 or 40.1878.
    const double angleStepDegrees = geometry::snapToNiceAngleStep(idealStep);

    // Tolerance: keep a free zone for coarse grids (15°, 30° ...) so the user
    // can still draw off-grid lines. For fine grids (< 2°) quantize fully
    // (tolerance = step/2) — sub-pixel gaps are unreachable with a mouse.
    const double toleranceDegrees = (angleStepDegrees >= 2.0)
        ? angleStepDegrees * 0.4
        : angleStepDegrees * 0.5;

    auto candidates = geometry::findDirectionSnaps(base, request.cursor, entities,
                                                   toleranceDegrees, angleStepDegrees);
    std::cout<<zoom<<" "<<angleStepDegrees<<std::endl;

    candidates.erase(std::remove_if(candidates.begin(), candidates.end(), [&](const auto& candidate) {
        if (!candidate.reference) return false; // global H/V axes
        const auto it = std::find_if(entities.begin(), entities.end(), [&](const auto& entity) {
            return entity.id == candidate.reference->entity;
        });
        if (it == entities.end()) return true;
        const auto* line = std::get_if<Line2>(&it->geometry);
        if (!line || !isUsableLine(*line)) return true;
        if (candidate.kind == geometry::DirectionSnapKind::AngleToLine) {
            // Angle-to-line constraints require a shared endpoint with the
            // existing reference line; unlike perpendicularity, body contacts
            // are not treated as a common vertex for this relation.
            const double endpointGap = std::min(distance(base, line->start), distance(base, line->end));
            const double endpointTolerance = std::max(rule.endpointRadius,
                kLineBodySnapRadiusPixels / std::max(camera_.zoom(), 1e-9));
            return endpointGap > endpointTolerance;
        }
        if (candidate.kind == geometry::DirectionSnapKind::PerpendicularToLine) {
            // Perpendicularity is valid when the new segment starts at an
            // endpoint, midpoint, or any point on the body of the reference
            // segment. Previously this required a shared endpoint, which made
            // midpoint/body placement silently lose the perpendicular snap.
            const double startGap = distance(base, line->start);
            const double endGap = distance(base, line->end);
            const double bodyGap = distance(base, closestPointOnSegment(base, *line).value());
            const double anchorTolerance = std::max(rule.endpointRadius,
                kLineBodySnapRadiusPixels / std::max(camera_.zoom(), 1e-9));
            return std::min({startGap, endGap, bodyGap}) > anchorTolerance;
        }
        if (candidate.kind == geometry::DirectionSnapKind::ParallelToLine) {
            const double cursorToSegment = distance(request.cursor, closestPointOnSegment(request.cursor, *line).value());
            if (cursorToSegment > kParallelSegmentProximityPixels / std::max(camera_.zoom(), 1e-9)) return true;
            const Vec2 v = subtract(line->end, line->start);
            const double len = std::sqrt(lengthSquared(v));
            if (len <= 1e-12) return true;
            const double baseOffset = std::abs(cross(subtract(base, line->start), v)) / len;
            // If the anchor is already on this line (or its continuation), this is
            // collinearity, not a parallel relation. The dedicated collinear rule
            // decides whether the extension is close enough to offer.
            if (baseOffset <= kCollinearLateralRadiusPixels / std::max(camera_.zoom(), 1e-9)) return true;
        }
        return false;
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
            case geometry::DirectionSnapKind::AngleToLine:
                kind = SnapKind::AngleToLine;
                break;
        }
        const double directionDistance = std::sqrt(std::max(0.0, direction.distanceSquared));
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
        // Parallel and perpendicular have equal priority. When both are valid,
        // candidate ranking uses geometric fit/distance rather than favoring
        // perpendicularity by kind. Axis H/V suggestions keep their own priority.
        int priority = snapPriority(kind, request.action, request.drawState);
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
    const auto entities = algorithms::filterForbiddenEntities(entitiesResult.value(), request.forbiddenObjects);
    const auto nearest = geometry::findNearestStandalonePoint(request.cursor, entities);
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
    auto entitiesResult = sketch_.entities();
    if (!entitiesResult) return std::nullopt;
    const auto entities = algorithms::filterForbiddenEntities(entitiesResult.value(), request.forbiddenObjects);

    const bool allowCoincident = hasSnap(rule.mask, SnapMask::Coincident);
    const bool allowMidpoint = hasSnap(rule.mask, SnapMask::Midpoint);
    const bool allowCollinear = hasSnap(rule.mask, SnapMask::Collinear);
    const bool initialCollinear = allowCollinear && request.drawState == DrawState::Start;
    const bool continuationCollinear = allowCollinear && request.action == ActionType::Line &&
        request.drawState == DrawState::End && !request.anchors.empty();

    const auto hits = algorithms::queryLineFeatures(
        request.cursor, entities,
        allowCoincident, allowMidpoint, allowCoincident,
        initialCollinear, continuationCollinear, request.anchors,
        rule.endpointRadius, rule.midpointRadius, kLineBodySnapRadiusPixels / std::max(camera_.zoom(), 1e-9), rule.collinearRadius,
        kCollinearLateralRadiusPixels / std::max(camera_.zoom(), 1e-9));

    std::optional<SnapCandidate> best;
    for (const auto& hit : hits) {
        SnapKind kind = SnapKind::Coincident;
        int priority = 500;
        switch (hit.feature) {
        case algorithms::LineFeature::Body:
            kind = SnapKind::Coincident;
            priority = 500;
            break;
        case algorithms::LineFeature::Start:
        case algorithms::LineFeature::End:
            kind = SnapKind::Coincident;
            priority = snapPriority(kind, request.action, request.drawState);
            break;
        case algorithms::LineFeature::Midpoint:
            kind = SnapKind::Midpoint;
            priority = snapPriority(kind, request.action, request.drawState);
            break;
        case algorithms::LineFeature::CollinearExtension:
            kind = SnapKind::Collinear;
            priority = snapPriority(kind, request.action, request.drawState);
            break;
        }
        if (!isAllowed(kind, hit.distance, rule)) continue;
        auto candidate = makeCandidate(hit.point, kind, hit.distance, priority, {hit.source},
            hit.guide ? std::vector<Line2>{*hit.guide} : std::vector<Line2>{});
        if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) best = std::move(candidate);
    }
    return best;
}

std::optional<SnapCandidate> SnapSystem::findBestCircleCandidate(const SnapRequest& request, const SnapRule& rule) const {
    auto entitiesResult = sketch_.entities();
    if (!entitiesResult) {
        return std::nullopt;
    }
    const auto entities = algorithms::filterForbiddenEntities(entitiesResult.value(), request.forbiddenObjects);
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
    const auto entities = algorithms::filterForbiddenEntities(entitiesResult.value(), request.forbiddenObjects);
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

    const auto entities = algorithms::filterForbiddenEntities(entitiesResult.value(), request.forbiddenObjects);
    std::optional<algorithms::TangencyCandidate> candidate;
    if (request.action == ActionType::Line && request.drawState == DrawState::End) {
        candidate = algorithms::findLinePlacementTangency(
            anchor, request.cursor, entities, request.forbiddenObjects, rule.tangentRadius);
    } else if ((request.action == ActionType::Circle || request.action == ActionType::Arc) && request.drawState == DrawState::Body) {
        candidate = algorithms::findCurvePlacementTangency(
            anchor, request.cursor, entities, request.forbiddenObjects, rule.tangentRadius);
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

    const auto arcs = algorithms::filterForbiddenEntities(arcsResult.value(), request.forbiddenObjects);
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