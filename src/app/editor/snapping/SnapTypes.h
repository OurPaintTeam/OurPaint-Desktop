#pragma once

#include <cstdint>

namespace snap {

// Active editor tool.
enum class ToolType {
    Point,
    Line,
    Circle,
    Arc,

    Move,
    Rotate,
    Scale
};

// =============================================================================
// SnapKind — type of a single snap candidate.
// Describes ONE candidate point or ONE direction, not a pair of objects.
// Multiple intersections from one pair become multiple candidates.
// =============================================================================
enum class SnapKind {
    /// No snap: candidate failed or mask disallows everything.
    None,

    /// Standalone point (Point object), not attached to a primitive.
    StandalonePoint,

    // --- Line ----------------------------------------------------------------

    /// End point of a line segment.
    LineEndpoint,
    /// Midpoint of a line segment.
    LineMidpoint,
    /// Point on a line segment: nearest projection of the cursor.
    /// In selection mode — the segment itself.
    LineBody,

    // --- Circle --------------------------------------------------------------

    /// Point on a circle: nearest projection of the cursor.
    /// In selection mode — the circle itself.
    CircleBody,
    /// Centre of a circle.
    CircleCentre,

    // --- Arc -----------------------------------------------------------------

    /// Point on an arc within its angular range.
    /// In selection mode — the arc itself.
    ArcBody,
    /// Centre of an arc (centre of its support circle).
    ArcCentre,
    /// End point of an arc (boundary of the angular range).
    ArcEndpoint,

    // --- Intersections -------------------------------------------------------

    /// Both points lie on real parts of the primitives.
    Intersection,
    /// One point is real, the other is on an extension.
    /// For an arc, extension = support circle outside its angular range.
    ExtendedIntersection,
    /// Both points lie on extensions (two virtual lines).
    VirtualIntersection,
    /// Tangency on real parts. Separate type: own icon and priority.
    Tangent,

    // --- Directional snaps ---------------------------------------------------
    // Require a basePoint (last committed point).
    // Without basePoint they are not activated.

    /// Horizontal from basePoint.
    Horizontal,
    /// Vertical from basePoint.
    Vertical,
    /// Parallel to a reference object through basePoint.
    Parallel,
    /// Perpendicular to a reference object through basePoint.
    Perpendicular,

    // --- Reserved (no algorithm yet) -----------------------------------------

    /// Angle. Not enabled in defaultSnapMask by default.
    Angle,
    /// Perpendicular bisector of a reference segment or arc chord.
    /// Not enabled in defaultSnapMask by default.
    PerpendicularBisector
};

// Tool drawing state.
enum class DrawState {
    Idle,        ///< Waiting for the first point.
    FirstPoint,  ///< First point committed.
    SecondPoint  ///< Second point committed.
};

// Bit mask of enabled snaps.
enum class SnapMask : std::uint32_t {
    None = 0,

    StandalonePoint = 1u << 0,
    LineEndpoint = 1u << 1,
    LineMidpoint = 1u << 2,
    LineBody = 1u << 3,

    CircleBody = 1u << 4,
    CircleCentre = 1u << 5,

    Intersection = 1u << 6,
    ExtendedIntersection = 1u << 7,

    Horizontal = 1u << 8,
    Vertical = 1u << 9,
    Parallel = 1u << 10,
    Perpendicular = 1u << 11,
    Angle = 1u << 12,

    ArcBody = 1u << 13,
    ArcCentre = 1u << 14,
    ArcEndpoint = 1u << 15,

    Tangent = 1u << 16,
    VirtualIntersection = 1u << 17,
    PerpendicularBisector = 1u << 18
};

// SnapMask operators.
constexpr SnapMask operator|(SnapMask a, SnapMask b) noexcept { return static_cast<SnapMask>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b)); }

constexpr SnapMask operator&(SnapMask a, SnapMask b) noexcept { return static_cast<SnapMask>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b)); }

constexpr SnapMask& operator|=(SnapMask& a, SnapMask b) noexcept {
    a = a | b;
    return a;
}

constexpr SnapMask& operator&=(SnapMask& a, SnapMask b) noexcept {
    a = a & b;
    return a;
}

// Checks whether a specific snap flag is enabled.
constexpr bool hasSnap(SnapMask mask, SnapMask flag) noexcept { return (mask & flag) != SnapMask::None; }

// All positional snaps (return a concrete point).
constexpr SnapMask Position = SnapMask::StandalonePoint | SnapMask::LineEndpoint | SnapMask::LineMidpoint | SnapMask::LineBody | SnapMask::CircleBody |
                              SnapMask::CircleCentre | SnapMask::ArcBody | SnapMask::ArcCentre | SnapMask::ArcEndpoint | SnapMask::Intersection |
                              SnapMask::ExtendedIntersection | SnapMask::VirtualIntersection | SnapMask::Tangent;

// All directional snaps (require a basePoint; no point on their own).
constexpr SnapMask Direction =
    SnapMask::Horizontal | SnapMask::Vertical | SnapMask::Parallel | SnapMask::Perpendicular | SnapMask::Angle | SnapMask::PerpendicularBisector;

constexpr SnapMask All = Position | Direction;

// Default snap mask for a tool.
constexpr SnapMask defaultSnapMask(ToolType tool) noexcept {
    using M = SnapMask;

    switch (tool) {
        case ToolType::Move:
            return M::StandalonePoint | M::LineEndpoint | M::LineBody | M::CircleBody | M::CircleCentre | M::ArcBody | M::ArcCentre | M::ArcEndpoint;

        case ToolType::Point:
        case ToolType::Line:
        case ToolType::Circle:
        case ToolType::Arc:
            return Position;

        case ToolType::Rotate:
        case ToolType::Scale:
            return M::StandalonePoint | M::LineEndpoint | M::CircleCentre | M::ArcCentre | M::Intersection;

        default:
            return M::None;
    }
}

// Priority of a snap kind for tie-breaking.
// Higher wins when two candidates have nearly equal distance to the cursor.
constexpr int snapPriority(SnapKind kind, ToolType tool, DrawState state) noexcept {
    // Directional snaps get boosted only when drawing a line-like tool
    // after the first point has been committed.
    const bool directionalBoost = (state == DrawState::FirstPoint) && (tool == ToolType::Line || tool == ToolType::Arc);

    switch (kind) {
        case SnapKind::None:
            return 0;

        case SnapKind::Intersection:
            return 1000;

        case SnapKind::VirtualIntersection:
            return 990;

        case SnapKind::ExtendedIntersection:
            return 980;

        case SnapKind::StandalonePoint:
        case SnapKind::LineEndpoint:
        case SnapKind::ArcEndpoint:
        case SnapKind::CircleCentre:
        case SnapKind::ArcCentre:
            return 850;

        case SnapKind::Tangent:
            return 800;
        case SnapKind::LineMidpoint:
            return 700;

        case SnapKind::Parallel:
            return directionalBoost ? 650 : 300;
        case SnapKind::Perpendicular:
            return directionalBoost ? 650 : 300;
        case SnapKind::Horizontal:
            return directionalBoost ? 600 : 250;
        case SnapKind::Vertical:
            return directionalBoost ? 600 : 250;
        case SnapKind::Angle:
            return 150;  // 550 при constraint — см. SnapSystem
        case SnapKind::PerpendicularBisector:
            return 150;

        case SnapKind::LineBody:
        case SnapKind::CircleBody:
        case SnapKind::ArcBody:
            return 500;
    }
    return 0;
}

}  // namespace snap