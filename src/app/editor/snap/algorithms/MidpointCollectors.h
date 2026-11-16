#ifndef OURPAINT_MIDPOINTCOLLECTORS_H
#define OURPAINT_MIDPOINTCOLLECTORS_H

#include <variant>

#include "GeometryMath.h"
#include "SketchTypes.h"
#include "SnapSystem.h"
#include "SnapTypes.h"

namespace snap {
struct SnapRequest;
}

namespace snap::midpoint {
namespace {

using namespace core::sketch;

bool forbiddenSub(const SnapRequest& req, EntityId id, SubElement sub) {
    for (const auto& f : req.forbiddenObjects)
        if (f.entity == id && f.sub == sub) return true;
    return false;
}

bool bodySnapAllowed(const SnapRequest& req) noexcept {
    return req.drawState != DrawState::Body;
}

}  // namespace

inline std::optional<SnapCandidate>
collectFromLines(const SnapRequest& request,
                 std::span<const SketchEntity> lines,
                 double tolWorld) {
    if (!bodySnapAllowed(request)) {
        return std::nullopt;
    }
    const double tol2 = tolWorld * tolWorld;
    const int prio = snapPriority(SnapKind::Midpoint,
                                  request.primitive, request.drawState);

    std::optional<SnapCandidate> best;
    for (const auto& e : lines) {
        const auto* line = std::get_if<Line2>(&e.geometry);
        if (!line) continue;

        if (forbiddenSub(request, e.id, SubElement::Whole)) {
            continue;
        }

        const Vec2 mid = geometry::detail::midpoint(*line);
        const double d2 = geometry::detail::distanceSquared(mid, request.cursor);

        if (d2 > tol2) continue;

        if (best) {
            if (prio <  best->priority) continue;
            if (prio == best->priority && d2 >= best->distanceSquared) continue;
        }

        SnapCandidate c;
        c.result.point = Point2{mid};
        c.result.type  = SnapKind::Midpoint;
        c.result.objects.push_back({e.id, SubElement::Whole});
        c.distanceSquared = d2;
        c.priority = prio;

        best = std::move(c);
    }
    return best;
}

}  // namespace snap::midpoint

#endif  // OURPAINT_MIDPOINTCOLLECTORS_H