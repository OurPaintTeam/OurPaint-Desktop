#pragma once

#include <cmath>

#include "geometry/GeometryTypes.h"

namespace geometry::relations {

// True when two non-zero direction vectors meet at approximately 90 degrees.
// angularToleranceRadians is an angular tolerance, not a distance tolerance.
inline bool arePerpendicular(const Vec2& a, const Vec2& b, double angularToleranceRadians = 1e-6) noexcept {
    const double aLength = std::hypot(a.x, a.y);
    const double bLength = std::hypot(b.x, b.y);
    if (aLength <= 1e-12 || bLength <= 1e-12) {
        return false;
    }

    const double normalizedDot = std::abs(a.x * b.x + a.y * b.y) / (aLength * bLength);
    return normalizedDot <= std::sin(angularToleranceRadians);
}

// Convenience overload for a tolerance specified in degrees.
inline bool arePerpendicularDegrees(const Vec2& a, const Vec2& b, double angularToleranceDegrees = 0.1) noexcept {
    constexpr double kPi = 3.14159265358979323846;
    return arePerpendicular(a, b, angularToleranceDegrees * kPi / 180.0);
}

}  // namespace geometry::relations
