#ifndef OURPAINT_INTERSECTIONCOLLECTORS_H
#define OURPAINT_INTERSECTIONCOLLECTORS_H

#include <variant>
#include <vector>

#include "GeometryMath.h"
#include "SketchTypes.h"
#include "SnapSystem.h"
#include "SnapTypes.h"
#include "IntersectionDispatcher.h"

namespace snap {
struct SnapRequest;
}

namespace snap::intersection {
namespace {

using namespace core::sketch;

struct Hit {
    Vec2 point;
    std::vector<Line2> guides;
};

bool forbiddenWhole(const SnapRequest& req, EntityId id) {
    for (const auto& f : req.forbiddenObjects)
        if (f.entity == id && f.sub == SubElement::Whole) return true;
    return false;
}

bool isCurve(const SketchEntity& e) {
    return std::holds_alternative<Line2>(e.geometry)
        || std::holds_alternative<Circle2>(e.geometry)
        || std::holds_alternative<Arc2>(e.geometry);
}

double parameterAlong(const Line2& line, const Vec2& p) {
    const Vec2 d = geometry::detail::subtract(line.end, line.start);
    const double l2 = geometry::detail::lengthSquared(d);
    if (l2 <= geometry::EPSILON * geometry::EPSILON) return 0.0;
    return geometry::detail::dot(geometry::detail::subtract(p, line.start), d) / l2;
}

bool inSegment(double t) {
    return t >= 0.0 && t <= 1.0;
}

Line2 extensionGuide(const Line2& line, const Vec2& point) {
    const double ds = geometry::detail::distanceSquared(line.start, point);
    const double de = geometry::detail::distanceSquared(line.end, point);
    return ds <= de ? Line2{line.start, point} : Line2{line.end, point};
}

std::vector<Hit> classifyPair(const SketchEntity& a,
                              const SketchEntity& b,
                              SnapKind wantKind) {
    std::vector<Hit> hits;

    const auto* la = std::get_if<Line2>(&a.geometry);
    const auto* lb = std::get_if<Line2>(&b.geometry);

    if (wantKind == SnapKind::Intersection) {
        const auto pts = geometry::intersections::intersectGeometries(a, b);
        for (const auto& p : pts) {
            Hit h;
            h.point = Vec2{p.position};
            hits.push_back(std::move(h));
        }
        return hits;
    }

    if (!la && !lb) return hits;

    const auto infinitePts = geometry::intersections::intersectGeometries(a, b, {false});
    for (const auto& p : infinitePts) {
        const Vec2 pt = Vec2{p.position};

        bool inA = true;
        bool inB = true;
        if (la) inA = inSegment(parameterAlong(*la, pt));
        if (lb) inB = inSegment(parameterAlong(*lb, pt));

        if (inA && inB) continue;

        SnapKind got;
        if (!inA && !inB && la && lb) got = SnapKind::VirtualIntersection;
        else                          got = SnapKind::ExtendedIntersection;

        if (got != wantKind) continue;

        Hit h;
        h.point = pt;
        if (got == SnapKind::ExtendedIntersection) {
            if (!inA && la) h.guides.push_back(extensionGuide(*la, pt));
            else if (!inB && lb) h.guides.push_back(extensionGuide(*lb, pt));
        } else {  // VirtualIntersection
            if (la) h.guides.push_back(extensionGuide(*la, pt));
            if (lb) h.guides.push_back(extensionGuide(*lb, pt));
        }
        hits.push_back(std::move(h));
    }
    return hits;
}

}  // namespace

inline std::optional<SnapCandidate>
collectFromEntities(const SnapRequest& request,
                    std::span<const SketchEntity> entities,
                    double tolWorld,
                    SnapKind wantKind) {
    if (request.drawState == DrawState::Body &&
        wantKind != SnapKind::VirtualIntersection) {
        return std::nullopt;
    }

    const double tol2 = tolWorld * tolWorld;
    const int prio = snapPriority(wantKind, request.primitive, request.drawState);

    std::optional<SnapCandidate> best;

    for (std::size_t i = 0; i < entities.size(); ++i) {
        const auto& a = entities[i];
        if (!isCurve(a)) continue;
        if (forbiddenWhole(request, a.id)) continue;

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            const auto& b = entities[j];
            if (!isCurve(b)) continue;
            if (forbiddenWhole(request, b.id)) continue;

            const auto hits = classifyPair(a, b, wantKind);
            for (const auto& hit : hits) {
                const double d2 = geometry::detail::distanceSquared(
                                      hit.point, request.cursor);
                if (d2 > tol2) continue;

                if (best) {
                    if (prio <  best->priority) continue;
                    if (prio == best->priority && d2 >= best->distanceSquared) continue;
                }

                SnapCandidate c;
                c.result.point = Point2{hit.point};
                c.result.type  = wantKind;
                c.result.objects.push_back({a.id, SubElement::Whole});
                c.result.objects.push_back({b.id, SubElement::Whole});
                c.result.guideLines = hit.guides;
                c.distanceSquared = d2;
                c.priority = prio;

                best = std::move(c);
            }
        }
    }
    return best;
}

}  // namespace snap::intersection

#endif  // OURPAINT_INTERSECTIONCOLLECTORS_H