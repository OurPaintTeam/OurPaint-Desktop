//
// Created by Tim on 07.10.2026.
//

#ifndef OURPAINT_ALGORITMS_H
#define OURPAINT_ALGORITMS_H

#include <algorithm>
#include <cmath>
#include <optional>

#include "sketch/SketchTypes.h"

namespace geometry {

/// Допуск для сравнения вещественных чисел.
static constexpr double EPSILON = 1e-9;

/// Пересечение двух бесконечных прямых.
/// Хранит начала исходных прямых и точку пересечения —
/// достаточно для построения виртуального продолжения.
struct ExtendedIntersection {
    core::sketch::Vec2 startA;
    core::sketch::Vec2 startB;
    core::sketch::Vec2 intersection;
};

/// Ближайшая точка на бесконечной прямой.
/// Хранит точку и начало прямой — для построения продолжения.
struct ClosestPointOnLineResult {
    core::sketch::Vec2 point;
    core::sketch::Vec2 lineStart;
};

// =========================Point===================================

/// Квадрат расстояния между точками (без sqrt — для сравнений).
static double distanceSquared(const core::sketch::Vec2& a, const core::sketch::Vec2& b) {
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;

    return dx * dx + dy * dy;
}

/// Расстояние между точками.
static double distance(const core::sketch::Vec2& a, const core::sketch::Vec2& b) { return std::sqrt(distanceSquared(a, b)); }

/// Ближайшая точка на отрезке.
/// Если проекция выходит за границы — возвращает ближайший конец.
static core::sketch::Vec2 closestPoint(const core::sketch::Vec2& point, const core::sketch::Line2& line) {
    const double dx = line.end.x - line.start.x;
    const double dy = line.end.y - line.start.y;

    const double lengthSquared = dx * dx + dy * dy;

    // Вырожденный отрезок.
    if (lengthSquared <= EPSILON) {
        return line.start;
    }

    const double px = point.x - line.start.x;
    const double py = point.y - line.start.y;

    double t = (px * dx + py * dy) / lengthSquared;

    // Ограничиваем проекцию границами отрезка.
    t = std::clamp(t, 0.0, 1.0);

    return {line.start.x + t * dx, line.start.y + t * dy};
}

/// Ближайшая точка на бесконечной прямой.
/// Проекция может выходить за start/end.
static std::optional<ClosestPointOnLineResult> closestPointOnLine(const core::sketch::Vec2& point, const core::sketch::Line2& line) {
    const double dx = line.end.x - line.start.x;
    const double dy = line.end.y - line.start.y;

    const double lengthSquared = dx * dx + dy * dy;

    if (lengthSquared <= EPSILON) {
        return std::nullopt;
    }

    const double px = point.x - line.start.x;
    const double py = point.y - line.start.y;

    const double t = (px * dx + py * dy) / lengthSquared;

    const core::sketch::Vec2 closestPoint{line.start.x + t * dx, line.start.y + t * dy};

    return ClosestPointOnLineResult{.point = closestPoint, .lineStart = line.start};
}

// =============Line intersection===========================

/// Пересечение двух бесконечных прямых.
/// Параллельные и совпадающие прямые → std::nullopt.
static std::optional<ExtendedIntersection> intersectLines(const core::sketch::Line2& lineA, const core::sketch::Line2& lineB) {
    const double x1 = lineA.start.x;
    const double y1 = lineA.start.y;

    const double x2 = lineA.end.x;
    const double y2 = lineA.end.y;

    const double x3 = lineB.start.x;
    const double y3 = lineB.start.y;

    const double x4 = lineB.end.x;
    const double y4 = lineB.end.y;

    const double dx1 = x2 - x1;
    const double dy1 = y2 - y1;

    const double dx2 = x4 - x3;
    const double dy2 = y4 - y3;

    const double denominator = dx1 * dy2 - dy1 * dx2;

    // Прямые параллельны или совпадают.
    if (std::abs(denominator) <= EPSILON) {
        return std::nullopt;
    }

    const double dx3 = x3 - x1;
    const double dy3 = y3 - y1;

    const double t = (dx3 * dy2 - dy3 * dx2) / denominator;

    const core::sketch::Vec2 intersection{x1 + t * dx1, y1 + t * dy1};

    return ExtendedIntersection{.startA = lineA.start, .startB = lineB.start, .intersection = intersection};
}

// =================Segment intersection=============================

/// Пересечение двух отрезков.
/// Не пересекаются → std::nullopt.
/// Коллинеарные/совпадающие отрезки → std::nullopt
/// (у них нет единственной точки пересечения).
static std::optional<core::sketch::Vec2> intersectSegments(const core::sketch::Line2& segmentA, const core::sketch::Line2& segmentB) {
    const double x1 = segmentA.start.x;
    const double y1 = segmentA.start.y;

    const double x2 = segmentA.end.x;
    const double y2 = segmentA.end.y;

    const double x3 = segmentB.start.x;
    const double y3 = segmentB.start.y;

    const double x4 = segmentB.end.x;
    const double y4 = segmentB.end.y;

    const double dx1 = x2 - x1;
    const double dy1 = y2 - y1;

    const double dx2 = x4 - x3;
    const double dy2 = y4 - y3;

    const double denominator = dx1 * dy2 - dy1 * dx2;

    // Параллельные или коллинеарные отрезки.
    if (std::abs(denominator) <= EPSILON) {
        return std::nullopt;
    }

    const double dx3 = x3 - x1;
    const double dy3 = y3 - y1;

    const double t = (dx3 * dy2 - dy3 * dx2) / denominator;

    const double u = (dx3 * dy1 - dy3 * dx1) / denominator;

    // Пересечение находится за пределами
    // хотя бы одного из отрезков.
    if (t < -EPSILON || t > 1.0 + EPSILON) {
        return std::nullopt;
    }

    if (u < -EPSILON || u > 1.0 + EPSILON) {
        return std::nullopt;
    }

    return core::sketch::Vec2{x1 + t * dx1, y1 + t * dy1};
}

// ==================== Midpoint ====================

/// Середина отрезка.
/// Возвращает nullopt для вырожденного отрезка (start == end).
static std::optional<core::sketch::Vec2> midpoint(const core::sketch::Line2& line) {
    const double dx = line.end.x - line.start.x;

    // Вырожденный отрезок — середины нет.
    if (const double dy = line.end.y - line.start.y; dx * dx + dy * dy <= EPSILON) {
        return std::nullopt;
    }

    return core::sketch::Vec2{(line.start.x + line.end.x) * 0.5, (line.start.y + line.end.y) * 0.5};
}

}  // namespace geometry

#endif  // OURPAINT_ALGORITMS_H