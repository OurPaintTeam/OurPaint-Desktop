#ifndef OURPAINT_COINCIDENTCOLLECTORS_H
#define OURPAINT_COINCIDENTCOLLECTORS_H

#include <variant>

#include "ArcAlgorithms.h"     // closestPointOnArc
#include "CircleAlgorithms.h"  // closestPointOnCircle
#include "GeometryMath.h"      // distanceSquared
#include "SketchTypes.h"
#include "SnapSystem.h"
#include "SnapTypes.h"

namespace snap {
struct SnapRequest;
}

namespace snap::coincident {
namespace {

using namespace core::sketch;



int coincidentPriority(const SnapRequest& req, bool isBody) noexcept {
    const int base = snapPriority(SnapKind::Coincident, req.primitive, req.drawState);
    return isBody ? base - kBodyCoincidentPenalty : base;
}

bool forbiddenWhole(const SnapRequest& req, EntityId id) {
    for (const auto& f : req.forbiddenObjects)
        if (f.entity == id && f.sub == SubElement::Whole) return true;
    return false;
}
bool forbiddenSub(const SnapRequest& req, EntityId id, SubElement sub) {
    for (const auto& f : req.forbiddenObjects)
        if (f.entity == id && f.sub == sub) return true;
    return false;
}

SnapCandidate makeCandidate(const Vec2& p, GeometryRef ref, double d2, int priority) {
    SnapResult r;
    r.point = Point2{p};
    r.type  = SnapKind::Coincident;
    r.objects.push_back(std::move(ref));
    return SnapCandidate{std::move(r), d2, priority};
}

void tryCandidate(std::optional<SnapCandidate>& best,
                  const Vec2& p, GeometryRef ref,
                  const Vec2& cursor, double tol2, int priority) {
    const double d2 = geometry::detail::distanceSquared(p, cursor);
    if (d2 > tol2) return;

    if (best) {
        if (priority <  best->priority) return;
        if (priority == best->priority && d2 >= best->distanceSquared) return;
    }
    best = makeCandidate(p, std::move(ref), d2, priority);
}

bool bodySnapAllowed(const SnapRequest& req) noexcept {
    return req.drawState != DrawState::Body;
}

}  // namespace

inline std::optional<SnapCandidate>
collectFromPoints(const SnapRequest& request,
                  std::span<const SketchEntity> points,
                  double tolWorld) {
    const double tol2 = tolWorld * tolWorld;
    const int    prio = coincidentPriority(request, /*isBody=*/false);

    std::optional<SnapCandidate> best;
    for (const auto& e : points) {
        if (forbiddenWhole(request, e.id)) continue;
        const auto* pt = std::get_if<Point2>(&e.geometry);
        if (!pt) continue;
        tryCandidate(best, pt->position, {e.id, SubElement::Whole},
                     request.cursor, tol2, prio);
    }
    return best;
}

inline std::optional<SnapCandidate>
collectFromLines(const SnapRequest& request,
                 std::span<const SketchEntity> lines,
                 double tolWorld) {
    const double tol2  = tolWorld * tolWorld;
    const int    pPrio = coincidentPriority(request, /*isBody=*/false);
    const int    bPrio = coincidentPriority(request, /*isBody=*/true);

    std::optional<SnapCandidate> best;
    for (const auto& e : lines) {
        const auto* line = std::get_if<Line2>(&e.geometry);
        if (!line) continue;

        if (!forbiddenSub(request, e.id, SubElement::Start))
            tryCandidate(best, line->start, {e.id, SubElement::Start},
                         request.cursor, tol2, pPrio);
        if (!forbiddenSub(request, e.id, SubElement::End))
            tryCandidate(best, line->end,   {e.id, SubElement::End},
                         request.cursor, tol2, pPrio);

          if (bodySnapAllowed(request) && !forbiddenSub(request, e.id, SubElement::Whole)) {
            const Vec2 d{{line->end.x - line->start.x},
                         {line->end.y - line->start.y}};

            const double l2 = d.x * d.x + d.y * d.y;
            double t = (l2 > 0.0)
                ? ((request.cursor.x - line->start.x) * d.x +
                   (request.cursor.y - line->start.y) * d.y) / l2
                : 0.0;
            t = std::clamp(t, 0.0, 1.0);

            const Vec2 p{{line->start.x + d.x * t},
                         {line->start.y + d.y * t}};

            tryCandidate(best, p, {e.id, SubElement::Whole},
                         request.cursor, tol2, bPrio);
        }
    }
    return best;
}

inline std::optional<SnapCandidate>
collectFromCircles(const SnapRequest& request,
                   std::span<const SketchEntity> circles,
                   double tolWorld) {
    const double tol2  = tolWorld * tolWorld;
    const int    pPrio = coincidentPriority(request, false);
    const int    bPrio = coincidentPriority(request, true);

    std::optional<SnapCandidate> best;
    for (const auto& e : circles) {
        const auto* circle = std::get_if<Circle2>(&e.geometry);
        if (!circle) continue;

        if (!forbiddenSub(request, e.id, SubElement::Center))
            tryCandidate(best, circle->center, {e.id, SubElement::Center},
                         request.cursor, tol2, pPrio);

        if (bodySnapAllowed(request)
            && !forbiddenSub(request, e.id, SubElement::Whole))
            if (auto p = geometry::detail::closestPointOnCircle(request.cursor, *circle))
                tryCandidate(best, *p, {e.id, SubElement::Whole},
                             request.cursor, tol2, bPrio);
    }
    return best;
}

inline std::optional<SnapCandidate>
collectFromArcs(const SnapRequest& request,
                std::span<const SketchEntity> arcs,
                double tolWorld) {
    const double tol2  = tolWorld * tolWorld;
    const int    pPrio = coincidentPriority(request, false);
    const int    bPrio = coincidentPriority(request, true);

    std::optional<SnapCandidate> best;
    for (const auto& e : arcs) {
        const auto* arc = std::get_if<Arc2>(&e.geometry);
        if (!arc) continue;

        if (!forbiddenSub(request, e.id, SubElement::Center))
            tryCandidate(best, arc->center, {e.id, SubElement::Center}, request.cursor, tol2, pPrio);
        if (!forbiddenSub(request, e.id, SubElement::Start))
            tryCandidate(best, arc->start,  {e.id, SubElement::Start},  request.cursor, tol2, pPrio);
        if (!forbiddenSub(request, e.id, SubElement::End))
            tryCandidate(best, arc->end,    {e.id, SubElement::End},    request.cursor, tol2, pPrio);

        if (bodySnapAllowed(request)
            && !forbiddenSub(request, e.id, SubElement::Whole))
            if (auto p = geometry::detail::closestPointOnArc(request.cursor, *arc))
                tryCandidate(best, *p, {e.id, SubElement::Whole}, request.cursor, tol2, bPrio);
    }
    return best;
}

}  // namespace snap::coincident

#endif  // OURPAINT_COINCIDENTCOLLECTORS_H
