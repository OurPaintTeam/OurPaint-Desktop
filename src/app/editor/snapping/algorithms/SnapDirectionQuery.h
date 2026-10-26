#ifndef OURPAINT_SNAP_DIRECTION_QUERY_H
#define OURPAINT_SNAP_DIRECTION_QUERY_H

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <span>
#include <variant>
#include <vector>

#include "geometry/GeometryMath.h"

namespace geometry {

enum class DirectionSnapKind { AxisXParallel, AxisYParallel, AxisXPerpendicular, AxisYPerpendicular, ParallelToLine, PerpendicularToLine, AngleToLine };

struct DirectionSnapCandidate {
    Vec2 endpoint;  // Cursor projected onto the proposed direction.
    Vec2 unitDirection;
    double angularDeviationRadians = 0.0;
    double distanceSquared = 0.0;  // From raw cursor to projected endpoint.
    DirectionSnapKind kind = DirectionSnapKind::AxisXParallel;
    std::optional<GeometryRef> reference;
};

namespace detail_direction_snap {
inline double normalizeUndirectedAngle(double angle) {
    angle = std::fmod(angle, std::numbers::pi);
    if (angle < 0.0) {
        angle += std::numbers::pi;
    }
    return angle;
}
inline double angleDifference(double a, double b) {
    const double d = std::abs(normalizeUndirectedAngle(a) - normalizeUndirectedAngle(b));
    return std::min(d, std::numbers::pi - d);
}
inline Vec2 unit(const Vec2& v) {
    const double n = std::hypot(v.x, v.y);
    return n <= EPSILON ? Vec2{0.0, 0.0} : Vec2{v.x / n, v.y / n};
}
inline double angle(const Vec2& v) { return std::atan2(v.y, v.x); }
}  // namespace detail_direction_snap

// Finds direction suggestions from the line start toward the current cursor.
// Directions are accepted within toleranceDegrees (default +/-25 degrees).
// Line references are taken from existing Line2 entities; axes are always considered.
inline std::vector<DirectionSnapCandidate> findDirectionSnaps(const Vec2& start, const Vec2& cursor, std::span<const SketchEntity> entities,
                                                              double toleranceDegrees = 25.0, double angleStepDegrees = 15.0) {
    using namespace detail_direction_snap;
    std::vector<DirectionSnapCandidate> result;
    if (!detail::isFinite(start) || !detail::isFinite(cursor) || !std::isfinite(toleranceDegrees) || toleranceDegrees < 0.0 || toleranceDegrees > 90.0 || !std::isfinite(angleStepDegrees) || angleStepDegrees <= 0.0 || angleStepDegrees > 90.0) {
        return result;
    }
    const Vec2 raw{cursor.x - start.x, cursor.y - start.y};
    const double rawLength = std::hypot(raw.x, raw.y);
    if (rawLength <= EPSILON) {
        return result;
    }
    const double rawAngle = angle(raw);
    const double tolerance = toleranceDegrees * std::numbers::pi / 180.0;

    const auto add = [&](Vec2 direction, DirectionSnapKind kind, std::optional<GeometryRef> ref) {
        direction = unit(direction);
        if (std::hypot(direction.x, direction.y) <= EPSILON) {
            return;
        }
        const double targetAngle = angle(direction);
        const double deviation = angleDifference(rawAngle, targetAngle);
        if (deviation > tolerance + EPSILON) {
            return;
        }
        const double projection = raw.x * direction.x + raw.y * direction.y;
        const Vec2 endpoint{start.x + projection * direction.x, start.y + projection * direction.y};
        result.push_back({endpoint, direction, deviation, detail::distanceSquared(cursor, endpoint), kind, std::move(ref)});
    };

    // The X/Y axis constraints reduce to horizontal/vertical directions.
    add({1.0, 0.0}, DirectionSnapKind::AxisXParallel, std::nullopt);
    add({0.0, 1.0}, DirectionSnapKind::AxisYParallel, std::nullopt);
    add({0.0, 1.0}, DirectionSnapKind::AxisXPerpendicular, std::nullopt);
    add({1.0, 0.0}, DirectionSnapKind::AxisYPerpendicular, std::nullopt);

    for (const auto& entity : entities) {
        const auto* line = std::get_if<Line2>(&entity.geometry);
        if (!line || detail::isDegenerate(*line) || !detail::isFinite(*line)) {
            continue;
        }
        const Vec2 d{line->end.x - line->start.x, line->end.y - line->start.y};
        const GeometryRef ref{entity.id, SubElement::Whole};
        add(d, DirectionSnapKind::ParallelToLine, ref);
        add({-d.y, d.x}, DirectionSnapKind::PerpendicularToLine, ref);

        // Generate angular directions at a zoom-dependent increment relative to
        // each existing line. The SnapSystem filters topology and zoom-scaled range.
        const double baseAngle = angle(d);
        for (double degrees = angleStepDegrees; degrees < 180.0 - EPSILON; degrees += angleStepDegrees) {
            const double delta = degrees * std::numbers::pi / 180.0;
            add({std::cos(baseAngle + delta), std::sin(baseAngle + delta)}, DirectionSnapKind::AngleToLine, ref);
            add({std::cos(baseAngle - delta), std::sin(baseAngle - delta)}, DirectionSnapKind::AngleToLine, ref);
        }
    }

    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (std::abs(a.angularDeviationRadians - b.angularDeviationRadians) > 1e-12) {
            return a.angularDeviationRadians < b.angularDeviationRadians;
        }
        return a.distanceSquared < b.distanceSquared;
    });
    return result;
}

inline double snapToNiceAngleStep(double idealStep) {
    // Nice values, logarithmic nearest. Includes both the zoom-out branch
    // (15, 30, 45, 60) and the zoom-in branch (15, 10, 5, 2, 1, 0.5, 0.2, 0.1).
    constexpr double kNiceSteps[] = {0.1, 0.2, 0.5, 1.0, 2.0, 5.0, 10.0, 15.0, 30.0, 45.0, 60.0};
    double best = kNiceSteps[0];
    double bestDist = std::abs(std::log(idealStep / best));
    for (double candidate : kNiceSteps) {
        const double d = std::abs(std::log(idealStep / candidate));
        if (d < bestDist) { bestDist = d; best = candidate; }
    }
    return best;
}

inline std::optional<DirectionSnapCandidate> findBestDirectionSnap(const Vec2& start, const Vec2& cursor, std::span<const SketchEntity> entities,
                                                                   double toleranceDegrees = 25.0, double angleStepDegrees = 15.0) {
    auto candidates = findDirectionSnaps(start, cursor, entities, toleranceDegrees, angleStepDegrees);
    if (candidates.empty()) {
        return std::nullopt;
    }
    return candidates.front();
}

}  // namespace geometry

#endif  // OURPAINT_SNAP_DIRECTION_QUERY_H
