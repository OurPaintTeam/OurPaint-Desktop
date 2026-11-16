#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>

#include "sketch/Sketch.h"

namespace core::sketch {
namespace {

ConstraintDefinition tangent(EntityId first, EntityId second, std::optional<TangencyBranch> branch = std::nullopt) {
    return {ConstraintType::Tangent, {{first}, {second}}, std::nullopt, std::nullopt, branch};
}

Circle2 circleSupport(const SketchGeometry& geometry) {
    if (const auto* circle = std::get_if<Circle2>(&geometry)) {
        return *circle;
    }
    const auto& arc = std::get<Arc2>(geometry);
    return {arc.center, std::hypot(arc.start.x - arc.center.x, arc.start.y - arc.center.y)};
}

SketchGeometry circularGeometry(Vec2 center, double radius, bool arc) {
    if (arc) {
        return Arc2{center, {center.x + radius, center.y}, {center.x, center.y + radius}};
    }
    return Circle2{center, radius};
}

void expectBranch(const Sketch& sketch, ConstraintId id, TangencyBranch expected) {
    const auto constraint = sketch.constraint(id);
    ASSERT_TRUE(constraint);
    EXPECT_EQ(constraint.value().definition.tangencyBranch, expected);
}

void expectConverged(const Result<SolveDiagnostics>& solved) {
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
}

void expectLineTangency(const Sketch& sketch, EntityId lineId, EntityId curveId, TangencyBranch branch) {
    const auto lineEntity = sketch.entity(lineId);
    const auto curveEntity = sketch.entity(curveId);
    ASSERT_TRUE(lineEntity);
    ASSERT_TRUE(curveEntity);
    const auto& line = std::get<Line2>(lineEntity.value().geometry);
    const auto curve = circleSupport(curveEntity.value().geometry);
    const double dx = line.end.x - line.start.x, dy = line.end.y - line.start.y;
    const double length = std::hypot(dx, dy);
    ASSERT_GT(length, 0);
    ASSERT_GT(curve.radius, 0);
    const double signedDistance = (dx * (curve.center.y - line.start.y) - dy * (curve.center.x - line.start.x)) / length;
    EXPECT_NEAR(signedDistance, branch == TangencyBranch::PositiveSide ? curve.radius : -curve.radius, 1e-6);
}

void expectCircularTangency(const Sketch& sketch, EntityId firstId, EntityId secondId, TangencyBranch branch) {
    const auto firstEntity = sketch.entity(firstId);
    const auto secondEntity = sketch.entity(secondId);
    ASSERT_TRUE(firstEntity);
    ASSERT_TRUE(secondEntity);
    const auto first = circleSupport(firstEntity.value().geometry), second = circleSupport(secondEntity.value().geometry);
    ASSERT_GT(first.radius, 0);
    ASSERT_GT(second.radius, 0);
    const double distance = std::hypot(second.center.x - first.center.x, second.center.y - first.center.y);
    const double expected = branch == TangencyBranch::External              ? first.radius + second.radius
                            : branch == TangencyBranch::FirstContainsSecond ? first.radius - second.radius
                                                                            : second.radius - first.radius;
    EXPECT_GE(expected, 0);
    EXPECT_NEAR(distance, expected, 1e-6);
    for (const auto* entity : {&firstEntity.value(), &secondEntity.value()}) {
        if (const auto* arc = std::get_if<Arc2>(&entity->geometry)) {
            EXPECT_NEAR(std::hypot(arc->start.x - arc->center.x, arc->start.y - arc->center.y),
                        std::hypot(arc->end.x - arc->center.x, arc->end.y - arc->center.y), 1e-6);
        }
    }
}

template <class T>
void expectError(const Result<T>& result, ErrorCode code = ErrorCode::InvalidArgument) {
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, code);
}

TEST(TangentConstraint, FullyFixedIncompatibleSupportsReportFailedConvergence) {
    Sketch sketch;
    const auto line = sketch.addLine({-4, 0}, {4, 0});
    const auto circle = sketch.addCircle({0, 2}, 1);
    ASSERT_TRUE(line);
    ASSERT_TRUE(circle);
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{line.value()}}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{circle.value()}}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch.addConstraint(tangent(line.value(), circle.value())));
    const auto solved = sketch.solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Failed);
}

}  // namespace
}  // namespace core::sketch
