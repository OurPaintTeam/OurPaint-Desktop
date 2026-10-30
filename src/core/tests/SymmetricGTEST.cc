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
        ASSERT_TRUE(sketch.capabilities().supports(ConstraintType::Symmetric));
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

TEST(SymmetricConstraint, RetargetsPreservingIdentityTransfersStateAndRemovesNativeRows) {
    Sketch sketch;
    const auto axis = sketch.addLine({0, 0}, {4, 4});
    const auto first = sketch.addPoint({1, 3});
    const auto second = sketch.addPoint({5, -6});
    const auto center = sketch.addPoint({4, 2});
    ASSERT_TRUE(axis);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(center);
    const GeometryRef a{first.value()}, b{second.value()}, line{axis.value()}, c{center.value()};
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {line}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {a}, std::nullopt, Vec2{1, 3}}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {c}, std::nullopt, Vec2{4, 2}}));
    const auto added = sketch.addConstraint({ConstraintType::Symmetric, {a, b, line}, std::nullopt, std::nullopt});
    ASSERT_TRUE(added);
    ASSERT_TRUE(sketch.solve());
    expectPosition(sketch, b, {3, 1});
    const ConstraintDefinition central{ConstraintType::Symmetric, {a, b, c}, std::nullopt, std::nullopt};
    ASSERT_TRUE(sketch.updateConstraint(added.value(), central));
    const auto updated = sketch.constraint(added.value());
    ASSERT_TRUE(updated);
    EXPECT_EQ(updated.value().id, added.value());
    EXPECT_EQ(updated.value().definition.refs, central.refs);
    ASSERT_TRUE(sketch.solve());
    expectPosition(sketch, b, {7, 1});
    const auto state = sketch.snapshot();
    ASSERT_TRUE(state);
    Sketch restored;
    ASSERT_TRUE(restored.replaceState(state.value()));
    const auto transferred = restored.snapshot();
    ASSERT_TRUE(transferred);
    expectSnapshot(transferred.value(), state.value());
    ASSERT_TRUE(restored.updatePoint(second.value(), {{-3, 8}}));
    ASSERT_TRUE(restored.solve());
    expectPosition(restored, b, {7, 1});
    ASSERT_TRUE(restored.removeConstraint(added.value()));
    expectError(restored.constraint(added.value()), ErrorCode::NotFound);
    ASSERT_TRUE(restored.updatePoint(second.value(), {{11, 12}}));
    ASSERT_TRUE(restored.solve());
    expectPosition(restored, b, {11, 12});
}

TEST(SymmetricConstraint, RemovingAxisOrCenterCascadesConstraintAndRetainsPointEntities) {
    for (bool lineAxis : {true, false}) {
        Sketch sketch;
        const auto first = sketch.addPoint({1, 3});
        const auto second = sketch.addPoint({5, -6});
        const auto third = lineAxis ? sketch.addLine({0, 0}, {4, 4}) : sketch.addPoint({2, 2});
        ASSERT_TRUE(first);
        ASSERT_TRUE(second);
        ASSERT_TRUE(third);
        const GeometryRef a{first.value()}, b{second.value()};
        const auto added = sketch.addConstraint({ConstraintType::Symmetric, {a, b, {third.value()}}, std::nullopt, std::nullopt});
        ASSERT_TRUE(added);
        ASSERT_TRUE(sketch.removeEntity(third.value()));
        expectError(sketch.constraint(added.value()), ErrorCode::NotFound);
        EXPECT_EQ(sketch.constraintCount(), 0U);
        EXPECT_EQ(sketch.entityCount(), 2U);
        expectPosition(sketch, a, {1, 3});
        expectPosition(sketch, b, {5, -6});
        ASSERT_TRUE(sketch.solve());
    }
}

TEST(SymmetricConstraint, RejectsMalformedDefinitionsWithoutChangingStateOrAllocatingIds) {
    Sketch sketch;
    const auto first = sketch.addPoint({1, 3});
    const auto second = sketch.addPoint({5, -6});
    const auto axis = sketch.addLine({0, 0}, {4, 4});
    const auto circle = sketch.addCircle({2, 2}, 2);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(axis);
    ASSERT_TRUE(circle);
    const GeometryRef a{first.value()}, b{second.value()}, line{axis.value()}, wholeCircle{circle.value()};
    const ConstraintDefinition valid{ConstraintType::Symmetric, {a, b, line}, std::nullopt, std::nullopt};
    const auto added = sketch.addConstraint(valid);
    ASSERT_TRUE(added);
    const auto before = sketch.snapshot();
    ASSERT_TRUE(before);
    struct Rejection {
        ConstraintDefinition definition;
        ErrorCode code;
    };
    const auto rejected = [](std::vector<GeometryRef> refs, std::optional<double> value = std::nullopt, std::optional<Vec2> fixed = std::nullopt,
                             ErrorCode code = ErrorCode::InvalidArgument) {
        return Rejection{{ConstraintType::Symmetric, std::move(refs), value, fixed}, code};
    };
    const std::array cases{rejected({a, b}),
                           rejected({a, b, line, wholeCircle}),
                           rejected({a, a, line}),
                           rejected({a, b, a}),
                           rejected({a, b, b}),
                           rejected({line, b, a}),
                           rejected({a, wholeCircle, line}),
                           rejected({a, b, wholeCircle}),
                           rejected({a, b, {axis.value(), SubElement::Center}}),
                           rejected({a, b, {first.value(), SubElement::Start}}),
                           rejected({a, b, line}, 1),
                           rejected({a, b, line}, std::nullopt, Vec2{0, 0}),
                           rejected({a, b, {EntityId(999)}}, std::nullopt, std::nullopt, ErrorCode::NotFound),
                           rejected({{EntityId(999)}, b, line}, std::nullopt, std::nullopt, ErrorCode::NotFound),
                           rejected({a, b, {EntityId(0)}}),
                           rejected({a, b, {axis.value(), static_cast<SubElement>(999)}})};
    for (const auto& test : cases) {
        expectError(sketch.supportsConstraint(test.definition), test.code);
        expectError(sketch.addConstraint(test.definition), test.code);
        expectError(sketch.updateConstraint(added.value(), test.definition), test.code);
        const auto after = sketch.snapshot();
        ASSERT_TRUE(after);
        expectSnapshot(after.value(), before.value());
    }
    const auto next = sketch.addConstraint({ConstraintType::Symmetric, {a, b, {circle.value(), SubElement::Center}}, std::nullopt, std::nullopt});
    ASSERT_TRUE(next);
    EXPECT_EQ(next.value().get(), before.value().lastConstraintId + 1);
}

}  // namespace
}  // namespace core::sketch
