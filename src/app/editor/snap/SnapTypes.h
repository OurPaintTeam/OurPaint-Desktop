#ifndef OURPAINT_SNAPTYPES_H_
#define OURPAINT_SNAPTYPES_H_

#include <cstdint>

namespace snap {

enum class ActionType {
    None,
    Draw,
    Move,
};

// User action. Geometry placement stage is described separately by DrawState.
enum class PrimitiveType {
    None,
    Point,
    Line,
    Circle,
    Arc,
};

// Current part of the geometry being placed/edited.
enum class DrawState {
    None,
    Center, // circle / arc center
    Body,   // circle / arc body
    Start,  // line / arc start; point placement
    End     // line / arc end
};

enum class SnapKind : std::uint8_t {
    None,
    Coincident,
    Midpoint,
    Collinear,
    Intersection,
    ExtendedIntersection,
    VirtualIntersection,
    Tangent,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    AngleToLine,
};

// Bit index of SnapMask equals the SnapKind ordinal (see static_assert below).
enum class SnapMask : std::uint32_t {
    Coincident           = 1u << 0,
    Midpoint             = 1u << 1,
    Collinear            = 1u << 2,
    Intersection         = 1u << 3,
    ExtendedIntersection = 1u << 4,
    VirtualIntersection  = 1u << 5,
    Tangent              = 1u << 6,
    Horizontal           = 1u << 7,
    Vertical             = 1u << 8,
    Parallel             = 1u << 9,
    Perpendicular        = 1u << 10,
    AngleToLine          = 1u << 11,
    None                 = 1u << 12,
};

inline constexpr SnapKind kAllSnapKinds[] = {
    SnapKind::Coincident,
    SnapKind::Midpoint,
    SnapKind::Collinear,
    SnapKind::Intersection,
    SnapKind::ExtendedIntersection,
    SnapKind::VirtualIntersection,
    SnapKind::Tangent,
    SnapKind::Horizontal,
    SnapKind::Vertical,
    SnapKind::Parallel,
    SnapKind::Perpendicular,
    SnapKind::AngleToLine,
};

constexpr SnapMask toMask(SnapKind kind) noexcept {
    return static_cast<SnapMask>(1u << static_cast<std::uint32_t>(kind));
}

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

constexpr bool hasSnap(SnapMask mask, SnapMask flag) noexcept {
    return (mask & flag) != SnapMask::None;
}
constexpr bool hasSnap(SnapMask mask, SnapKind kind) noexcept {
    return hasSnap(mask, toMask(kind));
}

// Concrete-point snaps. Tangent is included here because it resolves to a point.
constexpr SnapMask IntersectionSnaps =
    SnapMask::Intersection | SnapMask::ExtendedIntersection | SnapMask::VirtualIntersection;

constexpr SnapMask PointSnaps =
    SnapMask::Coincident | SnapMask::Midpoint | IntersectionSnaps;

constexpr SnapMask Direction =
    SnapMask::Horizontal | SnapMask::Vertical |
    SnapMask::Parallel   | SnapMask::Perpendicular |
    SnapMask::AngleToLine | SnapMask::Collinear;

constexpr SnapMask LineEndSnaps   = PointSnaps | Direction | SnapMask::Tangent | SnapMask::Collinear;
constexpr SnapMask LineStartSnaps   = PointSnaps | Direction  | SnapMask::Collinear;
constexpr SnapMask CurveBodySnaps = SnapMask::Coincident | SnapMask::Tangent | IntersectionSnaps;
constexpr SnapMask All            = PointSnaps | Direction | SnapMask::Tangent;

// Centralized primitive/state -> allowed snap kinds mapping.
constexpr SnapMask snapMaskFor(PrimitiveType primitive, DrawState state) noexcept {
    using M = SnapMask;
    switch (primitive) {
        case PrimitiveType::Point:
            return state == DrawState::Start ? PointSnaps : M::None;

        case PrimitiveType::Line:
            if (state == DrawState::Start) return LineStartSnaps;
            if (state == DrawState::End)   return LineEndSnaps;
            return M::None;

        case PrimitiveType::Circle:
            if (state == DrawState::Center) return PointSnaps;
            if (state == DrawState::Body)   return CurveBodySnaps;
            return M::None;

        case PrimitiveType::Arc:
            if (state == DrawState::Center ||
                state == DrawState::Start  ||
                state == DrawState::End)    return PointSnaps;
            if (state == DrawState::Body)   return CurveBodySnaps;
            return M::None;
        case PrimitiveType::None:
            break;
    }
    return M::None;
}

constexpr int kBodyCoincidentPenalty = 350;

constexpr int snapPriority(SnapKind kind, PrimitiveType primitive, DrawState state) noexcept {
    const bool lineEnd = primitive == PrimitiveType::Line && state == DrawState::End;
    switch (kind) {
        case SnapKind::Midpoint:           return 1000;
        case SnapKind::ExtendedIntersection: return 900;
        case SnapKind::VirtualIntersection:  return 900;
        case SnapKind::Intersection:              return 900;
        case SnapKind::Coincident:         return 800;
        case SnapKind::Tangent:             return 700;
        case SnapKind::Parallel:             return lineEnd ? 650 : 300;
        case SnapKind::Perpendicular:        return lineEnd ? 650 : 300;
        case SnapKind::Horizontal:
        case SnapKind::Vertical:             return lineEnd ? 600 : 250;
        case SnapKind::Collinear:            return lineEnd ? 275 : 225;
        case SnapKind::AngleToLine:          return 0;
        case SnapKind::None:
            break;
    }
    return 0;
}

inline constexpr double kDefaultSnapTolerancePixels = 10.0;

constexpr double snapTolerancePixels(PrimitiveType primitive, DrawState state) noexcept {
    switch (primitive) {
        case PrimitiveType::Point:
            return state == DrawState::Start ? 10.0 : kDefaultSnapTolerancePixels;

        case PrimitiveType::Line:
            if (state == DrawState::Start) return 10.0;
            if (state == DrawState::End)   return 10.0;
            return kDefaultSnapTolerancePixels;

        case PrimitiveType::Circle:
            if (state == DrawState::Center) return 10.0;
            if (state == DrawState::Body)   return 10.0;
            return kDefaultSnapTolerancePixels;

        case PrimitiveType::Arc:
            if (state == DrawState::Center ||
                state == DrawState::Start  ||
                state == DrawState::End)    return 10.0;
            if (state == DrawState::Body)   return 10.0;
            return kDefaultSnapTolerancePixels;

        case PrimitiveType::None:
            return 0.0;
    }
    return kDefaultSnapTolerancePixels;
}

} // namespace snap
#endif // OURPAINT_SNAPTYPES_H_