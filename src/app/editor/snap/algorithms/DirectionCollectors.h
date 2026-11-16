#ifndef OURPAINT_DIRECTIONCOLLECTORS_H
#define OURPAINT_DIRECTIONCOLLECTORS_H

#include <cmath>
#include <variant>
#include <vector>

#include "GeometryMath.h"
#include "SketchTypes.h"
#include "SnapSystem.h"
#include "SnapTypes.h"
#include "AxisRelations.h"
#include "LineAlgorithms.h"
#include "CircleAlgorithms.h"
#include "ArcAlgorithms.h"

namespace snap {
struct SnapRequest;
}

namespace snap::direction {
namespace {

using namespace core::sketch;

bool isAnchorConnected(const Vec2& anchor, std::span<const SketchEntity> entities) {
    constexpr double kEps = 1e-6;
    const double eps2 = kEps * kEps;

    auto isClose = [eps2](const Vec2& a, const Vec2& b) {
        return geometry::detail::distanceSquared(a, b) <= eps2;
    };

    for (const auto& e : entities) {
        if (const auto* p = std::get_if<Point2>(&e.geometry)) {
            if (isClose(anchor, p->position)) return true;
        }
        else if (const auto* l = std::get_if<Line2>(&e.geometry)) {
            auto closest = geometry::detail::closestPointOnSegment(anchor, *l);
            if (closest && isClose(anchor, *closest)) return true;
        }
        else if (const auto* c = std::get_if<Circle2>(&e.geometry)) {
            if (isClose(anchor, c->center)) return true;
            auto closest = geometry::detail::closestPointOnCircle(anchor, *c);
            if (closest && isClose(anchor, *closest)) return true;
        }
        else if (const auto* a = std::get_if<Arc2>(&e.geometry)) {
            if (isClose(anchor, a->center)) return true;
            auto closest = geometry::detail::closestPointOnArc(anchor, *a);
            if (closest && isClose(anchor, *closest)) return true;
        }
    }
    return false;
}

} // namespace

inline std::optional<SnapCandidate>
collectFromEntities(const SnapRequest& request,
                    std::span<const SketchEntity> entities,
                    double tolWorld,
                    SnapKind wantKind) {
    if (request.primitive != PrimitiveType::Line || request.drawState != DrawState::End) {
        return std::nullopt;
    }
    if (request.anchors.empty()) return std::nullopt;

    const Vec2 anchor = request.anchors[0].position;
    const Vec2 cursor = request.cursor;
    const Vec2 dir = geometry::detail::subtract(cursor, anchor);

    if (geometry::detail::lengthSquared(dir) <= geometry::EPSILON * geometry::EPSILON) {
        return std::nullopt;
    }

    if (isAnchorConnected(anchor, entities)) {
        return std::nullopt;
    }

    auto snappedDirOpt = geometry::relations::snapDirectionToAxis(dir, 15.0);
    if (!snappedDirOpt) return std::nullopt;

    const Vec2 snappedDir = *snappedDirOpt;
    SnapKind kind = (std::abs(snappedDir.y) < 1e-9) ? SnapKind::Horizontal : SnapKind::Vertical;
    if (kind != wantKind) return std::nullopt;

    const Vec2 snappedCursor = geometry::detail::add(anchor, snappedDir);

    const double d2 = geometry::detail::distanceSquared(snappedCursor, cursor);
    const double tol2 = tolWorld * tolWorld;
    if (d2 > tol2) {
        return std::nullopt;
    }

    SnapCandidate c;
    c.result.point = Point2{snappedCursor};
    c.result.type = kind;
    c.distanceSquared = d2;
    c.priority = snapPriority(kind, request.primitive, request.drawState);

    c.result.guideLines.push_back(Line2{anchor, snappedCursor});

    return c;
}

} // namespace snap::direction

#endif // OURPAINT_DIRECTIONCOLLECTORS_H