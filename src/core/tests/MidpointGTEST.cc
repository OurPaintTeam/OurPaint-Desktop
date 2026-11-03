#include <gtest/gtest.h>

#include <array>
#include <type_traits>

#include "sketch/Sketch.h"

namespace core::sketch {
namespace {

void expectSolved(Sketch& sketch) {
    const auto solved = sketch.solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
}

void expectPosition(const Sketch& sketch, GeometryRef ref, Vec2 expected) {
    const auto actual = sketch.pointPosition(ref);
    ASSERT_TRUE(actual);
    EXPECT_NEAR(actual.value().x, expected.x, 1e-6);
    EXPECT_NEAR(actual.value().y, expected.y, 1e-6);
}

template <class T>
void expectError(const Result<T>& result, ErrorCode code) {
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, code);
}

void expectSnapshot(const SketchSnapshot& actual, const SketchSnapshot& expected) {
    EXPECT_EQ(actual.lastEntityId, expected.lastEntityId);
    EXPECT_EQ(actual.lastConstraintId, expected.lastConstraintId);
    ASSERT_EQ(actual.entities.size(), expected.entities.size());
    for (size_t i = 0; i < actual.entities.size(); ++i) {
        const auto& entity = actual.entities[i];
        const auto& other = expected.entities[i];
        EXPECT_EQ(entity.id, other.id);
        EXPECT_EQ(entity.construction, other.construction);
        ASSERT_EQ(entity.geometry.index(), other.geometry.index());
        std::visit(
            [&](const auto& geometry) {
                using Geometry = std::decay_t<decltype(geometry)>;
                const auto& expectedGeometry = std::get<Geometry>(other.geometry);
                if constexpr (std::is_same_v<Geometry, Point2>) {
                    EXPECT_EQ(geometry.position, expectedGeometry.position);
                } else if constexpr (std::is_same_v<Geometry, Line2>) {
                    EXPECT_EQ(geometry.start, expectedGeometry.start);
                    EXPECT_EQ(geometry.end, expectedGeometry.end);
                } else if constexpr (std::is_same_v<Geometry, Circle2>) {
                    EXPECT_EQ(geometry.center, expectedGeometry.center);
                    EXPECT_EQ(geometry.radius, expectedGeometry.radius);
                } else if constexpr (std::is_same_v<Geometry, Arc2>) {
                    EXPECT_EQ(geometry.center, expectedGeometry.center);
                    EXPECT_EQ(geometry.start, expectedGeometry.start);
                    EXPECT_EQ(geometry.end, expectedGeometry.end);
                } else if constexpr (std::is_same_v<Geometry, NurbsCurve2>) {
                    EXPECT_EQ(geometry.degree, expectedGeometry.degree);
                    EXPECT_EQ(geometry.weights, expectedGeometry.weights);
                    EXPECT_EQ(geometry.knots, expectedGeometry.knots);
                    ASSERT_EQ(geometry.controlPoints.size(), expectedGeometry.controlPoints.size());
                    for (size_t j = 0; j < geometry.controlPoints.size(); ++j) {
                        EXPECT_EQ(geometry.controlPoints[j].position, expectedGeometry.controlPoints[j].position);
                    }
                }
            },
            entity.geometry);
    }
    ASSERT_EQ(actual.constraints.size(), expected.constraints.size());
    for (size_t i = 0; i < actual.constraints.size(); ++i) {
        EXPECT_EQ(actual.constraints[i].id, expected.constraints[i].id);
        const auto& definition = actual.constraints[i].definition;
        const auto& other = expected.constraints[i].definition;
        EXPECT_EQ(definition.type, other.type);
        EXPECT_EQ(definition.refs, other.refs);
        EXPECT_EQ(definition.value, other.value);
        EXPECT_EQ(definition.fixedPosition, other.fixedPosition);
    }
}

TEST(MidpointConstraint, AcceptsLineEndpointsCircleCentersAndAllArcPointElements) {
    struct Case {
        SketchGeometry geometry;
        SubElement sub;
    };
    const std::array cases{Case{Line2{{1, 1}, {7, 9}}, SubElement::Start},      Case{Line2{{1, 1}, {7, 9}}, SubElement::End},
                           Case{Circle2{{1, 1}, 2}, SubElement::Center},        Case{Arc2{{1, 1}, {3, 1}, {1, 3}}, SubElement::Start},
                           Case{Arc2{{1, 1}, {3, 1}, {1, 3}}, SubElement::End}, Case{Arc2{{1, 1}, {3, 1}, {1, 3}}, SubElement::Center}};
    for (const auto& test : cases) {
        Sketch sketch;
        const auto segment = sketch.addLine({0, 0}, {8, 4});
        const auto entity = sketch.addEntity(test.geometry);
        ASSERT_TRUE(segment);
        ASSERT_TRUE(entity);
        ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{segment.value()}}, std::nullopt, std::nullopt}));
        const GeometryRef point{entity.value(), test.sub};
        const ConstraintDefinition definition{ConstraintType::Midpoint, {point, {segment.value()}}, std::nullopt, std::nullopt};
        ASSERT_TRUE(sketch.supportsConstraint(definition));
        ASSERT_TRUE(sketch.addConstraint(definition));
        expectSolved(sketch);
        expectPosition(sketch, point, {4, 2});
    }
}

TEST(MidpointConstraint, AllowsPointReferenceToOwnSegmentEndpointAndRejectsZeroLengthGeometry) {
    Sketch sketch;
    expectError(sketch.addLine({2, 3}, {2, 3}), ErrorCode::InvalidArgument);
    const auto segment = sketch.addLine({0, 0}, {8, 4});
    ASSERT_TRUE(segment);
    EXPECT_EQ(segment.value().get(), 1);
    const ConstraintDefinition definition{ConstraintType::Midpoint, {{segment.value(), SubElement::Start}, {segment.value()}}, std::nullopt, std::nullopt};
    ASSERT_TRUE(sketch.supportsConstraint(definition));
    ASSERT_TRUE(sketch.addConstraint(definition));
}

}  // namespace
}  // namespace core::sketch
