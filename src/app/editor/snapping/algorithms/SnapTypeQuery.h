#ifndef OURPAINT_SNAP_TYPE_QUERY_H
#define OURPAINT_SNAP_TYPE_QUERY_H
#include "snapping/SnapTypes.h"
namespace snap {
constexpr SnapMask maskForSnapKind(SnapKind kind) noexcept {
    switch (kind) {
        case SnapKind::Coincident: return SnapMask::Coincident;
        case SnapKind::Midpoint: return SnapMask::Midpoint;
        case SnapKind::Collinear: return SnapMask::Collinear;
        case SnapKind::Alignment: return SnapMask::Alignment;
        case SnapKind::EqualityEndLines: return SnapMask::EqualityEndLines;
        case SnapKind::Intersection: return SnapMask::Intersection;
        case SnapKind::ExtendedIntersection: return SnapMask::ExtendedIntersection;
        case SnapKind::VirtualIntersection: return SnapMask::VirtualIntersection;
        case SnapKind::Tangent: return SnapMask::Tangent;
        case SnapKind::Horizontal: return SnapMask::Horizontal;
        case SnapKind::Vertical: return SnapMask::Vertical;
        case SnapKind::Parallel: return SnapMask::Parallel;
        case SnapKind::Perpendicular: return SnapMask::Perpendicular;
        case SnapKind::AngleToLine: return SnapMask::AngleToLine;
        case SnapKind::PerpendicularBisector: return SnapMask::PerpendicularBisector;
    }
    return SnapMask::None;
}
constexpr bool isDirectionalSnapKind(SnapKind kind) noexcept { return hasSnap(Direction, maskForSnapKind(kind)); }
}
#endif
