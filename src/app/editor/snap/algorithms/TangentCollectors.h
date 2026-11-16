#ifndef OURPAINT_TANGENTCOLLECTORS_H
#define OURPAINT_TANGENTCOLLECTORS_H

#include <variant>
#include <vector>

#include "ArcAlgorithms.h"
#include "CircleAlgorithms.h"
#include "GeometryMath.h"
#include "LineAlgorithms.h"
#include "SketchTypes.h"
#include "SnapSystem.h"
#include "SnapTypes.h"
#include "geometry/relations/PointCurveTangency.h"  // pointOnArcSweep

namespace snap {
struct SnapRequest;
}

namespace snap::tangent {
namespace {

using namespace core::sketch;

bool forbiddenWhole(const SnapRequest& req, EntityId id) {
    for (const auto& f : req.forbiddenObjects)
        if (f.entity == id && f.sub == SubElement::Whole) return true;
    return false;
}

bool isEndpointOf(const SketchEntity& e, const Vec2& p) {
    constexpr double kEps = 1e-9;
    const double eps2 = kEps * kEps;
    if (const auto* l = std::get_if<Line2>(&e.geometry)) {
        return geometry::detail::distanceSquared(l->start, p) <= eps2
            || geometry::detail::distanceSquared(l->end,   p) <= eps2;
    }
    if (const auto* a = std::get_if<Arc2>(&e.geometry)) {
        return geometry::detail::distanceSquared(a->start, p) <= eps2
            || geometry::detail::distanceSquared(a->end,   p) <= eps2;
    }
    return false;
}

}  // namespace

inline std::optional<SnapCandidate>
collectFromEntities(const SnapRequest& request,
                    std::span<const SketchEntity> entities,
                    double tolWorld) {
    const bool isCircleBody = request.primitive == PrimitiveType::Circle &&
                              request.drawState == DrawState::Body;
    const bool isArcBody    = request.primitive == PrimitiveType::Arc &&
                              request.drawState == DrawState::Body;
    const bool isLineEnd    = request.primitive == PrimitiveType::Line &&
                              request.drawState == DrawState::End &&
                              !request.anchors.empty();

    if (!isCircleBody && !isArcBody && !isLineEnd) return std::nullopt;

    const double tol2 = tolWorld * tolWorld;
    const int prio = snapPriority(SnapKind::Tangent,
                                  request.primitive, request.drawState);

    std::optional<SnapCandidate> best;

    if (isLineEnd) {
        const Vec2 anchor{request.anchors[0].position.x,
                          request.anchors[0].position.y};
        const Vec2 cursor = request.cursor;
        const Vec2 dir = geometry::detail::subtract(cursor, anchor);
        const double len2 = geometry::detail::lengthSquared(dir);
        if (len2 <= geometry::EPSILON * geometry::EPSILON) return best;

        for (const auto& e : entities) {
            if (forbiddenWhole(request, e.id)) continue;

            std::optional<Vec2> center;
            double r = 0.0;
            const Arc2* arcPtr = nullptr;

            if (const auto* c = std::get_if<Circle2>(&e.geometry)) {
                center = c->center;
                r = c->radius;
            } else if (const auto* a = std::get_if<Arc2>(&e.geometry)) {
                center = a->center;
                r = std::sqrt(geometry::detail::distanceSquared(a->center, a->start));
                arcPtr = a;
            } else {
                continue;
            }
            if (r <= geometry::EPSILON) continue;

            const double t = geometry::detail::dot(
                geometry::detail::subtract(*center, anchor), dir) / len2;
            if (t <= 0.0 || t >= 1.0) continue;

            const Vec2 proj = geometry::detail::add(
                anchor, geometry::detail::multiply(dir, t));
            const double dProj = std::sqrt(
                geometry::detail::distanceSquared(proj, *center));
            if (dProj <= geometry::EPSILON) continue;

            if (std::abs(dProj - r) > tolWorld) continue;

            const Vec2 tangentPoint = geometry::detail::add(
                *center,
                geometry::detail::multiply(
                    geometry::detail::subtract(proj, *center), r / dProj));

            if (geometry::detail::distanceSquared(tangentPoint, anchor)
                    <= geometry::EPSILON * geometry::EPSILON) continue;
            if (isEndpointOf(e, tangentPoint)) continue;
            if (arcPtr && !geometry::relations::pointOnArcSweep(tangentPoint, *arcPtr))
                continue;

            const double d2 = geometry::detail::distanceSquared(tangentPoint, cursor);

            if (best) {
                if (prio <  best->priority) continue;
                if (prio == best->priority && d2 >= best->distanceSquared) continue;
            }

            SnapCandidate c;
            c.result.point = Point2{tangentPoint};
            c.result.type  = SnapKind::Tangent;
            c.result.objects.push_back({e.id, SubElement::Whole});
            c.distanceSquared = d2;
            c.priority = prio;
            best = std::move(c);
        }
        return best;
    }

    for (const auto& e : entities) {
        if (forbiddenWhole(request, e.id)) continue;

        std::optional<Vec2> tp;
        if (const auto* c = std::get_if<Circle2>(&e.geometry)) {
            tp = geometry::detail::closestPointOnCircle(request.cursor, *c);
        } else if (const auto* a = std::get_if<Arc2>(&e.geometry)) {
            tp = geometry::detail::closestPointOnArc(request.cursor, *a);
        } else if (const auto* l = std::get_if<Line2>(&e.geometry)) {
            tp = geometry::detail::closestPointOnSegment(request.cursor, *l);
        } else {
            continue;
        }
        if (!tp) continue;

        if (isEndpointOf(e, *tp)) continue;

        const double d2 = geometry::detail::distanceSquared(*tp, request.cursor);
        if (d2 > tol2) continue;

        if (best) {
            if (prio <  best->priority) continue;
            if (prio == best->priority && d2 >= best->distanceSquared) continue;
        }

        SnapCandidate c;
        c.result.point = Point2{*tp};
        c.result.type  = SnapKind::Tangent;
        c.result.objects.push_back({e.id, SubElement::Whole});
        c.distanceSquared = d2;
        c.priority = prio;
        best = std::move(c);
    }
    return best;
}

}  // namespace snap::tangent

#endif  // OURPAINT_TANGENTCOLLECTORS_H