#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <type_traits>

#include "sketch/Sketch.h"

namespace core::sketch {
namespace {

void expectSolved(Sketch& sketch) {
    const auto solved = sketch.solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
}

void expectCenter(const Sketch& sketch, EntityId id, Vec2 expected) {
    const auto center = sketch.pointPosition({id, SubElement::Center});
    ASSERT_TRUE(center);
    EXPECT_NEAR(center.value().x, expected.x, 1e-6);
    EXPECT_NEAR(center.value().y, expected.y, 1e-6);
}

void expectCircularArc(const Sketch& sketch, EntityId id) {
    const auto entity = sketch.entity(id);
    ASSERT_TRUE(entity);
    const auto& arc = std::get<Arc2>(entity.value().geometry);
    const double startRadius = std::hypot(arc.start.x - arc.center.x, arc.start.y - arc.center.y);
    const double endRadius = std::hypot(arc.end.x - arc.center.x, arc.end.y - arc.center.y);
    EXPECT_GT(startRadius, 0);
    EXPECT_GT(endRadius, 0);
    EXPECT_NEAR(startRadius, endRadius, 1e-6);
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

TEST(ConcentricConstraint, SupportsAndSolvesEveryCircleAndArcPairing) {
    for (bool firstArc : {false, true}) {
        for (bool secondArc : {false, true}) {
            Sketch sketch;
            ASSERT_TRUE(sketch.capabilities().supports(ConstraintType::Concentric));
            const auto anchor = sketch.addPoint({2, 3});
            const auto first = firstArc ? sketch.addArc({2, 3}, {6, 3}, {2, 7}) : sketch.addCircle({2, 3}, 4);
            const auto second = secondArc ? sketch.addArc({8, 5}, {10, 5}, {8, 7}) : sketch.addCircle({8, 5}, 2);
            ASSERT_TRUE(anchor);
            ASSERT_TRUE(first);
            ASSERT_TRUE(second);
            ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{anchor.value()}}, std::nullopt, Vec2{2, 3}}));
            ASSERT_TRUE(
                sketch.addConstraint({ConstraintType::Coincident, {{first.value(), SubElement::Center}, {anchor.value()}}, std::nullopt, std::nullopt}));
            const ConstraintDefinition definition{ConstraintType::Concentric, {{first.value()}, {second.value()}}, std::nullopt, std::nullopt};
            ASSERT_TRUE(sketch.supportsConstraint(definition));
            const auto added = sketch.addConstraint(definition);
            ASSERT_TRUE(added);
            const auto queried = sketch.constraint(added.value());
            ASSERT_TRUE(queried);
            EXPECT_EQ(queried.value().definition.type, ConstraintType::Concentric);
            EXPECT_EQ(queried.value().definition.refs, definition.refs);
            expectSolved(sketch);
            expectCenter(sketch, first.value(), {2, 3});
            expectCenter(sketch, second.value(), {2, 3});
            if (firstArc) {
                expectCircularArc(sketch, first.value());
            }
            if (secondArc) {
                expectCircularArc(sketch, second.value());
            }
        }
    }
}

TEST(ConcentricConstraint, RetargetsPreservingIdentityAndReleasesPreviousCenterAliases) {
    Sketch sketch;
    const auto anchor = sketch.addPoint({2, 3});
    const auto first = sketch.addCircle({2, 3}, 4);
    const auto second = sketch.addCircle({8, 5}, 2);
    const auto third = sketch.addCircle({10, -4}, 3);
    ASSERT_TRUE(anchor);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(third);
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{anchor.value()}}, std::nullopt, Vec2{2, 3}}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Coincident, {{first.value(), SubElement::Center}, {anchor.value()}}, std::nullopt, std::nullopt}));
    const auto added = sketch.addConstraint({ConstraintType::Concentric, {{first.value()}, {second.value()}}, std::nullopt, std::nullopt});
    ASSERT_TRUE(added);
    expectSolved(sketch);
    expectCenter(sketch, second.value(), {2, 3});
    const ConstraintDefinition replacement{ConstraintType::Concentric, {{first.value()}, {third.value()}}, std::nullopt, std::nullopt};
    ASSERT_TRUE(sketch.updateConstraint(added.value(), replacement));
    const auto updated = sketch.constraint(added.value());
    ASSERT_TRUE(updated);
    EXPECT_EQ(updated.value().id, added.value());
    EXPECT_EQ(updated.value().definition.refs, replacement.refs);
    ASSERT_TRUE(sketch.updateCircle(second.value(), {{8, 9}, 2}));
    expectSolved(sketch);
    expectCenter(sketch, first.value(), {2, 3});
    expectCenter(sketch, second.value(), {8, 9});
    expectCenter(sketch, third.value(), {2, 3});
    ASSERT_TRUE(sketch.removeConstraint(added.value()));
    expectError(sketch.constraint(added.value()), ErrorCode::NotFound);
    ASSERT_TRUE(sketch.updateCircle(third.value(), {{-7, 6}, 3}));
    expectSolved(sketch);
    expectCenter(sketch, first.value(), {2, 3});
    expectCenter(sketch, third.value(), {-7, 6});
}

TEST(ConcentricConstraint, TransfersSnapshotsAndCascadesWhenEitherCurveIsRemoved) {
    Sketch sketch;
    const auto first = sketch.addEntity(Circle2{{2, 3}, 4}, true);
    const auto second = sketch.addArc({2, 3}, {4, 3}, {2, 5});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const auto added = sketch.addConstraint({ConstraintType::Concentric, {{first.value()}, {second.value()}}, std::nullopt, std::nullopt});
    ASSERT_TRUE(added);
    const auto state = sketch.snapshot();
    ASSERT_TRUE(state);
    for (bool removeFirst : {false, true}) {
        Sketch restored;
        ASSERT_TRUE(restored.replaceState(state.value()));
        const auto transferred = restored.snapshot();
        ASSERT_TRUE(transferred);
        expectSnapshot(transferred.value(), state.value());
        expectSolved(restored);
        expectCenter(restored, first.value(), {2, 3});
        expectCenter(restored, second.value(), {2, 3});
        expectCircularArc(restored, second.value());
        ASSERT_TRUE(restored.removeEntity(removeFirst ? first.value() : second.value()));
        expectError(restored.constraint(added.value()), ErrorCode::NotFound);
        EXPECT_EQ(restored.constraintCount(), 0U);
        EXPECT_EQ(restored.entityCount(), 1U);
        const EntityId remaining = removeFirst ? second.value() : first.value();
        if (removeFirst) {
            ASSERT_TRUE(restored.updateArc(remaining, {{8, 9}, {10, 9}, {8, 11}}));
        } else {
            ASSERT_TRUE(restored.updateCircle(remaining, {{8, 9}, 4}));
        }
        expectSolved(restored);
        expectCenter(restored, remaining, {8, 9});
    }
}

TEST(ConcentricConstraint, RejectsMalformedDefinitionsWithoutMutatingStateOrAllocatingIds) {
    Sketch sketch;
    const auto first = sketch.addCircle({0, 0}, 2);
    const auto second = sketch.addArc({0, 0}, {3, 0}, {0, 3});
    const auto point = sketch.addPoint({1, 1});
    const auto line = sketch.addLine({0, 0}, {4, 4});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(point);
    ASSERT_TRUE(line);
    const GeometryRef a{first.value()}, b{second.value()};
    const ConstraintDefinition valid{ConstraintType::Concentric, {a, b}, std::nullopt, std::nullopt};
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
        return Rejection{{ConstraintType::Concentric, std::move(refs), value, fixed}, code};
    };
    const std::array cases{rejected({}),
                           rejected({a}),
                           rejected({a, b, {point.value()}}),
                           rejected({a, a}),
                           rejected({{first.value(), SubElement::Center}, b}),
                           rejected({a, {second.value(), SubElement::Center}}),
                           rejected({{first.value(), SubElement::Start}, b}),
                           rejected({a, {second.value(), SubElement::Start}}),
                           rejected({{point.value()}, b}),
                           rejected({a, {line.value()}}),
                           rejected({a, b}, 1),
                           rejected({a, b}, std::nullopt, Vec2{0, 0}),
                           rejected({{EntityId(999)}, b}, std::nullopt, std::nullopt, ErrorCode::NotFound),
                           rejected({a, {EntityId(999)}}, std::nullopt, std::nullopt, ErrorCode::NotFound),
                           rejected({a, {EntityId(0)}}),
                           rejected({a, {second.value(), static_cast<SubElement>(999)}})};
    for (const auto& test : cases) {
        expectError(sketch.supportsConstraint(test.definition), test.code);
        expectError(sketch.addConstraint(test.definition), test.code);
        expectError(sketch.updateConstraint(added.value(), test.definition), test.code);
        const auto after = sketch.snapshot();
        ASSERT_TRUE(after);
        expectSnapshot(after.value(), before.value());
    }
    const auto next = sketch.addConstraint(valid);
    ASSERT_TRUE(next);
    EXPECT_EQ(next.value().get(), before.value().lastConstraintId + 1);
}

}  // namespace
}  // namespace core::sketch
