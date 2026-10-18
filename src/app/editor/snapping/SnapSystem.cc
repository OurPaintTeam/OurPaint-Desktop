#include "SnapSystem.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

#include "Algorithms.h"
#include "Camera2D.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"

namespace snap {
namespace {

using namespace core::sketch;

constexpr double kDefaultSnapRadiusPixels = 5.0;

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

SnapSystem::SnapSystem(const core::sketch::Sketch& sketch,const Camera2D& camera) : sketch_(sketch), camera_(camera) {
    zoom_ = camera_.zoom();
}

SnapResult SnapSystem::getSnapCandidate(const SnapRequest& request) const { return parser(request); }

SnapRule SnapSystem::ruleFor(ToolType tool, DrawState state) {
    using M = SnapMask;

    SnapRule rule;
    // The effective radius is converted from screen pixels to world units in parser().
    rule.maxRadius = kDefaultSnapRadiusPixels;
    rule.priorityRadius = kDefaultSnapRadiusPixels;

    constexpr M commonPosition = M::StandalonePoint | M::LineEndpoint | M::LineMidpoint | M::LineBody | M::CircleBody | M::CircleCentre | M::Intersection |
                                 M::ArcBody | M::ArcEndpoint | M::ArcCentre | M::ExtendedIntersection | M::VirtualIntersection;

    switch (tool) {

        case ToolType::Move:
            rule.mask = commonPosition;
            break;

        case ToolType::Point:
        case ToolType::Circle:
        case ToolType::Arc:
            rule.mask = commonPosition;
            break;

        case ToolType::Line:
            rule.mask = commonPosition;

            // FirstPoint means the first point has already been placed.
            // Directional constraints are useful while positioning the second point.
            if (state == DrawState::FirstPoint) {
                rule.mask |= Direction;
            }

            break;

        case ToolType::Rotate:
        case ToolType::Scale:
            rule.mask = M::StandalonePoint | M::LineEndpoint | M::CircleCentre | M::Intersection;
            break;

        default:
            rule.mask = M::None;
            break;
    }

    return rule;
}

SnapResult SnapSystem::parser(const SnapRequest& request) const {
    SnapRule rule = ruleFor(request.tool, request.drawState);

    // All distances produced by geometry queries are in world units. Convert the
    // pixel hit tolerance to world units so the snap target stays the same size
    // on screen at every zoom level.
    // A missing/invalid zoom must not disable every snap (especially endpoints).
    // SnapRequest defaults to 1.0; use the same safe fallback if a caller passes
    // zero, a negative value, NaN, or infinity. The view integration should still
    // pass its actual world-to-screen scale for exact pixel tolerance.
    const double zoom = (std::isfinite(zoom_) && zoom_ > 1e-9) ? zoom_ : 1.0;
    rule.maxRadius /= zoom;
    rule.priorityRadius /= zoom;

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

        if (!best || isBetterCandidate(*candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    };

    if (hasSnap(rule.mask, SnapMask::StandalonePoint)) {
        consider(findBestStandalonePoint(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::LineEndpoint) || hasSnap(rule.mask, SnapMask::LineMidpoint) || hasSnap(rule.mask, SnapMask::LineBody)) {
        consider(findBestLineCandidate(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::CircleBody) || hasSnap(rule.mask, SnapMask::CircleCentre)) {
        consider(findBestCircleCandidate(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::Intersection) || hasSnap(rule.mask, SnapMask::ExtendedIntersection) || hasSnap(rule.mask, SnapMask::VirtualIntersection) ||
        hasSnap(rule.mask, SnapMask::Tangent)) {
        consider(findBestIntersection(request, rule));
    }

    if (hasSnap(rule.mask, SnapMask::ArcBody) || hasSnap(rule.mask, SnapMask::ArcEndpoint) || hasSnap(rule.mask, SnapMask::ArcCentre)) {
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
    if (request.anchors.empty() || request.tool != ToolType::Line || request.drawState != DrawState::FirstPoint) {
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
    const auto candidates = geometry::findDirectionSnaps(base, request.cursor, entitiesResult.value());
    std::optional<SnapCandidate> best;
    for (const auto& direction : candidates) {
        SnapKind kind = SnapKind::None;
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
        if (!isAllowed(kind, std::sqrt(std::max(0.0, direction.distanceSquared)), rule)) {
            continue;
        }
        SnapResult result;
        result.snapped = true;
        result.point = Point2{direction.endpoint};
        result.type = kind;
        result.angle = std::atan2(direction.unitDirection.y, direction.unitDirection.x);
        if (direction.reference) {
            result.objects.push_back(*direction.reference);
        }
        result.guideLines.push_back(Line2{base, direction.endpoint});
        SnapCandidate candidate{std::move(result), direction.distanceSquared, snapPriority(kind, request.tool, request.drawState)};
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
    if (!isAllowed(SnapKind::StandalonePoint, d, rule)) {
        return std::nullopt;
    }
    return makeCandidate(nearest->point, SnapKind::StandalonePoint, d, snapPriority(SnapKind::StandalonePoint, request.tool, request.drawState),
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

    // The shared query owns nearest-point-on-segment selection across all lines.
    if (hasSnap(rule.mask, SnapMask::LineBody)) {
        if (auto allEntitiesResult = sketch_.entities()) {
            if (const auto nearest = geometry::findNearestLine(request.cursor, allEntitiesResult.value())) {
                if (const double d = std::sqrt(std::max(0.0, nearest->distanceSquared)); isAllowed(SnapKind::LineBody, d, rule)) {
                    consider(makeCandidate(nearest->point, SnapKind::LineBody, d, snapPriority(SnapKind::LineBody, request.tool, request.drawState),
                                           {nearest->source}));
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

        if (hasSnap(rule.mask, SnapMask::LineEndpoint)) {
            const double startDistance = distance(request.cursor, line->start);

            if (isAllowed(SnapKind::LineEndpoint, startDistance, rule)) {
                consider(makeCandidate(line->start, SnapKind::LineEndpoint, startDistance,
                                       snapPriority(SnapKind::LineEndpoint, request.tool, request.drawState), {start}));
            }

            const double endDistance = distance(request.cursor, line->end);

            if (isAllowed(SnapKind::LineEndpoint, endDistance, rule)) {
                consider(makeCandidate(line->end, SnapKind::LineEndpoint, endDistance, snapPriority(SnapKind::LineEndpoint, request.tool, request.drawState),
                                       {end}));
            }
        }

        if (hasSnap(rule.mask, SnapMask::LineMidpoint)) {
            const Vec2 middle = midpoint(*line);
            const double middleDistance = distance(request.cursor, middle);

            if (isAllowed(SnapKind::LineMidpoint, middleDistance, rule)) {
                consider(makeCandidate(middle, SnapKind::LineMidpoint, middleDistance, snapPriority(SnapKind::LineMidpoint, request.tool, request.drawState),
                                       {whole}));
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
        SnapCandidate candidate = makeCandidate(nearest->point, kind, d, snapPriority(kind, request.tool, request.drawState), {nearest->source});
        if (!best || isBetterCandidate(candidate, *best, rule.priorityRadius)) {
            best = std::move(candidate);
        }
    };
    if (hasSnap(rule.mask, SnapMask::CircleCentre)) {
        consider(geometry::findNearestCircleCenter(request.cursor, entities), SnapKind::CircleCentre);
    }
    if (hasSnap(rule.mask, SnapMask::CircleBody)) {
        consider(geometry::findNearestCircleBody(request.cursor, entities), SnapKind::CircleBody);
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
        auto candidate = makeCandidate(point, kind, d, snapPriority(kind, request.tool, request.drawState), {first, second});
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
            auto candidate = makeCandidate(ext.point, kind, extDistance, snapPriority(kind, request.tool, request.drawState), {ext.first, ext.second});
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

std::optional<SnapCandidate> SnapSystem::findBestArcCandidate(const SnapRequest& request, const SnapRule& rule) const {
    const bool allowBody = hasSnap(rule.mask, SnapMask::ArcBody);
    const bool allowEndpoint = hasSnap(rule.mask, SnapMask::ArcEndpoint);
    const bool allowCentre = hasSnap(rule.mask, SnapMask::ArcCentre);

    if (!allowBody && !allowEndpoint && !allowCentre) {
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
        consider(geometry::findNearestArcEndpoint(request.cursor, arcs), SnapKind::ArcEndpoint,
                 snapPriority(SnapKind::LineEndpoint, request.tool, request.drawState));
    }

    if (allowCentre) {
        consider(geometry::findNearestArcCenter(request.cursor, arcs), SnapKind::ArcCentre,
                 snapPriority(SnapKind::CircleCentre, request.tool, request.drawState));
    }

    if (allowBody) {
        consider(geometry::findNearestArcBody(request.cursor, arcs), SnapKind::ArcBody, snapPriority(SnapKind::CircleBody, request.tool, request.drawState));
    }

    return best;
}

}  // namespace snap