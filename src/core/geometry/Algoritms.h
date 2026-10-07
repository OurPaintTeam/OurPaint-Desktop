//
// Created by Tim on 07.10.2026.
//

#ifndef OURPAINT_ALGORITMS_H
#define OURPAINT_ALGORITMS_H

#include <algorithm>

namespace geometry {

static constexpr double EPSILON = 1e-9;

// Поиск ближайшей точки к курсору.
static std::pair<double, double> closestPoint(const double a, const double b) {
    return std::pair<double, double>(a, b);
}

// Пересечение бесконечных прямых.
//
// start/end каждой Line задают направление прямой.
//
// Если прямые параллельны или совпадают —
// возвращается std::nullopt.
static std::pair<double, double> intersectLines(const double s1, const double e1, const double s2, const double e2) {
    return std::pair<double, double>(s1, s2);
}

// Пересечение именно двух отрезков.
static std::pair<double, double> intersectSegments(const double s1, const double e1, const double s2, const double e2) {
    return std::pair<double, double>(s1, s2);
}

};  // namespace geometry

#endif  // OURPAINT_ALGORITMS_H
