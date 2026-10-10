#pragma once

#include <cstdint>

namespace snap {

// Активный инструмент редактора.
enum class ToolType {
    Select,

    Point,
    Line,
    Circle,
    Arc,

    Move,
    Rotate,
    Scale
};

// Тип привязки.
enum class SnapKind {
    None,

    StandalonePoint,

    LineEndpoint,
    LineMidpoint,
    LineBody,

    CircleBody,
    CircleCentre,

    Intersection,
    ExtendedIntersection,

    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Angle
};

// Состояние отрисовки инструмента.
enum class DrawState {
    Idle,        // Ожидание первой точки
    FirstPoint,  // Задана первая точка
    SecondPoint  // Задана вторая точка
};

// Маска доступных привязок.
enum class SnapMask : std::uint32_t {
    None            = 0,

    StandalonePoint = 1u << 0,
    LineEndpoint    = 1u << 1,
    LineMidpoint    = 1u << 2,
    LineBody        = 1u << 3,

    CircleBody      = 1u << 4,
    CircleCentre    = 1u << 5,

    Intersection         = 1u << 6,
    ExtendedIntersection = 1u << 7,

    Horizontal    = 1u << 8,
    Vertical      = 1u << 9,
    Parallel      = 1u << 10,
    Perpendicular = 1u << 11,
    Angle         = 1u << 12
};

// Операторы для SnapMask.
constexpr SnapMask operator|(SnapMask a, SnapMask b) noexcept {
    return static_cast<SnapMask>(
        static_cast<std::uint32_t>(a) |
        static_cast<std::uint32_t>(b)
    );
}

constexpr SnapMask operator&(SnapMask a, SnapMask b) noexcept {
    return static_cast<SnapMask>(
        static_cast<std::uint32_t>(a) &
        static_cast<std::uint32_t>(b)
    );
}

constexpr SnapMask& operator|=(SnapMask& a, SnapMask b) noexcept {
    a = a | b;
    return a;
}

constexpr SnapMask& operator&=(SnapMask& a, SnapMask b) noexcept {
    a = a & b;
    return a;
}

// Проверяет, включён ли конкретный тип привязки.
constexpr bool hasSnap(SnapMask mask, SnapMask flag) noexcept {
    return (mask & flag) != SnapMask::None;
}

// Все позиционные привязки.
constexpr SnapMask Position =
    SnapMask::StandalonePoint |
    SnapMask::LineEndpoint |
    SnapMask::LineMidpoint |
    SnapMask::LineBody |
    SnapMask::CircleBody |
    SnapMask::CircleCentre |
    SnapMask::Intersection |
    SnapMask::ExtendedIntersection;

constexpr SnapMask Direction =
    SnapMask::Horizontal |
    SnapMask::Vertical |
    SnapMask::Parallel |
    SnapMask::Perpendicular |
    SnapMask::Angle;

constexpr SnapMask All = Position | Direction;

// Привязки по умолчанию для инструмента.
constexpr SnapMask defaultSnapMask(ToolType tool) noexcept {
    using M = SnapMask;

    switch (tool) {
        case ToolType::Select:
        case ToolType::Move:
            return M::StandalonePoint
                | M::LineEndpoint
                | M::LineMidpoint
                | M::LineBody
                | M::CircleBody
                | M::CircleCentre
                | M::Intersection;

        case ToolType::Point:
        case ToolType::Line:
        case ToolType::Circle:
        case ToolType::Arc:
            return Position;

        case ToolType::Rotate:
        case ToolType::Scale:
            return M::StandalonePoint
                | M::LineEndpoint
                | M::CircleCentre
                | M::Intersection;

        default:
            return M::None;
    }
}

} // namespace snap
