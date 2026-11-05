#include <gtest/gtest.h>

#include <array>
#include <type_traits>

#include "sketch/Sketch.h"

namespace core::sketch {
namespace {

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

TEST(SymmetricConstraint, ReflectsAboutFixedHorizontalVerticalAndTiltedLines) {
    struct Case {
        Line2 axis;
        Vec2 expected;
    };
    const std::array cases{Case{{{0, 2}, {4, 2}}, {1, 1}}, Case{{{2, 0}, {2, 4}}, {3, 3}}, Case{{{0, 0}, {4, 4}}, {3, 1}}};
    for (const auto& test : cases) {
        Sketch sketch;
        const auto axis = sketch.addEntity(test.axis);
        const auto first = sketch.addPoint({1, 3});
        const auto second = sketch.addPoint({5, -6});
        ASSERT_TRUE(axis);
        ASSERT_TRUE(first);
        ASSERT_TRUE(second);
        const GeometryRef a{first.value()}, b{second.value()}, line{axis.value()};
        ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {line}, std::nullopt, std::nullopt}));
        ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {a}, std::nullopt, Vec2{1, 3}}));
        const ConstraintDefinition symmetric{ConstraintType::Symmetric, {a, b, line}, std::nullopt, std::nullopt};
        ASSERT_TRUE(sketch.supportsConstraint(symmetric));
        ASSERT_TRUE(sketch.addConstraint(symmetric));
        ASSERT_TRUE(sketch.solve());
        expectPosition(sketch, a, {1, 3});
        expectPosition(sketch, b, test.expected);
        expectPosition(sketch, {axis.value(), SubElement::Start}, test.axis.start);
        expectPosition(sketch, {axis.value(), SubElement::End}, test.axis.end);
    }
}

TEST(SymmetricConstraint, ReflectsCurvePointElementsAboutPointCenter) {
    Sketch sketch;
    const auto first = sketch.addLine({1, 3}, {4, 7});
    const auto second = sketch.addCircle({5, -6}, 2);
    const auto center = sketch.addPoint({2, 2});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(center);
    const GeometryRef a{first.value(), SubElement::Start}, b{second.value(), SubElement::Center}, c{center.value()};
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{first.value()}}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {c}, std::nullopt, Vec2{2, 2}}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Symmetric, {a, b, c}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch.solve());
    expectPosition(sketch, a, {1, 3});
    expectPosition(sketch, {first.value(), SubElement::End}, {4, 7});
    expectPosition(sketch, b, {3, 1});
    expectPosition(sketch, c, {2, 2});
    const auto circle = sketch.entity(second.value());
    ASSERT_TRUE(circle);
    EXPECT_DOUBLE_EQ(std::get<Circle2>(circle.value().geometry).radius, 2);
}

}  // namespace
}  // namespace core::sketch
