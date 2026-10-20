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
                } else {
                    EXPECT_EQ(geometry.center, expectedGeometry.center);
                    EXPECT_EQ(geometry.start, expectedGeometry.start);
                    EXPECT_EQ(geometry.end, expectedGeometry.end);
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

TEST(MidpointConstraint, PlacesPointAtCenterOfFixedHorizontalVerticalAndTiltedSegments) {
    const std::array lines{Line2{{-2, 2}, {6, 2}}, Line2{{2, -2}, {2, 6}}, Line2{{-2, -1}, {6, 5}}};
    for (const auto& line : lines) {
        Sketch sketch;
        ASSERT_TRUE(sketch.capabilities().supports(ConstraintType::Midpoint));
        const auto segment = sketch.addEntity(line);
        const auto point = sketch.addPoint({9, -4});
        ASSERT_TRUE(segment);
        ASSERT_TRUE(point);
        const GeometryRef p{point.value()}, l{segment.value()};
        ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {l}, std::nullopt, std::nullopt}));
        const ConstraintDefinition definition{ConstraintType::Midpoint, {p, l}, std::nullopt, std::nullopt};
        ASSERT_TRUE(sketch.supportsConstraint(definition));
        const auto added = sketch.addConstraint(definition);
        ASSERT_TRUE(added);
        const auto queried = sketch.constraint(added.value());
        ASSERT_TRUE(queried);
        EXPECT_EQ(queried.value().definition.type, ConstraintType::Midpoint);
        EXPECT_EQ(queried.value().definition.refs, definition.refs);
        expectSolved(sketch);
        expectPosition(sketch, p, {(line.start.x + line.end.x) / 2, (line.start.y + line.end.y) / 2});
        expectPosition(sketch, {segment.value(), SubElement::Start}, line.start);
        expectPosition(sketch, {segment.value(), SubElement::End}, line.end);
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

TEST(MidpointConstraint, FollowsSegmentWhenItsFixedEndpointTargetsMove) {
    Sketch sketch;
    const auto segment = sketch.addLine({0, 0}, {8, 4});
    const auto point = sketch.addPoint({3, 1});
    ASSERT_TRUE(segment);
    ASSERT_TRUE(point);
    const GeometryRef start{segment.value(), SubElement::Start}, end{segment.value(), SubElement::End}, p{point.value()};
    const auto fixedStart = sketch.addConstraint({ConstraintType::Fix, {start}, std::nullopt, Vec2{0, 0}});
    const auto fixedEnd = sketch.addConstraint({ConstraintType::Fix, {end}, std::nullopt, Vec2{8, 4}});
    ASSERT_TRUE(fixedStart);
    ASSERT_TRUE(fixedEnd);
    const auto added = sketch.addConstraint({ConstraintType::Midpoint, {p, {segment.value()}}, std::nullopt, std::nullopt});
    ASSERT_TRUE(added);
    expectSolved(sketch);
    expectPosition(sketch, p, {4, 2});
    ASSERT_TRUE(sketch.updateConstraint(fixedStart.value(), {ConstraintType::Fix, {start}, std::nullopt, Vec2{4, 2}}));
    ASSERT_TRUE(sketch.updateConstraint(fixedEnd.value(), {ConstraintType::Fix, {end}, std::nullopt, Vec2{12, 6}}));
    expectSolved(sketch);
    expectPosition(sketch, p, {8, 4});
    const auto queried = sketch.constraint(added.value());
    ASSERT_TRUE(queried);
    EXPECT_EQ(queried.value().id, added.value());
}

TEST(MidpointConstraint, RetargetsTransfersSnapshotsAndRemovesEquationPreservingPublicIdentities) {
    Sketch sketch;
    const auto first = sketch.addLine({0, 0}, {8, 4});
    const auto second = sketch.addEntity(Line2{{2, 4}, {10, 8}}, true);
    const auto point = sketch.addPoint({1, 1});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(point);
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{first.value()}}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{second.value()}}, std::nullopt, std::nullopt}));
    const GeometryRef p{point.value()};
    const auto added = sketch.addConstraint({ConstraintType::Midpoint, {p, {first.value()}}, std::nullopt, std::nullopt});
    ASSERT_TRUE(added);
    expectSolved(sketch);
    expectPosition(sketch, p, {4, 2});
    const ConstraintDefinition replacement{ConstraintType::Midpoint, {p, {second.value()}}, std::nullopt, std::nullopt};
    ASSERT_TRUE(sketch.updateConstraint(added.value(), replacement));
    const auto updated = sketch.constraint(added.value());
    ASSERT_TRUE(updated);
    EXPECT_EQ(updated.value().id, added.value());
    EXPECT_EQ(updated.value().definition.refs, replacement.refs);
    expectSolved(sketch);
    expectPosition(sketch, p, {6, 6});
    const auto state = sketch.snapshot();
    ASSERT_TRUE(state);
    Sketch restored;
    ASSERT_TRUE(restored.replaceState(state.value()));
    const auto transferred = restored.snapshot();
    ASSERT_TRUE(transferred);
    expectSnapshot(transferred.value(), state.value());
    ASSERT_TRUE(restored.updatePoint(point.value(), {{-3, 9}}));
    expectSolved(restored);
    expectPosition(restored, p, {6, 6});
    ASSERT_TRUE(restored.removeConstraint(added.value()));
    expectError(restored.constraint(added.value()), ErrorCode::NotFound);
    ASSERT_TRUE(restored.updatePoint(point.value(), {{11, 12}}));
    expectSolved(restored);
    expectPosition(restored, p, {11, 12});
}

TEST(MidpointConstraint, RemovingEitherReferencedEntityCascadesConstraint) {
    for (bool removeSegment : {false, true}) {
        Sketch sketch;
        const auto segment = sketch.addLine({0, 0}, {8, 4});
        const auto point = sketch.addPoint({4, 2});
        ASSERT_TRUE(segment);
        ASSERT_TRUE(point);
        const auto added = sketch.addConstraint({ConstraintType::Midpoint, {{point.value()}, {segment.value()}}, std::nullopt, std::nullopt});
        ASSERT_TRUE(added);
        ASSERT_TRUE(sketch.removeEntity(removeSegment ? segment.value() : point.value()));
        expectError(sketch.constraint(added.value()), ErrorCode::NotFound);
        EXPECT_EQ(sketch.constraintCount(), 0U);
        EXPECT_EQ(sketch.entityCount(), 1U);
        expectSolved(sketch);
    }
}

TEST(MidpointConstraint, RejectsMalformedDefinitionsWithoutChangingStateOrAllocatingIds) {
    Sketch sketch;
    const auto point = sketch.addPoint({4, 2});
    const auto segment = sketch.addLine({0, 0}, {8, 4});
    const auto circle = sketch.addCircle({2, 2}, 2);
    ASSERT_TRUE(point);
    ASSERT_TRUE(segment);
    ASSERT_TRUE(circle);
    const GeometryRef p{point.value()}, l{segment.value()}, c{circle.value()};
    const ConstraintDefinition valid{ConstraintType::Midpoint, {p, l}, std::nullopt, std::nullopt};
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
        return Rejection{{ConstraintType::Midpoint, std::move(refs), value, fixed}, code};
    };
    const std::array cases{rejected({}),
                           rejected({p}),
                           rejected({p, l, c}),
                           rejected({p, p}),
                           rejected({l, p}),
                           rejected({c, l}),
                           rejected({p, c}),
                           rejected({p, {segment.value(), SubElement::Start}}),
                           rejected({{point.value(), SubElement::Center}, l}),
                           rejected({{segment.value(), SubElement::Center}, l}),
                           rejected({p, l}, 1),
                           rejected({p, l}, std::nullopt, Vec2{0, 0}),
                           rejected({{EntityId(999)}, l}, std::nullopt, std::nullopt, ErrorCode::NotFound),
                           rejected({p, {EntityId(999)}}, std::nullopt, std::nullopt, ErrorCode::NotFound),
                           rejected({p, {EntityId(0)}}),
                           rejected({p, {segment.value(), static_cast<SubElement>(999)}})};
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
