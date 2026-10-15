#ifndef OURPAINT_SNAP_TYPE_QUERY_H
#define OURPAINT_SNAP_TYPE_QUERY_H

#include "snapping/SnapTypes.h"

namespace snap {
// Single source of truth for mapping a resolved snap kind to its enabling mask.
constexpr SnapMask maskForSnapKind(SnapKind kind) noexcept {
    switch (kind) {
        case SnapKind::StandalonePoint:
            return SnapMask::StandalonePoint;
        case SnapKind::LineEndpoint:
            return SnapMask::LineEndpoint;
        case SnapKind::LineMidpoint:
            return SnapMask::LineMidpoint;
        case SnapKind::LineBody:
            return SnapMask::LineBody;
        case SnapKind::CircleBody:
            return SnapMask::CircleBody;
        case SnapKind::CircleCentre:
            return SnapMask::CircleCentre;
        case SnapKind::ArcBody:
            return SnapMask::ArcBody;
        case SnapKind::ArcCentre:
            return SnapMask::ArcCentre;
        case SnapKind::ArcEndpoint:
            return SnapMask::ArcEndpoint;
        case SnapKind::Intersection:
            return SnapMask::Intersection;
        case SnapKind::ExtendedIntersection:
            return SnapMask::ExtendedIntersection;
        case SnapKind::VirtualIntersection:
            return SnapMask::VirtualIntersection;
        case SnapKind::Tangent:
            return SnapMask::Tangent;
        case SnapKind::Horizontal:
            return SnapMask::Horizontal;
        case SnapKind::Vertical:
            return SnapMask::Vertical;
        case SnapKind::Parallel:
            return SnapMask::Parallel;
        case SnapKind::Perpendicular:
            return SnapMask::Perpendicular;
        case SnapKind::Angle:
            return SnapMask::Angle;
        case SnapKind::PerpendicularBisector:
            return SnapMask::PerpendicularBisector;
        case SnapKind::None:
        default:
            return SnapMask::None;
    }
}

constexpr bool isDirectionalSnapKind(SnapKind kind) noexcept { return hasSnap(Direction, maskForSnapKind(kind)); }
}  // namespace snap

#endif  // OURPAINT_SNAP_TYPE_QUERY_H
