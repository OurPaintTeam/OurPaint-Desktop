#include "SnapSystem.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

#include "Camera2D.h"
#include "CoincidentCollectors.h"
#include "IntersectionCollectors.h"
#include "LineAlgorithms.h"
#include "MidpointCollectors.h"
#include "Sketch.h"
#include "SketchTypes.h"

namespace snap {
namespace {
using namespace core::sketch;

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
}  // namespace

SnapSystem::SnapSystem(const core::sketch::Sketch& sketch, const render::Camera2D& camera) : sketch_(sketch), camera_(camera) {}

std::vector<SnapResult> SnapSystem::getSnapCandidates(const SnapRequest& request) const { return parserRequest(request); }

std::vector<SnapResult> SnapSystem::parserRequest(const SnapRequest& snapRequest) const {
    std::vector<SnapResult> results;
    if (snapRequest.action == ActionType::Draw) {
        return snapForDrawing(snapRequest);
    } else {
    }
    return results;
}

std::vector<SnapResult> SnapSystem::snapForDrawing(const SnapRequest& request) const {
    std::vector<SnapResult> results;
    if (request.action != ActionType::Draw) {
        return results;
    }

    const SnapMask mask = snapMaskFor(request.primitive, request.drawState);
    if (mask == SnapMask::None) {
        return results;
    }

    std::optional<SnapCandidate> best;

    auto append = [&best](std::optional<SnapCandidate> c) {
        if (!c) {
            return;
        }
        if (!best || c->priority > best->priority || (c->priority == best->priority && c->distanceSquared < best->distanceSquared)) {
            best = std::move(c);
        }
    };

    for (SnapKind kind : kAllSnapKinds) {
        if (!hasSnap(mask, kind)) {
            continue;
        }
        switch (kind) {
            case SnapKind::Coincident:
                append(collectCoincident(request));
                break;
            case SnapKind::Midpoint:
                append(collectMidpoint(request));
                break;
            case SnapKind::Intersection: append(collectIntersection(request));
                break;
            case SnapKind::ExtendedIntersection: append(collectExtendedIntersection(request));
                break;
            case SnapKind::VirtualIntersection: append(collectVirtualIntersection(request));
                break;
            case SnapKind::Tangent: /*append(collectTangent(request));*/
                break;

            case SnapKind::Horizontal: /*append(collectHorizontal(request));*/
                break;
            case SnapKind::Vertical: /*append(collectVertical(request));*/
                break;
            case SnapKind::Parallel: /*append(collectParallel(request));*/
                break;
            case SnapKind::Perpendicular: /*append(collectPerpendicular(request));*/
                break;
            case SnapKind::AngleToLine: /*append(collectAngleToLine(request));*/
                break;
            case SnapKind::Collinear: /*append(collectCollinear(request));*/
                break;
            case SnapKind::None:
                break;
        }
    }
    if (best) results.push_back(std::move(best->result));
    return results;
}

std::optional<SnapCandidate> SnapSystem::collectCoincident(const SnapRequest& request) const {
    const double tolWorld = camera_.screenLogicalToWorld(snapTolerancePixels(request.primitive, request.drawState));

    std::optional<SnapCandidate> best;

    auto mergeIn = [&](std::optional<SnapCandidate> c) {
        if (!c) {
            return;
        }
        if (!best || c->priority > best->priority || (c->priority == best->priority && c->distanceSquared < best->distanceSquared)) {
            best = std::move(c);
        }
    };

    if (auto pts = sketch_.points()) {
        mergeIn(coincident::collectFromPoints(request, pts.value(), tolWorld));
    }
    if (auto lines = sketch_.lines()) {
        mergeIn(coincident::collectFromLines(request, lines.value(), tolWorld));
    }
    if (auto circles = sketch_.circles()) {
        mergeIn(coincident::collectFromCircles(request, circles.value(), tolWorld));
    }
    if (auto arcs = sketch_.arcs()) {
        mergeIn(coincident::collectFromArcs(request, arcs.value(), tolWorld));
    }

    return best;
}
std::optional<SnapCandidate> SnapSystem::collectMidpoint(const SnapRequest& request) const {
    const double tolWorld = camera_.screenLogicalToWorld(snapTolerancePixels(request.primitive, request.drawState));

    std::optional<SnapCandidate> best;

    if (auto lines = sketch_.lines()) {
        best = midpoint::collectFromLines(request, lines.value(), tolWorld);
    }

    return best;
}

namespace {
std::optional<SnapCandidate>
runIntersection(const core::sketch::Sketch& sketch,
                const render::Camera2D& camera,
                const SnapRequest& request,
                SnapKind wantKind) {
    const double tolWorld = camera.screenLogicalToWorld(
        snapTolerancePixels(request.primitive, request.drawState));

    auto entities = sketch.entities();
    if (!entities) return std::nullopt;

    return intersection::collectFromEntities(
        request, entities.value(), tolWorld, wantKind);
}
}

std::optional<SnapCandidate>
SnapSystem::collectIntersection(const SnapRequest& request) const {
    return runIntersection(sketch_, camera_, request, SnapKind::Intersection);
}

std::optional<SnapCandidate>
SnapSystem::collectExtendedIntersection(const SnapRequest& request) const {
    return runIntersection(sketch_, camera_, request,
                           SnapKind::ExtendedIntersection);
}

std::optional<SnapCandidate>
SnapSystem::collectVirtualIntersection(const SnapRequest& request) const {
    return runIntersection(sketch_, camera_, request,
                           SnapKind::VirtualIntersection);
}

}  // namespace snap