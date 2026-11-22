#pragma once

#include <cmath>
#include <optional>

#include "geometry/GeometryTypes.h"

namespace geometry::relations {

inline constexpr double kDefaultAxisToleranceDegrees = 15.0;
inline constexpr double kDirectionEpsilon = 1e-12;

// Direction is considered horizontal/vertical regardless of its sign.
// A zero-length vector has no direction and returns false.
inline bool isNearHorizontal(const Vec2& direction, double toleranceDegrees = kDefaultAxisToleranceDegrees) noexcept {
    const double length = std::hypot(direction.x, direction.y);
    if (length <= kDirectionEpsilon || toleranceDegrees < 0.0 || toleranceDegrees > 90.0) {
        return false;
    }
    constexpr double kPi = 3.14159265358979323846;
    return std::abs(direction.y) / length <= std::sin(toleranceDegrees * kPi / 180.0);
}

inline bool isNearVertical(const Vec2& direction, double toleranceDegrees = kDefaultAxisToleranceDegrees) noexcept {
    const double length = std::hypot(direction.x, direction.y);
    if (length <= kDirectionEpsilon || toleranceDegrees < 0.0 || toleranceDegrees > 90.0) {
        return false;
    }
    constexpr double kPi = 3.14159265358979323846;
    return std::abs(direction.x) / length <= std::sin(toleranceDegrees * kPi / 180.0);
}

// If direction is within tolerance of an axis, return a direction with the
// same length, snapped exactly to that axis. Otherwise return std::nullopt.
// If it is equally close to both axes (possible only with a very large
// tolerance), horizontal wins deterministically.
inline std::optional<Vec2> snapDirectionToAxis(const Vec2& direction, double toleranceDegrees = kDefaultAxisToleranceDegrees) noexcept {
    const double length = std::hypot(direction.x, direction.y);
    if (length <= kDirectionEpsilon || toleranceDegrees < 0.0 || toleranceDegrees > 90.0) {
        return std::nullopt;
    }

    const bool horizontal = isNearHorizontal(direction, toleranceDegrees);
    const bool vertical = isNearVertical(direction, toleranceDegrees);
    if (!horizontal && !vertical) {
        return std::nullopt;
    }

    if (horizontal && (!vertical || std::abs(direction.y) <= std::abs(direction.x))) {
        return Vec2{direction.x < 0.0 ? -length : length, 0.0};
    }
    return Vec2{0.0, direction.y < 0.0 ? -length : length};
}

}  // namespace geometry::relations
