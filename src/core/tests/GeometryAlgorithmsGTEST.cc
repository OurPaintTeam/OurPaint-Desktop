//
// Created by Tim on 08.10.2026.
//

#include <limits>

#include <gtest/gtest.h>

#include "geometry/CircleAlgorithms.h"
#include "geometry/GeometryMath.h"
#include "geometry/LineAlgorithms.h"
#include "sketch/SketchTypes.h"

namespace {

using core::sketch::Circle2;
using core::sketch::Line2;
using core::sketch::Vec2;

constexpr double EPS = 1e-9;

// ============================================================
// Distance
// ============================================================

TEST(GeometryAlgorithms, DistanceSquared) {
    EXPECT_DOUBLE_EQ(
        geometry::detail::distanceSquared(
            Vec2{0.0, 0.0}, Vec2{3.0, 4.0}),
        25.0);
}

TEST(GeometryAlgorithms, Distance) {
    EXPECT_NEAR(
        geometry::distance(Vec2{0.0, 0.0}, Vec2{3.0, 4.0}),
        5.0,
        EPS);
}

TEST(GeometryAlgorithms, DistanceBetweenSamePoints) {
    const Vec2 point{5.0, 7.0};

    EXPECT_DOUBLE_EQ(geometry::distance(point, point), 0.0);
}

// ============================================================
// Closest point on segment
// ============================================================

TEST(GeometryAlgorithms, ClosestPointInsideSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const auto result =
        geometry::detail::closestPointOnSegment(Vec2{4.0, 3.0}, line);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 4.0, EPS);
    EXPECT_NEAR(result->y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointBeforeSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const auto result =
        geometry::detail::closestPointOnSegment(Vec2{-5.0, 3.0}, line);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 0.0, EPS);
    EXPECT_NEAR(result->y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointAfterSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const auto result =
        geometry::detail::closestPointOnSegment(Vec2{15.0, 3.0}, line);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 10.0, EPS);
    EXPECT_NEAR(result->y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnDiagonalSegment) {
    const Line2 line{{0.0, 0.0}, {10.0, 10.0}};

    const auto result =
        geometry::detail::closestPointOnSegment(Vec2{5.0, 0.0}, line);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 2.5, EPS);
    EXPECT_NEAR(result->y, 2.5, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnDegenerateSegment) {
    const Line2 line{{5.0, 7.0}, {5.0, 7.0}};

    const auto result =
        geometry::detail::closestPointOnSegment(Vec2{100.0, 100.0}, line);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 5.0, EPS);
    EXPECT_NEAR(result->y, 7.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointRejectsNonFiniteCursor) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const Line2 line{{0.0, 0.0}, {10.0, 0.0}};

    const auto result =
        geometry::detail::closestPointOnSegment(Vec2{nan, 0.0}, line);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Closest point on circle
// ============================================================

TEST(GeometryAlgorithms, ClosestPointOnCircleBoundary) {
    const Circle2 circle{{0.0, 0.0}, 5.0};

    const auto result =
        geometry::detail::closestPointOnCircle(Vec2{10.0, 0.0}, circle);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 5.0, EPS);
    EXPECT_NEAR(result->y, 0.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnCircleFromCenter) {
    const Circle2 circle{{2.0, 3.0}, 5.0};

    const auto result =
        geometry::detail::closestPointOnCircle(Vec2{2.0, 3.0}, circle);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 7.0, EPS);
    EXPECT_NEAR(result->y, 3.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnZeroRadiusCircle) {
    const Circle2 circle{{2.0, 3.0}, 0.0};

    const auto result =
        geometry::detail::closestPointOnCircle(Vec2{10.0, 3.0}, circle);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->x, 2.0, EPS);
    EXPECT_NEAR(result->y, 3.0, EPS);
}

TEST(GeometryAlgorithms, ClosestPointOnCircleRejectsNegativeRadius) {
    const Circle2 circle{{0.0, 0.0}, -1.0};

    const auto result =
        geometry::detail::closestPointOnCircle(Vec2{1.0, 0.0}, circle);

    EXPECT_FALSE(result.has_value());
}

// ============================================================
// Infinite line intersection
// ============================================================

TEST(GeometryAlgorithms, IntersectLines) {
    const Line2 lineA{{0.0, 0.0}, {10.0, 10.0}};
    const Line2 lineB{{0.0, 10.0}, {10.0, 0.0}};

    const auto result = geometry::detail::intersectLines(lineA, lineB);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 5.0, EPS);
    EXPECT_NEAR(result->point.y, 5.0, EPS);
    EXPECT_NEAR(result->t, 0.5, EPS);
    EXPECT_NEAR(result->u, 0.5, EPS);
}

TEST(GeometryAlgorithms, ParallelLines) {
    const Line2 lineA{{0.0, 0.0}, {10.0, 0.0}};
    const Line2 lineB{{0.0, 5.0}, {10.0, 5.0}};

    EXPECT_FALSE(
        geometry::detail::intersectLines(lineA, lineB).has_value());
}

TEST(GeometryAlgorithms, CoincidentLines) {
    const Line2 lineA{{0.0, 0.0}, {10.0, 0.0}};
    const Line2 lineB{{5.0, 0.0}, {20.0, 0.0}};

    EXPECT_FALSE(
        geometry::detail::intersectLines(lineA, lineB).has_value());
}

TEST(GeometryAlgorithms, IntersectLinesRejectsDegenerateLine) {
    const Line2 lineA{{2.0, 2.0}, {2.0, 2.0}};
    const Line2 lineB{{0.0, 0.0}, {10.0, 10.0}};

    EXPECT_FALSE(
        geometry::detail::intersectLines(lineA, lineB).has_value());
}

// ============================================================
// Segment intersection
// ============================================================

TEST(GeometryAlgorithms, IntersectSegments) {
    const Line2 segmentA{{0.0, 0.0}, {10.0, 10.0}};
    const Line2 segmentB{{0.0, 10.0}, {10.0, 0.0}};

    const auto intersection =
        geometry::detail::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(intersection.has_value());
    EXPECT_TRUE(geometry::detail::onSegment(intersection->t));
    EXPECT_TRUE(geometry::detail::onSegment(intersection->u));

    EXPECT_NEAR(intersection->point.x, 5.0, EPS);
    EXPECT_NEAR(intersection->point.y, 5.0, EPS);
}

TEST(GeometryAlgorithms, SegmentIntersectionAtEndpoint) {
    const Line2 segmentA{{0.0, 0.0}, {10.0, 0.0}};
    const Line2 segmentB{{10.0, 0.0}, {10.0, 10.0}};

    const auto intersection =
        geometry::detail::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(intersection.has_value());
    EXPECT_TRUE(geometry::detail::onSegment(intersection->t));
    EXPECT_TRUE(geometry::detail::onSegment(intersection->u));

    EXPECT_NEAR(intersection->point.x, 10.0, EPS);
    EXPECT_NEAR(intersection->point.y, 0.0, EPS);
}

TEST(GeometryAlgorithms, SegmentsDoNotIntersect) {
    const Line2 segmentA{{0.0, 0.0}, {2.0, 0.0}};
    const Line2 segmentB{{5.0, -1.0}, {5.0, 1.0}};

    const auto intersection =
        geometry::detail::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(intersection.has_value());

    EXPECT_FALSE(geometry::detail::onSegment(intersection->t));
    EXPECT_TRUE(geometry::detail::onSegment(intersection->u));
}

TEST(GeometryAlgorithms, ParallelSegmentsDoNotIntersect) {
    const Line2 segmentA{{0.0, 0.0}, {10.0, 0.0}};
    const Line2 segmentB{{0.0, 5.0}, {10.0, 5.0}};

    EXPECT_FALSE(
        geometry::detail::intersectLines(segmentA, segmentB).has_value());
}

// ============================================================
// Extended intersection
// ============================================================

TEST(GeometryAlgorithms, ExtendedIntersection) {
    const Line2 segmentA{{0.0, 0.0}, {1.0, 1.0}};
    const Line2 segmentB{{2.0, 0.0}, {3.0, -1.0}};

    const auto result = geometry::detail::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 1.0, EPS);
    EXPECT_NEAR(result->point.y, 1.0, EPS);

    EXPECT_TRUE(geometry::detail::onSegment(result->t));
    EXPECT_FALSE(geometry::detail::onSegment(result->u));
}

TEST(GeometryAlgorithms, ExtendedIntersectionAfterSegment) {
    const Line2 segmentA{{0.0, 0.0}, {1.0, 1.0}};
    const Line2 segmentB{{3.0, 0.0}, {4.0, -1.0}};

    const auto result = geometry::detail::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 1.5, EPS);
    EXPECT_NEAR(result->point.y, 1.5, EPS);

    EXPECT_FALSE(geometry::detail::onSegment(result->t));
    EXPECT_FALSE(geometry::detail::onSegment(result->u));
}

TEST(GeometryAlgorithms, ExtendedIntersectionBeforeSegment) {
    const Line2 segmentA{{2.0, 2.0}, {4.0, 4.0}};
    const Line2 segmentB{{0.0, 4.0}, {0.0, 0.0}};

    const auto result = geometry::detail::intersectLines(segmentA, segmentB);

    ASSERT_TRUE(result.has_value());

    EXPECT_NEAR(result->point.x, 0.0, EPS);
    EXPECT_NEAR(result->point.y, 0.0, EPS);

    EXPECT_FALSE(geometry::detail::onSegment(result->t));
    EXPECT_TRUE(geometry::detail::onSegment(result->u));
}

TEST(GeometryAlgorithms, NoExtendedIntersectionForParallelLines) {
    const Line2 segmentA{{0.0, 0.0}, {1.0, 0.0}};
    const Line2 segmentB{{0.0, 5.0}, {1.0, 5.0}};

    EXPECT_FALSE(
        geometry::detail::intersectLines(segmentA, segmentB).has_value());
}

} // namespace
