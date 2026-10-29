#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <vector>

#include "geometry/GeometryMath.h"
#include "geometry/LineAlgorithms.h"
#include "geometry/CircleAlgorithms.h"
#include "geometry/ArcAlgorithms.h"
#include "geometry/relations/AxisRelations.h"
#include "geometry/relations/Parallelism.h"
#include "geometry/relations/Perpendicularity.h"
#include "geometry/relations/PointCurveTangency.h"
#include "geometry/intersections/IntersectionDispatcher.h"
#include "geometry/relations/Tangency.h"

namespace {
using namespace geometry;
constexpr double kTol = 1e-8;

void expectVec(const Vec2& actual, double x, double y, double tol = kTol) {
    EXPECT_NEAR(actual.x, x, tol);
    EXPECT_NEAR(actual.y, y, tol);
}
Vec2 pos(const Point2& p) { return p.position; }

TEST(GeometryMath, DistanceAndVectorOperations) {
    EXPECT_DOUBLE_EQ(distance(Vec2{0, 0}, Vec2{3, 4}), 5.0);
    expectVec(detail::add({1, 2}, {3, -5}), 4, -3);
    expectVec(detail::subtract({1, 2}, {3, -5}), -2, 7);
    expectVec(detail::multiply({2, -3}, 2.5), 5, -7.5);
    EXPECT_DOUBLE_EQ(detail::dot({1, 2}, {3, 4}), 11.0);
    EXPECT_DOUBLE_EQ(detail::cross({1, 2}, {3, 4}), -2.0);
}
TEST(GeometryMath, MidpointAndValidityHelpers) {
    expectVec(detail::midpoint(Line2{{-2, 4}, {6, 8}}), 2, 6);
    EXPECT_TRUE(detail::isFinite(Vec2{1, 2}));
    EXPECT_FALSE(detail::isFinite(Vec2{NAN, 2}));
    EXPECT_TRUE(detail::isFinite(Line2{{0, 0}, {1, 1}}));
    EXPECT_FALSE(detail::isFinite(Line2{{0, 0}, {INFINITY, 1}}));
    EXPECT_TRUE(detail::isFinite(Circle2{{0, 0}, 2}));
    EXPECT_FALSE(detail::isFinite(Circle2{{0, 0}, -1}));
    EXPECT_TRUE(detail::isDegenerate(Line2{{1, 1}, {1, 1}}));
    EXPECT_FALSE(detail::isDegenerate(Line2{{0, 0}, {1, 0}}));
    EXPECT_TRUE(detail::onSegment(0.0));
    EXPECT_TRUE(detail::onSegment(1.0));
    EXPECT_FALSE(detail::onSegment(1.1));
}

TEST(LineAlgorithms, ClosestPointProjectsAndClamps) {
    auto p = detail::closestPointOnSegment({2, 3}, Line2{{0, 0}, {4, 0}});
    ASSERT_TRUE(p); expectVec(*p, 2, 0);
    p = detail::closestPointOnSegment({-3, 2}, Line2{{0, 0}, {4, 0}});
    ASSERT_TRUE(p); expectVec(*p, 0, 0);
    p = detail::closestPointOnSegment({9, 2}, Line2{{0, 0}, {4, 0}});
    ASSERT_TRUE(p); expectVec(*p, 4, 0);
}
TEST(LineAlgorithms, ClosestPointHandlesDegenerateAndNonFinite) {
    auto p = detail::closestPointOnSegment({5, 5}, Line2{{1, 2}, {1, 2}});
    ASSERT_TRUE(p); expectVec(*p, 1, 2);
    EXPECT_FALSE(detail::closestPointOnSegment({NAN, 0}, Line2{{0, 0}, {1, 0}}));
}
TEST(LineAlgorithms, InfiniteLineIntersectionReturnsParameters) {
    auto hit = detail::intersectLines(Line2{{0, 0}, {4, 0}}, Line2{{2, -1}, {2, 1}});
    ASSERT_TRUE(hit); expectVec(hit->point, 2, 0);
    EXPECT_NEAR(hit->t, 0.5, kTol);
    EXPECT_NEAR(hit->u, 0.5, kTol);
    EXPECT_FALSE(detail::intersectLines(Line2{{0, 0}, {4, 0}}, Line2{{0, 1}, {4, 1}}));
    EXPECT_FALSE(detail::intersectLines(Line2{{0, 0}, {0, 0}}, Line2{{0, -1}, {0, 1}}));
}

TEST(CircleAlgorithms, ClosestPointOnCircle) {
    auto p = detail::closestPointOnCircle({3, 4}, Circle2{{0, 0}, 2});
    ASSERT_TRUE(p); expectVec(*p, 1.2, 1.6);
    p = detail::closestPointOnCircle({0, 0}, Circle2{{0, 0}, 2});
    ASSERT_TRUE(p); expectVec(*p, 2, 0);
    p = detail::closestPointOnCircle({3, 4}, Circle2{{0, 0}, 0});
    ASSERT_TRUE(p); expectVec(*p, 0, 0);
    EXPECT_FALSE(detail::closestPointOnCircle({NAN, 0}, Circle2{{0, 0}, 1}));
}

TEST(ArcAlgorithms, NormalizeAngleAndSweep) {
    EXPECT_NEAR(detail::normalizeAngle(-kTol), 2 * std::acos(-1.0) - kTol, 1e-7);
    EXPECT_NEAR(detail::normalizeAngle(2 * std::acos(-1.0) + 0.5), 0.5, kTol);
    Arc2 arc{{0, 0}, {1, 0}, {0, 1}};
    EXPECT_NEAR(detail::arcSweep(arc), std::acos(-1.0) / 2, kTol);
}
TEST(ArcAlgorithms, MidpointOnCounterClockwiseSweep) {
    auto p = detail::arcMidpoint(Arc2{{0, 0}, {2, 0}, {0, 2}});
    ASSERT_TRUE(p); expectVec(*p, std::sqrt(2.0), std::sqrt(2.0));
}
TEST(ArcAlgorithms, MidpointRejectsInvalidRadiusAndNonFinite) {
    EXPECT_FALSE(detail::arcMidpoint(Arc2{{0, 0}, {1, 0}, {0, 2}}));
    EXPECT_FALSE(detail::arcMidpoint(Arc2{{0, 0}, {0, 0}, {1, 0}}));
    EXPECT_FALSE(detail::arcMidpoint(Arc2{{0, 0}, {NAN, 0}, {1, 0}}));
}
TEST(ArcAlgorithms, ClosestPointUsesArcInteriorOrNearestEndpoint) {
    Arc2 arc{{0, 0}, {2, 0}, {0, 2}};
    auto p = detail::closestPointOnArc({1, 1}, arc);
    ASSERT_TRUE(p); expectVec(*p, std::sqrt(2.0), std::sqrt(2.0));
    p = detail::closestPointOnArc({-3, 0}, arc);
    ASSERT_TRUE(p); expectVec(*p, 0, 2);
    p = detail::closestPointOnArc({0, 0}, arc);
    ASSERT_TRUE(p); expectVec(*p, 2, 0);
    EXPECT_FALSE(detail::closestPointOnArc({0, 0}, Arc2{{0, 0}, {1, 0}, {1, 0}}));
}

TEST(LineLineIntersection, CrossesWithinSegments) {
    auto pts = intersections::lineLine(Line2{{0, 0}, {4, 4}}, Line2{{0, 4}, {4, 0}});
    ASSERT_EQ(pts.size(), 1u); expectVec(pos(pts[0]), 2, 2);
}
TEST(LineLineIntersection, RespectsSegmentModeAndEndpoints) {
    Line2 a{{0, 0}, {1, 0}}, b{{2, -1}, {2, 1}};
    EXPECT_TRUE(intersections::lineLine(a, b, true).empty());
    auto pts = intersections::lineLine(a, b, false);
    ASSERT_EQ(pts.size(), 1u); expectVec(pos(pts[0]), 2, 0);
    pts = intersections::lineLine(Line2{{0, 0}, {1, 0}}, Line2{{1, 0}, {1, 1}});
    ASSERT_EQ(pts.size(), 1u); expectVec(pos(pts[0]), 1, 0);
}
TEST(LineLineIntersection, ParallelCoincidentAndDegenerateHaveNoUniquePoint) {
    EXPECT_TRUE(intersections::lineLine(Line2{{0, 0}, {2, 0}}, Line2{{0, 1}, {2, 1}}).empty());
    EXPECT_TRUE(intersections::lineLine(Line2{{0, 0}, {2, 0}}, Line2{{1, 0}, {3, 0}}).empty());
    EXPECT_TRUE(intersections::lineLine(Line2{{0, 0}, {0, 0}}, Line2{{0, -1}, {0, 1}}).empty());
}
TEST(LineLineIntersection, RejectsNonFiniteInput) {
    EXPECT_TRUE(intersections::lineLine(Line2{{NAN, 0}, {1, 1}}, Line2{{0, 0}, {1, 0}}).empty());
}

TEST(LineCircleIntersection, SecantTangentAndMiss) {
    auto pts = intersections::lineCircle(Line2{{-2, 0}, {2, 0}}, Circle2{{0, 0}, 1});
    ASSERT_EQ(pts.size(), 2u);
    expectVec(pos(pts[0]), -1, 0); expectVec(pos(pts[1]), 1, 0);
    pts = intersections::lineCircle(Line2{{-2, 1}, {2, 1}}, Circle2{{0, 0}, 1});
    ASSERT_EQ(pts.size(), 1u); expectVec(pos(pts[0]), 0, 1);
    EXPECT_TRUE(intersections::lineCircle(Line2{{-2, 2}, {2, 2}}, Circle2{{0, 0}, 1}).empty());
}
TEST(LineCircleIntersection, SegmentModeAndInvalidInput) {
    EXPECT_TRUE(intersections::lineCircle(Line2{{2, 0}, {3, 0}}, Circle2{{0, 0}, 1}, true).empty());
    EXPECT_EQ(intersections::lineCircle(Line2{{2, 0}, {3, 0}}, Circle2{{0, 0}, 1}, false).size(), 2u);
    EXPECT_TRUE(intersections::lineCircle(Line2{{0, 0}, {0, 0}}, Circle2{{0, 0}, 1}).empty());
    EXPECT_TRUE(intersections::lineCircle(Line2{{-2, 0}, {2, 0}}, Circle2{{0, 0}, -1}).empty());
}

TEST(CircleCircleIntersection, TwoPointsTangentAndSeparate) {
    auto pts = intersections::circleCircle(Circle2{{0, 0}, 2}, Circle2{{2, 0}, 2});
    ASSERT_EQ(pts.size(), 2u);
    expectVec(pos(pts[0]), 1, std::sqrt(3.0)); expectVec(pos(pts[1]), 1, -std::sqrt(3.0));
    pts = intersections::circleCircle(Circle2{{0, 0}, 1}, Circle2{{2, 0}, 1});
    ASSERT_EQ(pts.size(), 1u); expectVec(pos(pts[0]), 1, 0);
    EXPECT_TRUE(intersections::circleCircle(Circle2{{0, 0}, 1}, Circle2{{3, 0}, 1}).empty());
}
TEST(CircleCircleIntersection, ConcentricAndContainedWithoutContact) {
    EXPECT_TRUE(intersections::circleCircle(Circle2{{0, 0}, 2}, Circle2{{0, 0}, 1}).empty());
    EXPECT_TRUE(intersections::circleCircle(Circle2{{0, 0}, 3}, Circle2{{1, 0}, 1}).empty());
}
TEST(CircleCircleIntersection, RejectsNonFiniteAndNegativeRadius) {
    EXPECT_TRUE(intersections::circleCircle(Circle2{{NAN, 0}, 1}, Circle2{{0, 0}, 1}).empty());
    EXPECT_TRUE(intersections::circleCircle(Circle2{{0, 0}, -1}, Circle2{{1, 0}, 1}).empty());
}

TEST(ArcIntersection, PointOnArcChecksSweepAndRadius) {
    Arc2 arc{{0, 0}, {1, 0}, {0, 1}};
    EXPECT_TRUE(intersections::pointOnArc({std::sqrt(0.5), std::sqrt(0.5)}, arc));
    EXPECT_TRUE(intersections::pointOnArc({1, 0}, arc));
    EXPECT_FALSE(intersections::pointOnArc({-1, 0}, arc));
    EXPECT_FALSE(intersections::pointOnArc({0.5, 0.5}, arc));
}
TEST(ArcIntersection, LineArcFiltersSupportingCircleIntersections) {
    Arc2 arc{{0, 0}, {1, 0}, {0, 1}};
    auto pts = intersections::lineArc(Line2{{-2, 0}, {2, 0}}, arc);
    ASSERT_EQ(pts.size(), 1u); expectVec(pos(pts[0]), 1, 0);
}
TEST(ArcIntersection, CircleArcAndArcArc) {
    Arc2 quarter{{0, 0}, {2, 0}, {0, 2}};
    auto pts = intersections::circleArc(
        Circle2{{2, 0}, 2},
        quarter
    );
    ASSERT_EQ(pts.size(), 1u);
    expectVec(pos(pts[0]), 1, std::sqrt(3.0));
    Arc2 other{{2, 0}, {2, 2}, {0, 0}};
    pts = intersections::arcArc(quarter, other);
    ASSERT_EQ(pts.size(), 1u);
    expectVec(pos(pts[0]), 1, std::sqrt(3.0));
}
TEST(IntersectionDispatcher, DispatchesSymmetricPairsAndVariants) {
    Line2 line{{-2, 0}, {2, 0}};
    Circle2 circle{{0, 0}, 1};
    EXPECT_EQ(intersections::intersect(line, circle).size(), 2u);
    EXPECT_EQ(intersections::intersect(circle, line).size(), 2u);
    EXPECT_EQ(intersections::intersect(line, line, IntersectionOptions{false}).size(), 0u);
    SketchEntity a{core::sketch::EntityId{1}, line, false};
    SketchEntity b{core::sketch::EntityId{2}, circle, false};
    EXPECT_EQ(intersections::intersectGeometries(a, b).size(), 2u);
}

TEST(AxisRelations, DetectsAxesRegardlessOfSign) {
    EXPECT_TRUE(relations::isNearHorizontal({-1, 0.1}));
    EXPECT_TRUE(relations::isNearVertical({0.1, -1}));
    EXPECT_FALSE(relations::isNearHorizontal({1, 1}));
    EXPECT_FALSE(relations::isNearVertical({1, 1}));
    EXPECT_FALSE(relations::isNearHorizontal({0, 0}));
    EXPECT_FALSE(relations::isNearVertical({1, 0}, -1));
}
TEST(AxisRelations, SnapsDirectionPreservingLengthAndSign) {
    auto v = relations::snapDirectionToAxis({-3, 0.1}, 5.0);
    ASSERT_TRUE(v); EXPECT_NEAR(std::hypot(v->x, v->y), std::hypot(3.0, 0.1), kTol); EXPECT_LT(v->x, 0); EXPECT_DOUBLE_EQ(v->y, 0);
    v = relations::snapDirectionToAxis({0.1, -3}, 5.0);
    ASSERT_TRUE(v); EXPECT_DOUBLE_EQ(v->x, 0); EXPECT_LT(v->y, 0);
    EXPECT_FALSE(relations::snapDirectionToAxis({1, 1}, 5.0));
    EXPECT_FALSE(relations::snapDirectionToAxis({0, 0}));
}
TEST(Parallelism, ParallelAntiParallelAndTolerance) {
    EXPECT_TRUE(relations::areParallel({1, 0}, {-4, 0}));
    EXPECT_TRUE(relations::areParallel({1, 1}, {2, 2}));
    EXPECT_FALSE(relations::areParallel({1, 0}, {0, 1}));
    EXPECT_FALSE(relations::areParallel({0, 0}, {1, 0}));
    EXPECT_TRUE(relations::areParallelDegrees({1, 0}, {1, 0.001}, 1.0));
}
TEST(Perpendicularity, OrthogonalAndTolerance) {
    EXPECT_TRUE(relations::arePerpendicular({1, 0}, {0, -5}));
    EXPECT_FALSE(relations::arePerpendicular({1, 0}, {1, 1}));
    EXPECT_FALSE(relations::arePerpendicular({0, 0}, {0, 1}));
    EXPECT_TRUE(relations::arePerpendicularDegrees({1, 0}, {0.001, 1}, 1.0));
}

TEST(PointCurveTangency, OutsidePointProducesTwoCircleTangencies) {
    auto candidates = relations::pointTangenciesToCircle({-3, 0}, {0, 0}, {0, 0}, 1);
    ASSERT_EQ(candidates.size(), 2u);
    for (const auto& c : candidates) {
        EXPECT_NEAR(distance(c.point, Vec2{0, 0}), 1.0, kTol);
        EXPECT_NEAR(detail::dot(detail::subtract(c.point, Vec2{-3, 0}),
                                detail::subtract(c.point, Vec2{0, 0})), 0.0, kTol);
    }
}
TEST(PointCurveTangency, OnOrInsideCircleHasNoTangent) {
    EXPECT_TRUE(relations::pointTangenciesToCircle({0, 0}, {0, 0}, {0, 0}, 1).empty());
    EXPECT_TRUE(relations::pointTangenciesToCircle({0.5, 0}, {0, 0}, {0, 0}, 1).empty());
    EXPECT_TRUE(relations::pointTangenciesToCircle({2, 0}, {0, 0}, {0, 0}, 0).empty());
}
TEST(PointCurveTangency, ArcTangenciesAreFilteredToSweep) {
    Arc2 arc{{0, 0}, {1, 0}, {0, 1}};
    auto candidates = relations::pointTangenciesToArc({-3, 0}, {0, 0}, arc);
    for (const auto& c : candidates) EXPECT_TRUE(relations::pointOnArcSweep(c.point, arc));
}

TEST(Tangency, FindsLineCircleTangency) {
    SketchEntity line{core::sketch::EntityId{1}, Line2{{-2, 1}, {2, 1}}, false};
    SketchEntity circle{core::sketch::EntityId{2}, Circle2{{0, 0}, 1}, false};
    std::vector<SketchEntity> entities{line, circle};
    auto tangencies = relations::findTangencies(entities);
    ASSERT_EQ(tangencies.size(), 1u); expectVec(tangencies[0].point, 0, 1);
}
TEST(Tangency, DoesNotTreatSecantAsTangent) {
    SketchEntity line{core::sketch::EntityId{1}, Line2{{-2, 0}, {2, 0}}, false};
    SketchEntity circle{core::sketch::EntityId{2}, Circle2{{0, 0}, 1}, false};
    std::vector<SketchEntity> entities{line, circle};
    EXPECT_TRUE(relations::findTangencies(entities).empty());
}
}  // namespace
