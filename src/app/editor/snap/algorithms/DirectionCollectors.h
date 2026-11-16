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
#include "Parallelism.h"
#include "Perpendicularity.h"
#include "LineAlgorithms.h"

namespace snap
{
    struct SnapRequest;
}

namespace snap::direction
{
    namespace
    {
        using namespace core::sketch;

        bool isAnchorOnLine(const Vec2& anchor, std::span<const SketchEntity> entities)
        {
            constexpr double kEps = 1e-5;
            const double eps2 = kEps * kEps;

            for (const auto& e : entities)
            {
                if (const auto* l = std::get_if<Line2>(&e.geometry))
                {
                    auto closest = geometry::detail::closestPointOnSegment(anchor, *l);
                    if (closest && geometry::detail::distanceSquared(*closest, anchor) <= eps2)
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        std::optional<Vec2> projectOntoAxis(const Vec2& anchor, const Vec2& cursor, const Vec2& axisDir,
                                            double tolWorld)
        {
            const double len2 = geometry::detail::lengthSquared(axisDir);
            if (len2 <= geometry::EPSILON) return std::nullopt;

            const double t = geometry::detail::dot(geometry::detail::subtract(cursor, anchor), axisDir) / len2;
            const Vec2 proj = geometry::detail::add(anchor, geometry::detail::multiply(axisDir, t));

            if (geometry::detail::distanceSquared(proj, cursor) > tolWorld * tolWorld)
            {
                return std::nullopt;
            }
            return proj;
        }
    } // namespace

    inline std::optional<SnapCandidate>
    collectFromEntities(const SnapRequest& request,
                        std::span<const SketchEntity> entities,
                        double tolWorld,
                        SnapKind wantKind)
    {
        if (request.primitive != PrimitiveType::Line || request.drawState != DrawState::End) return std::nullopt;
        if (request.anchors.empty()) return std::nullopt;

        const Vec2 anchor = request.anchors[0].position;
        const Vec2 cursor = request.cursor;
        const Vec2 propDir = geometry::detail::subtract(cursor, anchor);

        if (geometry::detail::lengthSquared(propDir) <= geometry::EPSILON * geometry::EPSILON) return std::nullopt;

        std::optional<SnapCandidate> best;

        // === 1. H / V ===
        if (wantKind == SnapKind::Horizontal || wantKind == SnapKind::Vertical)
        {
            // Запрещаем H/V ТОЛЬКО если первая точка лежит на линии
            if (isAnchorOnLine(anchor, entities)) return std::nullopt;

            auto snappedDirOpt = geometry::relations::snapDirectionToAxis(propDir, 15.0);
            if (!snappedDirOpt) return std::nullopt;

            const Vec2 snappedDir = *snappedDirOpt;
            SnapKind kind = (std::abs(snappedDir.y) < 1e-9) ? SnapKind::Horizontal : SnapKind::Vertical;
            if (kind != wantKind) return std::nullopt;

            const Vec2 snappedCursor = geometry::detail::add(anchor, snappedDir);
            const double d2 = geometry::detail::distanceSquared(snappedCursor, cursor);
            if (d2 > tolWorld * tolWorld) return std::nullopt;

            SnapCandidate c;
            c.result.point = Point2{snappedCursor};
            c.result.type = kind;
            c.distanceSquared = d2;
            c.priority = snapPriority(kind, request.primitive, request.drawState);
            c.result.guideLines.push_back(Line2{anchor, snappedCursor});
            return c;
        }

        // === 2. Parallel / Perpendicular ===
        if (wantKind == SnapKind::Parallel || wantKind == SnapKind::Perpendicular)
        {
            for (const auto& e : entities)
            {
                const auto* l = std::get_if<Line2>(&e.geometry);
                if (!l) continue;

                const Vec2 lineDir = geometry::detail::subtract(l->end, l->start);

                if (wantKind == SnapKind::Perpendicular)
                {
                    auto closestAnchor = geometry::detail::closestPointOnSegment(anchor, *l);
                    bool anchorOnLine = closestAnchor && geometry::detail::distanceSquared(*closestAnchor, anchor) <=
                        1e-4;

                    auto closestCursor = geometry::detail::closestPointOnSegment(cursor, *l);
                    bool cursorOnLine = closestCursor && geometry::detail::distanceSquared(*closestCursor, cursor) <=
                        tolWorld * tolWorld;

                    if (!anchorOnLine && !cursorOnLine) continue;

                    if (geometry::relations::arePerpendicularDegrees(lineDir, propDir, 5.0))
                    {
                        Vec2 perpDir{-lineDir.y, lineDir.x};
                        Vec2 finalPoint;
                        bool valid = false;

                        if (!anchorOnLine && cursorOnLine)
                        {
                            Line2 perpLine{anchor, {anchor.x + perpDir.x, anchor.y + perpDir.y}};
                            auto hit = geometry::detail::intersectLines(perpLine, *l);
                            if (hit)
                            {
                                finalPoint = hit->point;
                                valid = true;
                            }
                        }
                        else
                        {
                            auto proj = projectOntoAxis(anchor, cursor, perpDir, tolWorld);
                            if (proj)
                            {
                                finalPoint = *proj;
                                valid = true;
                            }
                        }

                        if (valid)
                        {
                            SnapCandidate c;
                            c.result.point = Point2{finalPoint};
                            c.result.type = wantKind;
                            c.result.objects.push_back({e.id, SubElement::Whole});
                            c.distanceSquared = geometry::detail::distanceSquared(finalPoint, cursor);
                            c.priority = snapPriority(wantKind, request.primitive, request.drawState);
                            c.result.guideLines.push_back(Line2{anchor, finalPoint});
                            if (!best || c.distanceSquared < best->distanceSquared) best = std::move(c);
                        }
                    }
                }
                else if (wantKind == SnapKind::Parallel)
                {
                    if (geometry::relations::areParallelDegrees(lineDir, propDir, 5.0))
                    {
                        auto proj = projectOntoAxis(anchor, cursor, lineDir, tolWorld);
                        if (proj)
                        {
                            SnapCandidate c;
                            c.result.point = Point2{*proj};
                            c.result.type = wantKind;
                            c.result.objects.push_back({e.id, SubElement::Whole});

                            auto closestToCursor = geometry::detail::closestPointOnSegment(cursor, *l);
                            c.distanceSquared = closestToCursor
                                                    ? geometry::detail::distanceSquared(*closestToCursor, cursor)
                                                    : 0.0;

                            c.priority = snapPriority(wantKind, request.primitive, request.drawState);
                            c.result.guideLines.push_back(Line2{anchor, *proj});

                            if (!best || c.distanceSquared < best->distanceSquared) best = std::move(c);
                        }
                    }
                }
            }
        }

        return best;
    }
} // namespace snap::direction

#endif // OURPAINT_DIRECTIONCOLLECTORS_H
