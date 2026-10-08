//
// Created by Tim on 08.10.2026.
//

/*
 расстояние между точками;
 квадрат расстояния;
 ближайшая точка на отрезке;
 ближайшая точка до начала/конца отрезка;
 ближайшая точка на бесконечной прямой;
 вырожденный отрезок;
 пересечение бесконечных прямых;
 параллельные прямые;
 совпадающие прямые;
 пересечение отрезков;
 пересечение в конечной точке;
 непересекающиеся отрезки;
 продолжение прямых за пределы отрезков;
 пересечение продолжений до/после отрезков;
 отсутствие даже расширенного пересечения для параллельных линий.
 midpoint есть
 midpoint вырожденный
 */

#include <gtest/gtest.h>

#include "geometry/Algorithms.h"
#include "sketch/SketchTypes.h"

namespace {

using core::sketch::Line2;
using core::sketch::Vec2;

constexpr double EPS = 1e-9;

// ============================================================
// Distance
// ============================================================

TEST(GeometryAlgorithms, DistanceSquared) {
    const Vec2 a{0.0, 0.0};
    const Vec2 b{3.0, 4.0};

    EXPECT_DOUBLE_EQ(geometry::distanceSquared(a, b), 25.0);
}

TEST(GeometryAlgorithms, Distance) {
    const Vec2 a{0.0, 0.0};
    const Vec2 b{3.0, 4.0};

    EXPECT_NEAR(geometry::distance(a, b), 5.0, EPS);
}

TEST(GeometryAlgorithms, DistanceBetweenSamePoints) {
    const Vec2 a{5.0, 7.0};

    EXPECT_DOUBLE_EQ(geometry::distance(a, a), 0.0);
}

// ============================================================
// Closest point on segment
// ============================================================

TEST(GeometryAlgorithms, ClosestPointInsideSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const Vec2 point{4.0, 3.0};

    const Vec2 result = geometry::closestPoint(point, line);

    EXPECT_NEAR(result.x, 4.0, EPS);
    EXPECT_NEAR(result.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointBeforeSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const Vec2 point{-5.0, 3.0};

    const Vec2 result = geometry::closestPoint(point, line);

    EXPECT_NEAR(result.x, 0.0, EPS);
    EXPECT_NEAR(result.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointAfterSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const Vec2 point{15.0, 3.0};

    const Vec2 result = geometry::closestPoint(point, line);

    EXPECT_NEAR(result.x, 10.0, EPS);
    EXPECT_NEAR(result.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnDiagonalSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 10.0}};

    const Vec2 point{5.0, 0.0};

    const Vec2 result = geometry::closestPoint(point, line);

    EXPECT_NEAR(result.x, 2.5, EPS);
    EXPECT_NEAR(result.y, 2.5, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnDegenerateSegment) {
    const Line2 line{{5.0, 7.0}, {5.0, 7.0}};

    const Vec2 point{100.0, 100.0};

    const Vec2 result = geometry::closestPoint(point, line);

    EXPECT_NEAR(result.x, 5.0, EPS);
    EXPECT_NEAR(result.y, 7.0, EPS);
}

// ============================================================
// Closest point on infinite line
// ============================================================

TEST(GeometryAlgorithms, ClosestPointOnInfiniteLine) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const Vec2 point{15.0, 3.0};

    const auto result = geometry::closestPointOnLine(point, line);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 15.0, EPS);
    EXPECT_NEAR(result->point.y, 0.0, EPS);

    EXPECT_NEAR(result->lineStart.x, 0.0, EPS);
    EXPECT_NEAR(result->lineStart.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnInfiniteLineBeforeStart) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const Vec2 point{-5.0, 3.0};

    const auto result = geometry::closestPointOnLine(point, line);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, -5.0, EPS);
    EXPECT_NEAR(result->point.y, 0.0, EPS);

    EXPECT_NEAR(result->lineStart.x, 0.0, EPS);
    EXPECT_NEAR(result->lineStart.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnInfiniteDiagonalLine) {
    const Line2 line{{0.0, 0.0}, {10.0, 10.0}};

    const Vec2 point{10.0, 0.0};

    const auto result = geometry::closestPointOnLine(point, line);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 5.0, EPS);
    EXPECT_NEAR(result->point.y, 5.0, EPS);

    EXPECT_NEAR(result->lineStart.x, 0.0, EPS);
    EXPECT_NEAR(result->lineStart.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnInfiniteLineDegenerate) {
    const Line2 line{{5.0, 7.0}, {5.0, 7.0}};

    const Vec2 point{100.0, 100.0};

    const auto result = geometry::closestPointOnLine(point, line);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Infinite line intersection
// ============================================================

TEST(GeometryAlgorithms, IntersectLines) {
    const Line2 lineA{{0.0, 0.0}, {10.0, 10.0}};

    const Line2 lineB{{0.0, 10.0}, {10.0, 0.0}};

    const auto result = geometry::intersectLines(lineA, lineB);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->intersection.x, 5.0, EPS);
    EXPECT_NEAR(result->intersection.y, 5.0, EPS);

    EXPECT_NEAR(result->startA.x, 0.0, EPS);
    EXPECT_NEAR(result->startA.y, 0.0, EPS);

    EXPECT_NEAR(result->startB.x, 0.0, EPS);
    EXPECT_NEAR(result->startB.y, 10.0, EPS);
}

TEST(GeometryAlgorithms, ParallelLines) {
    const Line2 lineA{{0.0, 0.0}, {10.0, 0.0}};

    const Line2 lineB{{0.0, 5.0}, {10.0, 5.0}};

    const auto result = geometry::intersectLines(lineA, lineB);

    EXPECT_FALSE(result.has_value());
}

TEST(GeometryAlgorithms, CoincidentLines) {
    const Line2 lineA{{0.0, 0.0}, {10.0, 0.0}};

    const Line2 lineB{{5.0, 0.0}, {20.0, 0.0}};

    const auto result = geometry::intersectLines(lineA, lineB);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Segment intersection
// ============================================================

TEST(GeometryAlgorithms, IntersectSegments) {
    const Line2 segmentA{{0.0, 0.0}, {10.0, 10.0}};

    const Line2 segmentB{{0.0, 10.0}, {10.0, 0.0}};

    const auto result = geometry::intersectSegments(segmentA, segmentB);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->x, 5.0, EPS);
    EXPECT_NEAR(result->y, 5.0, EPS);
}

TEST(GeometryAlgorithms, SegmentIntersectionAtEndpoint) {
    const Line2 segmentA{{0.0, 0.0}, {10.0, 0.0}};

    const Line2 segmentB{{10.0, 0.0}, {10.0, 10.0}};

    const auto result = geometry::intersectSegments(segmentA, segmentB);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->x, 10.0, EPS);
    EXPECT_NEAR(result->y, 0.0, EPS);
}

TEST(GeometryAlgorithms, SegmentsDoNotIntersect) {
    const Line2 segmentA{{0.0, 0.0}, {2.0, 0.0}};

    const Line2 segmentB{{5.0, -1.0}, {5.0, 1.0}};

    const auto result = geometry::intersectSegments(segmentA, segmentB);

    EXPECT_FALSE(result.has_value());
}

TEST(GeometryAlgorithms, ParallelSegmentsDoNotIntersect) {
    const Line2 segmentA{{0.0, 0.0}, {10.0, 0.0}};

    const Line2 segmentB{{0.0, 5.0}, {10.0, 5.0}};

    const auto result = geometry::intersectSegments(segmentA, segmentB);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Extended intersection
//
// Отрезки не пересекаются,
// но бесконечные линии имеют общую точку.
// ============================================================

TEST(GeometryAlgorithms, ExtendedIntersection) {
    const Line2 segmentA{{0.0, 0.0}, {1.0, 1.0}};

    const Line2 segmentB{{2.0, 0.0}, {3.0, -1.0}};

    // Отрезки не пересекаются.
    const auto segmentResult = geometry::intersectSegments(segmentA, segmentB);

    EXPECT_FALSE(segmentResult.has_value());

    // Бесконечные продолжения пересекаются.
    const auto lineResult = geometry::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(lineResult.has_value());

    EXPECT_NEAR(lineResult->intersection.x, 1.0, EPS);
    EXPECT_NEAR(lineResult->intersection.y, 1.0, EPS);

    EXPECT_NEAR(lineResult->startA.x, 0.0, EPS);
    EXPECT_NEAR(lineResult->startA.y, 0.0, EPS);

    EXPECT_NEAR(lineResult->startB.x, 2.0, EPS);
    EXPECT_NEAR(lineResult->startB.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ExtendedIntersectionAfterSegment) {
    const Line2 segmentA{{0.0, 0.0}, {1.0, 1.0}};

    const Line2 segmentB{{3.0, 0.0}, {4.0, -1.0}};

    const auto segmentResult = geometry::intersectSegments(segmentA, segmentB);

    EXPECT_FALSE(segmentResult.has_value());

    const auto lineResult = geometry::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(lineResult.has_value());

    EXPECT_NEAR(lineResult->intersection.x, 1.5, EPS);
    EXPECT_NEAR(lineResult->intersection.y, 1.5, EPS);

    EXPECT_NEAR(lineResult->startA.x, 0.0, EPS);
    EXPECT_NEAR(lineResult->startA.y, 0.0, EPS);

    EXPECT_NEAR(lineResult->startB.x, 3.0, EPS);
    EXPECT_NEAR(lineResult->startB.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ExtendedIntersectionBeforeSegment) {
    const Line2 segmentA{{2.0, 2.0}, {4.0, 4.0}};

    const Line2 segmentB{{0.0, 4.0}, {0.0, 0.0}};

    const auto segmentResult = geometry::intersectSegments(segmentA, segmentB);

    EXPECT_FALSE(segmentResult.has_value());

    const auto lineResult = geometry::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(lineResult.has_value());

    EXPECT_NEAR(lineResult->intersection.x, 0.0, EPS);
    EXPECT_NEAR(lineResult->intersection.y, 0.0, EPS);

    EXPECT_NEAR(lineResult->startA.x, 2.0, EPS);
    EXPECT_NEAR(lineResult->startA.y, 2.0, EPS);

    EXPECT_NEAR(lineResult->startB.x, 0.0, EPS);
    EXPECT_NEAR(lineResult->startB.y, 4.0, EPS);
}

// ============================================================
// Extended intersection must not exist for parallel lines
// ============================================================

TEST(GeometryAlgorithms, NoExtendedIntersectionForParallelLines) {
    const Line2 segmentA{{0.0, 0.0}, {1.0, 0.0}};

    const Line2 segmentB{{0.0, 5.0}, {1.0, 5.0}};

    const auto segmentResult = geometry::intersectSegments(segmentA, segmentB);

    const auto lineResult = geometry::intersectLines(segmentA, segmentB);

    EXPECT_FALSE(segmentResult.has_value());
    EXPECT_FALSE(lineResult.has_value());
}

// ============================================================
// Midpoint of a segment
// ============================================================

TEST(GeometryAlgorithms, MidpointOfSegment) {
    const Line2 segment{{0.0, 0.0}, {10.0, 4.0}};

    const auto result = geometry::midpoint(segment);

    ASSERT_TRUE(result.has_value());

    EXPECT_DOUBLE_EQ(result->x, 5.0);
    EXPECT_DOUBLE_EQ(result->y, 2.0);
}

// ============================================================
// Midpoint of a degenerate segment
// ============================================================

TEST(GeometryAlgorithms, NoMidpointForDegenerateSegment) {
    const Line2 segment{{3.0, 7.0}, {3.0, 7.0}};

    const auto result = geometry::midpoint(segment);

    EXPECT_FALSE(result.has_value());
}

}  // namespace