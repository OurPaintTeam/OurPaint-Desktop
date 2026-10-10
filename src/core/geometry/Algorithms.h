#ifndef OURPAINT_ALGORITHMS_H
#define OURPAINT_ALGORITHMS_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <span>

#include "sketch/SketchTypes.h"

namespace geometry {

using Vec2 = core::sketch::Vec2;
using Point2 = core::sketch::Point2;
using Line2 = core::sketch::Line2;
using Circle2 = core::sketch::Circle2;
using Arc2 = core::sketch::Arc2;
using SketchEntity = core::sketch::SketchEntity;
using GeometryRef = core::sketch::GeometryRef;
using SubElement = core::sketch::SubElement;

inline constexpr double EPSILON = 1e-9;

// Результат поиска ближайшей точки.
struct PointCandidate {
    Vec2 point;
    double distanceSquared = 0.0;
    GeometryRef source;
};

// Результат поиска пересечения.
struct IntersectionCandidate {
    Vec2 point;
    double distanceSquared = 0.0;
    GeometryRef first;
    GeometryRef second;
};

namespace detail {

inline Vec2 subtract(const Vec2& a, const Vec2& b) {
    return {a.x - b.x, a.y - b.y};
}

inline double dot(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.y * b.y;
}

inline double cross(const Vec2& a, const Vec2& b) {
    return a.x * b.y - a.y * b.x;
}

inline double lengthSquared(const Vec2& v) {
    return dot(v, v);
}

inline double distanceSquared(const Vec2& a, const Vec2& b) {
    return lengthSquared(subtract(a, b));
}

inline bool isFinite(const Vec2& p) {
    return std::isfinite(p.x) && std::isfinite(p.y);
}

inline bool isFinite(const Line2& line) {
    return isFinite(line.start) && isFinite(line.end);
}

inline bool isFinite(const Circle2& circle) {
    return isFinite(circle.center) &&
           std::isfinite(circle.radius) &&
           circle.radius >= 0.0;
}

inline bool isDegenerate(const Vec2& start, const Vec2& end) {
    return distanceSquared(start, end) <= EPSILON * EPSILON;
}

inline bool isDegenerate(const Line2& line) {
    return isDegenerate(line.start, line.end);
}

// Ближайшая точка на конечном отрезке.
inline std::optional<Vec2> closestPointOnSegment(
    const Vec2& cursor,
    const Line2& line)
{
    if (!isFinite(cursor) || !isFinite(line))
        return std::nullopt;

    const Vec2 direction = subtract(line.end, line.start);
    const double length2 = lengthSquared(direction);

    if (length2 <= EPSILON * EPSILON)
        return line.start;

    const Vec2 offset = subtract(cursor, line.start);

    const double t = std::clamp(
        dot(offset, direction) / length2,
        0.0,
        1.0);

    return Vec2{
        line.start.x + t * direction.x,
        line.start.y + t * direction.y
    };
}

// Ближайшая точка на окружности.
inline std::optional<Vec2> closestPointOnCircle(
    const Vec2& cursor,
    const Circle2& circle)
{
    if (!isFinite(cursor) || !isFinite(circle))
        return std::nullopt;

    const Vec2 offset = subtract(cursor, circle.center);
    const double length2 = lengthSquared(offset);

    // В центре направление неоднозначно.
    // Выбираем точку справа от центра.
    if (length2 <= EPSILON * EPSILON) {
        return Vec2{
            circle.center.x + circle.radius,
            circle.center.y
        };
    }

    const double scale = circle.radius / std::sqrt(length2);

    return Vec2{
        circle.center.x + offset.x * scale,
        circle.center.y + offset.y * scale
    };
}

struct LineIntersection {
    Vec2 point;
    double t = 0.0;
    double u = 0.0;
};

// Пересечение бесконечных прямых.
// t и u задают положение пересечения относительно отрезков.
inline std::optional<LineIntersection> intersectLines(
    const Line2& a,
    const Line2& b)
{
    if (!isFinite(a) || !isFinite(b) ||
        isDegenerate(a) || isDegenerate(b)) {
        return std::nullopt;
    }

    const Vec2 da = subtract(a.end, a.start);
    const Vec2 db = subtract(b.end, b.start);
    const Vec2 offset = subtract(b.start, a.start);

    const double denominator = cross(da, db);

    if (std::abs(denominator) <= EPSILON)
        return std::nullopt;

    const double t = cross(offset, db) / denominator;
    const double u = cross(offset, da) / denominator;

    return LineIntersection{
        {
            a.start.x + t * da.x,
            a.start.y + t * da.y
        },
        t,
        u
    };
}

inline bool onSegment(double t) {
    return t >= -EPSILON && t <= 1.0 + EPSILON;
}

inline bool isBetter(
    const PointCandidate& candidate,
    const PointCandidate& current)
{
    return candidate.distanceSquared < current.distanceSquared;
}

inline bool isBetter(
    const IntersectionCandidate& candidate,
    const IntersectionCandidate& current)
{
    return candidate.distanceSquared < current.distanceSquared;
}

// consider: поиск лучшего кандидата по отдельным аргументам.
inline void consider(
    std::optional<PointCandidate>& best,
    const Vec2& cursor,
    const Vec2& point,
    GeometryRef source)
{
    if (!isFinite(cursor) || !isFinite(point))
        return;

    PointCandidate candidate{
        point,
        distanceSquared(cursor, point),
        source
    };

    if (!best || isBetter(candidate, *best))
        best = candidate;
}

// consider: приём уже сформированного кандидата.
// Эта перегрузка нужна для findNearestLine и findNearestCircle.
inline void consider(
    std::optional<PointCandidate>& best,
    PointCandidate candidate)
{
    if (!isFinite(candidate.point) ||
        !std::isfinite(candidate.distanceSquared) ||
        candidate.distanceSquared < 0.0) {
        return;
    }

    if (!best || isBetter(candidate, *best))
        best = candidate;
}

inline void consider(
    std::optional<IntersectionCandidate>& best,
    const Vec2& cursor,
    const Vec2& point,
    GeometryRef first,
    GeometryRef second)
{
    if (!isFinite(cursor) || !isFinite(point))
        return;

    IntersectionCandidate candidate{
        point,
        distanceSquared(cursor, point),
        first,
        second
    };

    if (!best || isBetter(candidate, *best))
        best = candidate;
}

// Пересечения отрезка и окружности.
inline bool intersectLineCircle(
    const Line2& line,
    const Circle2& circle,
    const Vec2& cursor,
    GeometryRef lineRef,
    GeometryRef circleRef,
    std::optional<IntersectionCandidate>& best)
{
    if (!isFinite(line) || !isFinite(circle) ||
        !isFinite(cursor) || isDegenerate(line)) {
        return false;
    }

    const Vec2 direction = subtract(line.end, line.start);
    const Vec2 offset = subtract(line.start, circle.center);

    const double a = lengthSquared(direction);
    const double b = 2.0 * dot(offset, direction);
    const double c = lengthSquared(offset)
                   - circle.radius * circle.radius;

    double discriminant = b * b - 4.0 * a * c;

    if (discriminant < -EPSILON)
        return false;

    discriminant = std::max(0.0, discriminant);

    const double root = std::sqrt(discriminant);
    const double t1 = (-b - root) / (2.0 * a);
    const double t2 = (-b + root) / (2.0 * a);

    bool found = false;

    const auto add = [&](double t) {
        if (!onSegment(t))
            return;

        const Vec2 point{
            line.start.x + t * direction.x,
            line.start.y + t * direction.y
        };

        consider(best, cursor, point, lineRef, circleRef);
        found = true;
    };

    add(t1);

    if (std::abs(t2 - t1) > EPSILON)
        add(t2);

    return found;
}

// Пересечения двух окружностей.
inline void intersectCircles(
    const Circle2& a,
    const Circle2& b,
    const Vec2& cursor,
    GeometryRef refA,
    GeometryRef refB,
    std::optional<IntersectionCandidate>& best)
{
    if (!isFinite(a) || !isFinite(b) || !isFinite(cursor))
        return;

    const Vec2 delta = subtract(b.center, a.center);
    const double d2 = lengthSquared(delta);

    // Совпадающие центры: нет единственной точки пересечения.
    if (d2 <= EPSILON * EPSILON)
        return;

    const double d = std::sqrt(d2);

    if (d > a.radius + b.radius + EPSILON)
        return;

    if (d < std::abs(a.radius - b.radius) - EPSILON)
        return;

    const double x =
        (a.radius * a.radius - b.radius * b.radius + d2) /
        (2.0 * d);

    double h2 = a.radius * a.radius - x * x;

    if (h2 < -EPSILON)
        return;

    h2 = std::max(0.0, h2);

    const double h = std::sqrt(h2);

    const Vec2 base{
        a.center.x + x * delta.x / d,
        a.center.y + x * delta.y / d
    };

    const Vec2 offset{
        -delta.y * h / d,
        delta.x * h / d
    };

    consider(
        best, cursor,
        {base.x + offset.x, base.y + offset.y},
        refA, refB);

    if (h > EPSILON) {
        consider(
            best, cursor,
            {base.x - offset.x, base.y - offset.y},
            refA, refB);
    }
}

} // namespace detail

// 1. Расстояние между двумя точками.
inline double distance(const Vec2& a, const Vec2& b) {
    return std::sqrt(detail::distanceSquared(a, b));
}

// 2. Ближайшая самостоятельная точка.
// Учитываются только сущности Point2.
inline std::optional<PointCandidate> findNearestStandalonePoint(
    const Vec2& cursor,
    std::span<const SketchEntity> entities)
{
    if (!detail::isFinite(cursor))
        return std::nullopt;

    std::optional<PointCandidate> best;

    for (const auto& entity : entities) {
        const auto* point = std::get_if<Point2>(&entity.geometry);

        if (!point)
            continue;

        detail::consider(
            best,
            cursor,
            point->position,
            GeometryRef{entity.id, SubElement::Whole});
    }

    return best;
}

// 3. Ближайшая точка на отрезках.
// Внутри отрезка source.sub = Whole.
// На концах source.sub = Start или End.
inline std::optional<PointCandidate> findNearestLine(
    const Vec2& cursor,
    std::span<const SketchEntity> entities)
{
    if (!detail::isFinite(cursor))
        return std::nullopt;

    std::optional<PointCandidate> best;

    for (const auto& entity : entities) {
        const auto* line = std::get_if<Line2>(&entity.geometry);

        if (!line)
            continue;

        const auto closest =
            detail::closestPointOnSegment(cursor, *line);

        if (!closest)
            continue;

        const Vec2 point = *closest;

        GeometryRef source{entity.id, SubElement::Whole};

        if (detail::distanceSquared(point, line->start)
            <= EPSILON * EPSILON) {
            source.sub = SubElement::Start;
        } else if (
            detail::distanceSquared(point, line->end)
            <= EPSILON * EPSILON) {
            source.sub = SubElement::End;
        }

        detail::consider(best, PointCandidate{
            point,
            detail::distanceSquared(cursor, point),
            source
        });
    }

    return best;
}

// 4. Ближайшая точка на окружности или её центр.
// Если курсор ближе к центру, возвращается Center.
// Иначе возвращается точка на окружности (Whole).
inline std::optional<PointCandidate> findNearestCircle(
    const Vec2& cursor,
    std::span<const SketchEntity> entities)
{
    if (!detail::isFinite(cursor))
        return std::nullopt;

    std::optional<PointCandidate> best;

    for (const auto& entity : entities) {
        const auto* circle = std::get_if<Circle2>(&entity.geometry);

        if (!circle || !detail::isFinite(*circle) ||
            circle->radius <= 0.0) {
            continue;
        }

        const auto boundaryResult =
            detail::closestPointOnCircle(cursor, *circle);

        if (!boundaryResult)
            continue;

        const Vec2 boundary = *boundaryResult;

        const double boundaryDistance =
            detail::distanceSquared(cursor, boundary);

        const double centerDistance =
            detail::distanceSquared(cursor, circle->center);

        const bool useCenter = centerDistance < boundaryDistance;

        const Vec2 point = useCenter ? circle->center : boundary;

        const GeometryRef source{
            entity.id,
            useCenter ? SubElement::Center : SubElement::Whole
        };

        detail::consider(best, PointCandidate{
            point,
            detail::distanceSquared(cursor, point),
            source
        });
    }

    return best;
}

// 5. Проверка близости курсора к середине одной линии.
// tolerance задаётся в единицах координат эскиза.
inline bool isLineCenter(
    const Vec2& cursor,
    const Line2& line,
    double tolerance)
{
    if (!detail::isFinite(cursor) ||
        !detail::isFinite(line) ||
        !std::isfinite(tolerance) ||
        tolerance < 0.0) {
        return false;
    }

    if (detail::isDegenerate(line))
        return false;

    const Vec2 center{
        (line.start.x + line.end.x) * 0.5,
        (line.start.y + line.end.y) * 0.5
    };

    return detail::distanceSquared(cursor, center)
        <= tolerance * tolerance;
}

// 6. Ближайшее реальное пересечение:
// отрезок–отрезок, отрезок–окружность, окружность–окружность.
// Дуги пока не обрабатываются.
inline std::optional<IntersectionCandidate> findNearestIntersection(
    const Vec2& cursor,
    std::span<const SketchEntity> entities)
{
    if (!detail::isFinite(cursor))
        return std::nullopt;

    std::optional<IntersectionCandidate> best;

    for (std::size_t i = 0; i < entities.size(); ++i) {
        const auto& a = entities[i];

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            const auto& b = entities[j];

            const auto* lineA = std::get_if<Line2>(&a.geometry);
            const auto* lineB = std::get_if<Line2>(&b.geometry);
            const auto* circleA = std::get_if<Circle2>(&a.geometry);
            const auto* circleB = std::get_if<Circle2>(&b.geometry);

            const GeometryRef refA{a.id, SubElement::Whole};
            const GeometryRef refB{b.id, SubElement::Whole};

            if (lineA && lineB) {
                const auto intersection =
                    detail::intersectLines(*lineA, *lineB);

                if (intersection &&
                    detail::onSegment(intersection->t) &&
                    detail::onSegment(intersection->u)) {
                    detail::consider(
                        best, cursor, intersection->point, refA, refB);
                }
            } else if (lineA && circleB) {
                detail::intersectLineCircle(
                    *lineA, *circleB, cursor, refA, refB, best);
            } else if (circleA && lineB) {
                detail::intersectLineCircle(
                    *lineB, *circleA, cursor, refB, refA, best);
            } else if (circleA && circleB) {
                detail::intersectCircles(
                    *circleA, *circleB, cursor, refA, refB, best);
            }
        }
    }

    return best;
}

// 7. Ближайшее пересечение продолжений отрезков.
// Возвращает пересечения бесконечных прямых,
// находящиеся за пределами хотя бы одного исходного отрезка.
inline std::optional<IntersectionCandidate>
findNearestExtendedLineIntersection(
    const Vec2& cursor,
    std::span<const SketchEntity> entities)
{
    if (!detail::isFinite(cursor))
        return std::nullopt;

    std::optional<IntersectionCandidate> best;

    for (std::size_t i = 0; i < entities.size(); ++i) {
        const auto* a = std::get_if<Line2>(&entities[i].geometry);

        if (!a)
            continue;

        for (std::size_t j = i + 1; j < entities.size(); ++j) {
            const auto* b = std::get_if<Line2>(&entities[j].geometry);

            if (!b)
                continue;

            const auto intersection = detail::intersectLines(*a, *b);

            if (!intersection)
                continue;

            if (detail::onSegment(intersection->t) &&
                detail::onSegment(intersection->u)) {
                continue;
            }

            detail::consider(
                best,
                cursor,
                intersection->point,
                GeometryRef{entities[i].id, SubElement::Whole},
                GeometryRef{entities[j].id, SubElement::Whole});
        }
    }

    return best;
}

} // namespace geometry

#endif // OURPAINT_ALGORITHMS_H
