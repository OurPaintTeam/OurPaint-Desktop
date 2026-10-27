#ifndef OURPAINT_SNAPTYPES_H_
#define OURPAINT_SNAPTYPES_H_

#include <cstdint>

namespace snap {

// User action. Geometry placement stage is described separately by DrawState.
enum class ActionType {
    Point,
    Line,
    Circle,
    Arc,
    Move,
    Rotate,
    Scale
};

// Current part of the geometry being placed/edited.
enum class DrawState {
    Center, // circle / arc center
    Body,   // circle / arc body
    Start,  // line / arc start; point placement
    End     // line / arc end
};

enum class SnapKind {
    Coincident,
    Midpoint,
    Collinear,
    Alignment, // horizontal/vertical alignment through a characteristic point
    EqualityEndLines, // deprecated legacy kind; do not emit from new algorithms
    Intersection,
    ExtendedIntersection,
    VirtualIntersection,
    Tangent,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    AngleToLine,
    PerpendicularBisector
};

enum class SnapMask : std::uint32_t {
    None = 0,
    Coincident = 1u << 0,
    Midpoint = 1u << 2,
    Collinear = 1u << 3,
    Alignment = 1u << 14,
    EqualityEndLines = 1u << 16,
    Intersection = 1u << 4,
    ExtendedIntersection = 1u << 5,
    VirtualIntersection = 1u << 6,
    Tangent = 1u << 7,
    Horizontal = 1u << 8,
    Vertical = 1u << 9,
    Parallel = 1u << 10,
    Perpendicular = 1u << 11,
    AngleToLine = 1u << 12,
    PerpendicularBisector = 1u << 13
};

constexpr SnapMask operator|(SnapMask a, SnapMask b) noexcept {
    return static_cast<SnapMask>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}
constexpr SnapMask operator&(SnapMask a, SnapMask b) noexcept {
    return static_cast<SnapMask>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}
constexpr SnapMask& operator|=(SnapMask& a, SnapMask b) noexcept { return a = a | b; }
constexpr SnapMask& operator&=(SnapMask& a, SnapMask b) noexcept { return a = a & b; }
constexpr SnapMask operator~(SnapMask a) noexcept {
    return static_cast<SnapMask>(~static_cast<std::uint32_t>(a));
}
constexpr bool hasSnap(SnapMask mask, SnapMask flag) noexcept { return (mask & flag) != SnapMask::None; }

// Concrete-point snaps. Tangent is included here because it resolves to a point.
constexpr SnapMask IntersectionSnaps =  SnapMask::Intersection |SnapMask::ExtendedIntersection | SnapMask::VirtualIntersection;
constexpr SnapMask PointSnaps = SnapMask::Coincident | SnapMask::Midpoint | IntersectionSnaps;
constexpr SnapMask BodySnaps = SnapMask::Coincident; // nearest point on an existing circle/arc body; no intersections or midpoint
constexpr SnapMask Direction = SnapMask::Horizontal | SnapMask::Vertical | SnapMask::Alignment | SnapMask::Parallel |
    SnapMask::Perpendicular | SnapMask::AngleToLine | SnapMask::PerpendicularBisector | SnapMask::Collinear;
constexpr SnapMask LineEndSnaps = PointSnaps | Direction | SnapMask::Tangent | SnapMask::EqualityEndLines;
constexpr SnapMask CurveBodySnaps = BodySnaps | SnapMask::Tangent | IntersectionSnaps;
constexpr SnapMask All = PointSnaps | Direction | SnapMask::Tangent;

// Centralized action/state -> allowed snap kinds mapping.
constexpr SnapMask snapMaskFor(ActionType action, DrawState state) noexcept {
    using M = SnapMask;
    switch (action) {
        case ActionType::Point:
            return state == DrawState::Start ? (PointSnaps | M::Collinear | M::Alignment) : M::None;
        case ActionType::Line:
            if (state == DrawState::End) return LineEndSnaps | M::Alignment;
            if (state == DrawState::Start) return PointSnaps | M::Collinear | M::Alignment;
            return M::None;
        case ActionType::Circle:
            if (state == DrawState::Center) return PointSnaps | M::Alignment;
            if (state == DrawState::Body) return CurveBodySnaps | M::Alignment;
            return M::None;
        case ActionType::Arc:
            if (state == DrawState::Center || state == DrawState::Start || state == DrawState::End) return PointSnaps | M::Alignment;
            if (state == DrawState::Body) return CurveBodySnaps | M::Alignment;
            return M::None;
        case ActionType::Move:
            return PointSnaps | M::Tangent;
        case ActionType::Rotate:
        case ActionType::Scale:
            return M::Coincident | M::Intersection;
    }
    return M::None;
}

constexpr int snapPriority(SnapKind kind, ActionType action, DrawState state) noexcept {
    const bool lineEnd = action == ActionType::Line && state == DrawState::End;
    switch (kind) {
        case SnapKind::Coincident: return 1000;
        case SnapKind::Intersection: return 950;
        case SnapKind::ExtendedIntersection: return 450;
        case SnapKind::VirtualIntersection: return 400;
        case SnapKind::Tangent: return 800;
        case SnapKind::Midpoint: return 700;
        case SnapKind::Collinear: return lineEnd ? 275 : 225;
        case SnapKind::Alignment: return lineEnd ? 350 : 300;
        case SnapKind::EqualityEndLines: return lineEnd ? 275 : 225;
        case SnapKind::Parallel: return lineEnd ? 650 : 300;
        case SnapKind::Perpendicular: return lineEnd ? 650 : 300;
        case SnapKind::Horizontal:
        case SnapKind::Vertical: return lineEnd ? 600 : 250;
        case SnapKind::AngleToLine:
        case SnapKind::PerpendicularBisector: return 150;
    }
    return 0;
}

} // namespace snap
#endif // OURPAINT_SNAPTYPES_H_