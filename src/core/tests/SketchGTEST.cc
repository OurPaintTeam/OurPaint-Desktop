#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <type_traits>

#include "sketch/Sketch.h"

namespace core::sketch {
namespace {

static_assert(!std::is_same_v<EntityId, ConstraintId>);
static_assert(!std::is_convertible_v<EntityId, ConstraintId>);

template <class T>
void expectError(const Result<T>& result, ErrorCode code) {
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, code);
    EXPECT_FALSE(result.error().message.empty());
}

void expectPosition(Vec2 actual, Vec2 expected, double tolerance = 1e-6) {
    EXPECT_NEAR(actual.x, expected.x, tolerance);
    EXPECT_NEAR(actual.y, expected.y, tolerance);
}

void expectGeometry(const SketchGeometry& actual, const SketchGeometry& expected) {
    ASSERT_EQ(actual.index(), expected.index());
    std::visit(
        [&](const auto& value) {
            using Geometry = std::decay_t<decltype(value)>;
            const auto& other = std::get<Geometry>(expected);
            if constexpr (std::is_same_v<Geometry, Point2>) {
                expectPosition(value.position, other.position);
            } else if constexpr (std::is_same_v<Geometry, Line2>) {
                expectPosition(value.start, other.start);
                expectPosition(value.end, other.end);
            } else if constexpr (std::is_same_v<Geometry, Circle2>) {
                expectPosition(value.center, other.center);
                EXPECT_NEAR(value.radius, other.radius, 1e-6);
            } else {
                expectPosition(value.center, other.center);
                expectPosition(value.start, other.start);
                expectPosition(value.end, other.end);
            }
        },
        actual);
}

ConstraintDefinition horizontal(EntityId line) {
    return {
        ConstraintType::Horizontal,
        {{line, SubElement::Whole}},
        std::nullopt,
        std::nullopt};
}

class SketchBehavior : public testing::TestWithParam<BackendKind> {
protected:
    void SetUp() override {
        auto created = Sketch::create(GetParam());
        ASSERT_TRUE(created);
        sketch_ = std::move(created.value());
    }

    std::unique_ptr<Sketch> sketch_;
};

TEST_P(SketchBehavior, CreatesQueriesAndRemovesAllBasicEntities) {
    const std::vector<SketchGeometry> geometry = {Point2{{1, 2}}, Line2{{0, 0}, {3, 4}}, Circle2{{2, 3}, 5}, Arc2{{0, 0}, {2, 0}, {0, 2}}};
    std::vector<EntityId> ids;
    for (const auto& item : geometry) {
        ASSERT_TRUE(sketch_->capabilities().supports(entityKind(item)));
        auto added = sketch_->addEntity(item, true);
        ASSERT_TRUE(added);
        EXPECT_GT(added.value().get(), 0);
        ids.push_back(added.value());
        auto queried = sketch_->entity(added.value());
        ASSERT_TRUE(queried);
        EXPECT_EQ(queried.value().id, added.value());
        EXPECT_TRUE(queried.value().construction);
        expectGeometry(queried.value().geometry, item);
    }
    auto all = sketch_->entities();
    ASSERT_TRUE(all);
    EXPECT_EQ(all.value().size(), geometry.size());
    for (EntityId id : ids) {
        ASSERT_TRUE(sketch_->removeEntity(id));
        expectError(sketch_->entity(id), ErrorCode::NotFound);
        expectError(sketch_->removeEntity(id), ErrorCode::NotFound);
    }
    all = sketch_->entities();
    ASSERT_TRUE(all);
    EXPECT_TRUE(all.value().empty());
}

TEST_P(SketchBehavior, UpdatesBasicGeometryWithoutChangingIdentityOrDetachedQueries) {
    const std::vector<SketchGeometry> before = {Point2{{1, 2}}, Line2{{0, 0}, {3, 4}}, Circle2{{2, 3}, 5}, Arc2{{0, 0}, {2, 0}, {0, 2}}};
    const std::vector<SketchGeometry> after = {Point2{{5, 6}}, Line2{{1, 2}, {4, 6}}, Circle2{{7, 8}, 3}, Arc2{{1, 1}, {4, 1}, {1, 4}}};
    for (size_t i = 0; i < before.size(); ++i) {
        auto added = sketch_->addEntity(before[i]);
        ASSERT_TRUE(added);
        const EntityId id = added.value();
        auto detached = sketch_->entity(id);
        ASSERT_TRUE(detached);
        ASSERT_TRUE(sketch_->updateEntity(id, after[i], true));
        auto updated = sketch_->entity(id);
        ASSERT_TRUE(updated);
        EXPECT_EQ(updated.value().id, id);
        EXPECT_TRUE(updated.value().construction);
        expectGeometry(updated.value().geometry, after[i]);
        expectGeometry(detached.value().geometry, before[i]);
        EXPECT_FALSE(detached.value().construction);
    }
}

TEST_P(SketchBehavior, CreatesUpdatesQueriesAndRemovesConstraints) {
    auto line = sketch_->addLine({0, 0}, {4, 3});
    ASSERT_TRUE(line);
    auto definition = horizontal(line.value());
    ASSERT_TRUE(sketch_->supportsConstraint(definition));
    auto added = sketch_->addConstraint(definition);
    ASSERT_TRUE(added);
    const ConstraintId id = added.value();
    auto queried = sketch_->constraint(id);
    ASSERT_TRUE(queried);
    EXPECT_EQ(queried.value().id, id);
    EXPECT_EQ(queried.value().definition.type, ConstraintType::Horizontal);
    EXPECT_EQ(queried.value().definition.refs, definition.refs);
    definition.type = ConstraintType::Vertical;
    ASSERT_TRUE(sketch_->updateConstraint(id, definition));
    queried = sketch_->constraint(id);
    ASSERT_TRUE(queried);
    EXPECT_EQ(queried.value().id, id);
    EXPECT_EQ(queried.value().definition.type, ConstraintType::Vertical);
    auto all = sketch_->constraints();
    ASSERT_TRUE(all);
    ASSERT_EQ(all.value().size(), 1U);
    EXPECT_EQ(all.value()[0].id, id);
    ASSERT_TRUE(sketch_->removeConstraint(id));
    expectError(sketch_->constraint(id), ErrorCode::NotFound);
    expectError(sketch_->removeConstraint(id), ErrorCode::NotFound);
    all = sketch_->constraints();
    ASSERT_TRUE(all);
    EXPECT_TRUE(all.value().empty());
    EXPECT_TRUE(sketch_->entity(line.value()));
}

TEST_P(SketchBehavior, RemovingEntityCascadesConstraintsOnItsSubElements) {
    auto line = sketch_->addLine({0, 0}, {4, 0});
    auto point = sketch_->addPoint({4, 0});
    ASSERT_TRUE(line);
    ASSERT_TRUE(point);
    ConstraintDefinition coincident{
        ConstraintType::Coincident, {{line.value(), SubElement::End}, {point.value(), SubElement::Whole}}, std::nullopt, std::nullopt};
    auto relation = sketch_->addConstraint(coincident);
    auto horizontalRelation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(relation);
    ASSERT_TRUE(horizontalRelation);
    ASSERT_TRUE(sketch_->removeEntity(line.value()));
    EXPECT_TRUE(sketch_->entity(point.value()));
    expectError(sketch_->constraint(relation.value()), ErrorCode::NotFound);
    expectError(sketch_->constraint(horizontalRelation.value()), ErrorCode::NotFound);
    auto remaining = sketch_->entities();
    ASSERT_TRUE(remaining);
    ASSERT_EQ(remaining.value().size(), 1U);
    EXPECT_EQ(remaining.value()[0].id, point.value());
    EXPECT_TRUE(sketch_->solve());
}

TEST_P(SketchBehavior, CoincidentPlacementPreparesMidpointWithoutSolvingOtherEquations) {
    auto first = sketch_->addPoint({0, 2});
    auto second = sketch_->addPoint({8, 6});
    auto dirtyLine = sketch_->addLine({20, 0}, {24, 3});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(dirtyLine);
    ASSERT_TRUE(sketch_->addConstraint(horizontal(dirtyLine.value())));
    const GeometryRef a{first.value(), SubElement::Whole};
    const GeometryRef b{second.value(), SubElement::Whole};

    auto added = sketch_->addCoincident(a, b, CoincidentPlacement::Midpoint);
    ASSERT_TRUE(added);
    expectPosition(sketch_->pointPosition(a).value(), {4, 4});
    expectPosition(sketch_->pointPosition(b).value(), {4, 4});
    expectPosition(sketch_->pointPosition({dirtyLine.value(), SubElement::End}).value(), {24, 3});
    auto constraint = sketch_->constraint(added.value());
    ASSERT_TRUE(constraint);
    EXPECT_EQ(constraint.value().definition.type, ConstraintType::Coincident);
    EXPECT_EQ(constraint.value().definition.refs, (std::vector<GeometryRef>{a, b}));
    EXPECT_EQ(sketch_->constraintCount(), 2U);
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
}

TEST_P(SketchBehavior, CoincidentPlacementSupportsCurveElementsAndPreservesOtherCoordinates) {
    auto line = sketch_->addEntity(Line2{{0, 0}, {4, 0}}, true);
    auto circle = sketch_->addCircle({8, 2}, 3);
    auto arc = sketch_->addArc({20, 0}, {22, 0}, {20, 2});
    auto point = sketch_->addPoint({26, 0});
    ASSERT_TRUE(line);
    ASSERT_TRUE(circle);
    ASSERT_TRUE(arc);
    ASSERT_TRUE(point);
    const GeometryRef lineEnd{line.value(), SubElement::End};
    const GeometryRef center{circle.value(), SubElement::Center};
    ASSERT_TRUE(sketch_->addCoincident(lineEnd, center));
    expectPosition(sketch_->pointPosition(lineEnd).value(), {6, 1});
    expectPosition(sketch_->pointPosition(center).value(), {6, 1});
    expectPosition(sketch_->pointPosition({line.value(), SubElement::Start}).value(), {0, 0});
    EXPECT_TRUE(sketch_->entity(line.value()).value().construction);
    EXPECT_DOUBLE_EQ(std::get<Circle2>(sketch_->entity(circle.value()).value().geometry).radius, 3);

    const GeometryRef arcStart{arc.value(), SubElement::Start};
    ASSERT_TRUE(sketch_->addCoincident(arcStart, {point.value(), SubElement::Whole}));
    // The initial arc guess may be noncircular until the explicit solve.
    expectPosition(sketch_->pointPosition(arcStart).value(), {24, 0});
    expectPosition(sketch_->pointPosition({point.value(), SubElement::Whole}).value(), {24, 0});
    expectPosition(sketch_->pointPosition({arc.value(), SubElement::Center}).value(), {20, 0});
    expectPosition(sketch_->pointPosition({arc.value(), SubElement::End}).value(), {20, 2});
}

TEST_P(SketchBehavior, CoincidentPlacementMovesEntireTransitiveGroups) {
    auto created = sketch_->addEntities(std::array<EntityCreation, 4>{{{Point2{{0, 0}}}, {Point2{{0, 0}}}, {Point2{{0, 0}}}, {Point2{{6, 4}}}}});
    ASSERT_TRUE(created);
    const auto& ids = created.value();
    const GeometryRef a{ids[0], SubElement::Whole};
    const GeometryRef b{ids[1], SubElement::Whole};
    const GeometryRef c{ids[2], SubElement::Whole};
    const GeometryRef d{ids[3], SubElement::Whole};
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {a, b}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {b, c}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch_->addCoincident(c, d));
    for (const auto ref : {a, b, c, d}) {
        expectPosition(sketch_->pointPosition(ref).value(), {3, 2});
    }
}

TEST_P(SketchBehavior, CoincidentPlacementRespectsFixOnAnotherMemberInEitherArgumentOrder) {
    for (bool reverse : {false, true}) {
        auto anchor = sketch_->addPoint({1, 2});
        auto linked = sketch_->addPoint({1, 2});
        auto free = sketch_->addPoint({9, 8});
        ASSERT_TRUE(anchor);
        ASSERT_TRUE(linked);
        ASSERT_TRUE(free);
        const GeometryRef fixedRef{anchor.value(), SubElement::Whole};
        const GeometryRef linkedRef{linked.value(), SubElement::Whole};
        const GeometryRef freeRef{free.value(), SubElement::Whole};
        ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {fixedRef, linkedRef}, std::nullopt, std::nullopt}));
        ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {fixedRef}, std::nullopt, Vec2{1, 2}}));
        ASSERT_TRUE(sketch_->addCoincident(reverse ? freeRef : linkedRef, reverse ? linkedRef : freeRef));
        for (const auto ref : {fixedRef, linkedRef, freeRef}) {
            expectPosition(sketch_->pointPosition(ref).value(), {1, 2});
        }
        auto solved = sketch_->solve();
        ASSERT_TRUE(solved);
        EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    }
}

TEST_P(SketchBehavior, CoincidentPlacementUsesExplicitFixTargetBeforeSolving) {
    auto fixed = sketch_->addPoint({0, 0});
    auto free = sketch_->addPoint({9, 8});
    ASSERT_TRUE(fixed);
    ASSERT_TRUE(free);
    const GeometryRef a{fixed.value(), SubElement::Whole};
    const GeometryRef b{free.value(), SubElement::Whole};
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {a}, std::nullopt, Vec2{1, 2}}));
    ASSERT_TRUE(sketch_->addCoincident(a, b));
    expectPosition(sketch_->pointPosition(b).value(), {1, 2});
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    expectPosition(sketch_->pointPosition(a).value(), {1, 2});
    expectPosition(sketch_->pointPosition(b).value(), {1, 2});
}

TEST_P(SketchBehavior, CoincidentPlacementDoesNotReseedAnExistingGroup) {
    auto first = sketch_->addPoint({0, 0});
    auto second = sketch_->addPoint({0, 0});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const GeometryRef a{first.value(), SubElement::Whole};
    const GeometryRef b{second.value(), SubElement::Whole};
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {a, b}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch_->updatePoint(second.value(), Point2{{4, 2}}));
    const auto beforeA = sketch_->pointPosition(a);
    const auto beforeB = sketch_->pointPosition(b);
    ASSERT_TRUE(beforeA);
    ASSERT_TRUE(beforeB);
    ASSERT_TRUE(sketch_->addCoincident(a, b));
    expectPosition(sketch_->pointPosition(a).value(), beforeA.value());
    expectPosition(sketch_->pointPosition(b).value(), beforeB.value());
}

TEST_P(SketchBehavior, CoincidentPlacementLeavesConflictingFixedTargetsForExplicitSolve) {
    auto first = sketch_->addPoint({0, 0});
    auto second = sketch_->addPoint({5, 0});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const GeometryRef a{first.value(), SubElement::Whole};
    const GeometryRef b{second.value(), SubElement::Whole};
    auto fixA = sketch_->addConstraint({ConstraintType::Fix, {a}, std::nullopt, Vec2{0, 0}});
    auto fixB = sketch_->addConstraint({ConstraintType::Fix, {b}, std::nullopt, Vec2{5, 0}});
    ASSERT_TRUE(fixA);
    ASSERT_TRUE(fixB);
    ASSERT_TRUE(sketch_->addCoincident(a, b));
    EXPECT_EQ(sketch_->constraint(fixA.value()).value().definition.fixedPosition, (std::optional<Vec2>{{0, 0}}));
    EXPECT_EQ(sketch_->constraint(fixB.value()).value().definition.fixedPosition, (std::optional<Vec2>{{5, 0}}));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Failed);
}

TEST_P(SketchBehavior, CoincidentPreserveUsesOrdinaryConstraintInsertion) {
    auto first = sketch_->addPoint({0, 0});
    auto second = sketch_->addPoint({8, 4});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    auto state = sketch_->snapshot();
    ASSERT_TRUE(state);
    auto ordinary = Sketch::create(GetParam());
    ASSERT_TRUE(ordinary);
    ASSERT_TRUE(ordinary.value()->replaceState(state.value()));
    const GeometryRef a{first.value(), SubElement::Whole};
    const GeometryRef b{second.value(), SubElement::Whole};
    ASSERT_TRUE(ordinary.value()->addConstraint({ConstraintType::Coincident, {a, b}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch_->addCoincident(a, b, CoincidentPlacement::Preserve));
    for (const auto ref : {a, b}) {
        expectPosition(sketch_->pointPosition(ref).value(), ordinary.value()->pointPosition(ref).value());
    }
}

TEST_P(SketchBehavior, CoincidentPlacementPrevalidatesReferencesAndPolicyWithoutEditing) {
    auto point = sketch_->addPoint({0, 0});
    auto line = sketch_->addLine({6, 2}, {8, 3});
    ASSERT_TRUE(point);
    ASSERT_TRUE(line);
    const GeometryRef a{point.value(), SubElement::Whole};
    const GeometryRef b{line.value(), SubElement::Start};
    auto before = sketch_->snapshot();
    ASSERT_TRUE(before);
    expectError(sketch_->addCoincident(a, a), ErrorCode::InvalidArgument);
    expectError(sketch_->addCoincident(a, {line.value(), SubElement::Whole}), ErrorCode::InvalidArgument);
    expectError(sketch_->addCoincident(a, {EntityId(9999), SubElement::Whole}), ErrorCode::NotFound);
    expectError(sketch_->addCoincident(a, b, static_cast<CoincidentPlacement>(99)), ErrorCode::InvalidArgument);
    expectPosition(sketch_->pointPosition(a).value(), {0, 0});
    expectPosition(sketch_->pointPosition(b).value(), {6, 2});
    EXPECT_EQ(sketch_->constraintCount(), 0U);
    auto after = sketch_->snapshot();
    ASSERT_TRUE(after);
    EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
}

TEST_P(SketchBehavior, CoincidentPlacementChecksIdCapacityBeforeMovingPoints) {
    auto first = sketch_->addPoint({0, 0});
    auto second = sketch_->addPoint({8, 4});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    auto state = sketch_->snapshot();
    ASSERT_TRUE(state);
    state.value().lastConstraintId = std::numeric_limits<int64_t>::max();
    ASSERT_TRUE(sketch_->replaceState(state.value()));
    const GeometryRef a{first.value(), SubElement::Whole};
    const GeometryRef b{second.value(), SubElement::Whole};
    expectError(sketch_->addCoincident(a, b), ErrorCode::BackendFailure);
    expectPosition(sketch_->pointPosition(a).value(), {0, 0});
    expectPosition(sketch_->pointPosition(b).value(), {8, 4});
    EXPECT_EQ(sketch_->constraintCount(), 0U);
}

TEST_P(SketchBehavior, CoincidentMidpointAvoidsOverflowForLargeFiniteCoordinates) {
    auto first = sketch_->addPoint({1.2e308, -1.6e308});
    auto second = sketch_->addPoint({1.6e308, 1.6e308});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const GeometryRef a{first.value(), SubElement::Whole};
    const GeometryRef b{second.value(), SubElement::Whole};
    ASSERT_TRUE(sketch_->addCoincident(a, b));
    for (const auto ref : {a, b}) {
        const auto position = sketch_->pointPosition(ref);
        ASSERT_TRUE(position);
        EXPECT_TRUE(std::isfinite(position.value().x));
        EXPECT_NEAR(position.value().x / 1e308, 1.4, 1e-12);
        EXPECT_DOUBLE_EQ(position.value().y, 0);
    }
}

TEST_P(SketchBehavior, SolvesHorizontalLineThroughPublicApi) {
    auto line = sketch_->addLine({0, 0}, {5, 3});
    ASSERT_TRUE(line);
    auto relation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(relation);
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    auto queried = sketch_->entity(line.value());
    ASSERT_TRUE(queried);
    const auto& geometry = std::get<Line2>(queried.value().geometry);
    EXPECT_NEAR(geometry.start.y, geometry.end.y, 1e-4);
    EXPECT_EQ(queried.value().id, line.value());
    auto constraint = sketch_->constraint(relation.value());
    ASSERT_TRUE(constraint);
    EXPECT_EQ(constraint.value().definition.refs[0].entity, line.value());
    if (!sketch_->capabilities().degreesOfFreedom) EXPECT_FALSE(solved.value().degreesOfFreedom);
    if (!sketch_->capabilities().conflictingConstraints) EXPECT_FALSE(solved.value().conflictingConstraints);
    if (!sketch_->capabilities().redundantConstraints) EXPECT_FALSE(solved.value().redundantConstraints);
}

TEST_P(SketchBehavior, SolvesDistanceBetweenPublicPointReferences) {
    auto first = sketch_->addPoint({0, 0});
    auto second = sketch_->addPoint({3, 0});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ConstraintDefinition distance{ConstraintType::Distance, {{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}}, 5.0, std::nullopt};
    ASSERT_TRUE(sketch_->addConstraint(distance));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    auto a = sketch_->entity(first.value());
    auto b = sketch_->entity(second.value());
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    const Vec2 p = std::get<Point2>(a.value().geometry).position;
    const Vec2 q = std::get<Point2>(b.value().geometry).position;
    EXPECT_NEAR(std::hypot(q.x - p.x, q.y - p.y), 5.0, 1e-4);
}

TEST_P(SketchBehavior, UpdatingDimensionPreservesIdentityAndChangesSolvedLength) {
    auto line = sketch_->addLine({0, 0}, {3, 4});
    ASSERT_TRUE(line);
    ConstraintDefinition length{ConstraintType::Length, {{line.value(), SubElement::Whole}}, 5.0, std::nullopt};
    auto relation = sketch_->addConstraint(length);
    ASSERT_TRUE(relation);
    length.value = 8;
    ASSERT_TRUE(sketch_->updateConstraint(relation.value(), length));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    auto queried = sketch_->entity(line.value());
    ASSERT_TRUE(queried);
    const auto geometry = std::get<Line2>(queried.value().geometry);
    EXPECT_NEAR(std::hypot(geometry.end.x - geometry.start.x, geometry.end.y - geometry.start.y), 8, 1e-4);
    auto constraint = sketch_->constraint(relation.value());
    ASSERT_TRUE(constraint);
    EXPECT_EQ(constraint.value().id, relation.value());
    EXPECT_EQ(constraint.value().definition.value, 8);
}

TEST_P(SketchBehavior, AngleUsesRadiansBetweenDirectedLines) {
    auto a = sketch_->addLine({0, 0}, {4, 0});
    auto b = sketch_->addLine({0, 0}, {3, 3});
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    ConstraintDefinition angle{ConstraintType::Angle, {{a.value(), SubElement::Whole}, {b.value(), SubElement::Whole}}, std::numbers::pi / 3, std::nullopt};
    ASSERT_TRUE(sketch_->addConstraint(angle));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    auto first = sketch_->entity(a.value());
    auto second = sketch_->entity(b.value());
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const auto p = std::get<Line2>(first.value().geometry);
    const auto q = std::get<Line2>(second.value().geometry);
    const Vec2 u{p.end.x - p.start.x, p.end.y - p.start.y};
    const Vec2 v{q.end.x - q.start.x, q.end.y - q.start.y};
    const double cosine = (u.x * v.x + u.y * v.y) / (std::hypot(u.x, u.y) * std::hypot(v.x, v.y));
    EXPECT_NEAR(std::acos(std::clamp(cosine, -1.0, 1.0)), *angle.value, 1e-4);
}

TEST_P(SketchBehavior, IntrinsicArcEquationRepairsTransferredUnsolvedRadii) {
    auto arc = sketch_->addArc({0, 0}, {2, 0}, {0, 2});
    ASSERT_TRUE(arc);
    auto state = sketch_->snapshot();
    ASSERT_TRUE(state);
    std::get<Arc2>(state.value().entities.front().geometry).end = {0, 3};
    ASSERT_TRUE(sketch_->replaceState(state.value()));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    auto queried = sketch_->entity(arc.value());
    ASSERT_TRUE(queried);
    const auto geometry = std::get<Arc2>(queried.value().geometry);
    EXPECT_NEAR(std::hypot(geometry.start.x - geometry.center.x, geometry.start.y - geometry.center.y),
                std::hypot(geometry.end.x - geometry.center.x, geometry.end.y - geometry.center.y), 1e-4);
    auto constraints = sketch_->constraints();
    ASSERT_TRUE(constraints);
    EXPECT_TRUE(constraints.value().empty());
}

TEST_P(SketchBehavior, FixRetargetingAndRemovalDoNotLeavePrivateConstraints) {
    auto point = sketch_->addPoint({0, 0});
    ASSERT_TRUE(point);
    ConstraintDefinition fix{ConstraintType::Fix, {{point.value(), SubElement::Whole}}, std::nullopt, Vec2{1, 2}};
    auto relation = sketch_->addConstraint(fix);
    ASSERT_TRUE(relation);
    fix.fixedPosition = Vec2{3, 4};
    ASSERT_TRUE(sketch_->updateConstraint(relation.value(), fix));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    auto queried = sketch_->entity(point.value());
    ASSERT_TRUE(queried);
    expectGeometry(queried.value().geometry, Point2{{3, 4}});
    ASSERT_TRUE(sketch_->removeConstraint(relation.value()));
    auto dragged = sketch_->drag({{point.value(), SubElement::Whole}, {7, 8}});
    ASSERT_TRUE(dragged);
    EXPECT_EQ(dragged.value().status, SolveStatus::Converged);
    queried = sketch_->entity(point.value());
    ASSERT_TRUE(queried);
    expectGeometry(queried.value().geometry, Point2{{7, 8}});
}

TEST_P(SketchBehavior, CollapsedLineIsNotReportedAsValidConvergedGeometry) {
    auto line = sketch_->addLine({0, 0}, {1, 1});
    ASSERT_TRUE(line);
    auto definition = horizontal(line.value());
    ASSERT_TRUE(sketch_->addConstraint(definition));
    definition.type = ConstraintType::Vertical;
    ASSERT_TRUE(sketch_->addConstraint(definition));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Failed);
}

TEST_P(SketchBehavior, IdExhaustionDoesNotOverflowOrMutateModel) {
    SketchSnapshot state;
    state.lastEntityId = std::numeric_limits<int64_t>::max();
    state.lastConstraintId = std::numeric_limits<int64_t>::max();
    state.entities.push_back({EntityId(1), Line2{{0, 0}, {1, 0}}, false});
    ASSERT_TRUE(sketch_->replaceState(state));
    expectError(sketch_->addPoint({0, 0}), ErrorCode::BackendFailure);
    expectError(sketch_->addConstraint(horizontal(EntityId(1))), ErrorCode::BackendFailure);
    EXPECT_EQ(sketch_->entities().value().size(), 1U);
    EXPECT_TRUE(sketch_->constraints().value().empty());
}

TEST_P(SketchBehavior, CircularDimensionsAndEqualityRespectCapabilities) {
    auto circle = sketch_->addCircle({0, 0}, 3);
    auto arc = sketch_->addArc({10, 0}, {12, 0}, {10, 2});
    ASSERT_TRUE(circle);
    ASSERT_TRUE(arc);
    ConstraintDefinition radius{ConstraintType::Radius, {{circle.value(), SubElement::Whole}}, 4.0, std::nullopt};
    ConstraintDefinition equal{ConstraintType::Equal, {{circle.value(), SubElement::Whole}, {arc.value(), SubElement::Whole}}, std::nullopt, std::nullopt};
    if (!sketch_->capabilities().supports(ConstraintType::Radius)) {
        expectError(sketch_->addConstraint(radius), ErrorCode::Unsupported);
        expectError(sketch_->addConstraint(equal), ErrorCode::Unsupported);
        return;
    }
    auto dimension = sketch_->addConstraint(radius);
    ASSERT_TRUE(dimension);
    ASSERT_TRUE(sketch_->addConstraint(equal));
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    radius.type = ConstraintType::Diameter;
    radius.value = 10;
    ASSERT_TRUE(sketch_->updateConstraint(dimension.value(), radius));
    solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    auto c = sketch_->entity(circle.value());
    auto a = sketch_->entity(arc.value());
    ASSERT_TRUE(c);
    ASSERT_TRUE(a);
    EXPECT_NEAR(std::get<Circle2>(c.value().geometry).radius, 5, 1e-4);
    const auto g = std::get<Arc2>(a.value().geometry);
    EXPECT_NEAR(std::hypot(g.start.x - g.center.x, g.start.y - g.center.y), 5, 1e-4);
}

TEST_P(SketchBehavior, DragUpdatesGeometryWithoutPersistentConstraintsOrIdentityChanges) {
    auto point = sketch_->addPoint({1, 2});
    ASSERT_TRUE(point);
    ASSERT_TRUE(sketch_->capabilities().drag);
    auto dragged = sketch_->drag({{point.value(), SubElement::Whole}, {7, 9}});
    ASSERT_TRUE(dragged);
    EXPECT_EQ(dragged.value().status, SolveStatus::Converged);
    auto queried = sketch_->entity(point.value());
    ASSERT_TRUE(queried);
    EXPECT_EQ(queried.value().id, point.value());
    expectPosition(std::get<Point2>(queried.value().geometry).position, {7, 9});
    auto constraints = sketch_->constraints();
    ASSERT_TRUE(constraints);
    EXPECT_TRUE(constraints.value().empty());
}

TEST_P(SketchBehavior, BatchDragTranslatesConstrainedLineAndArcTogether) {
    auto line = sketch_->addLine({0, 0}, {5, 0});
    auto arc = sketch_->addArc({10, 0}, {12, 0}, {10, 2});
    ASSERT_TRUE(line);
    ASSERT_TRUE(arc);
    ASSERT_TRUE(sketch_->addConstraint(horizontal(line.value())));
    std::vector<DragRequest> requests{
        {{line.value(), SubElement::Start}, {3, 4}},
        {{line.value(), SubElement::End}, {8, 4}},
        {{arc.value(), SubElement::Start}, {15, 4}},
        {{arc.value(), SubElement::End}, {13, 6}},
        {{arc.value(), SubElement::Center}, {13, 4}},
    };
    auto dragged = sketch_->dragPoints(requests);
    ASSERT_TRUE(dragged);
    EXPECT_EQ(dragged.value().status, SolveStatus::Converged);
    for (const auto& request : requests) {
        auto position = sketch_->pointPosition(request.point);
        ASSERT_TRUE(position);
        expectPosition(position.value(), request.target);
    }
    EXPECT_EQ(sketch_->entityCount(), 2U);
    EXPECT_EQ(sketch_->constraintCount(), 1U);
}

TEST_P(SketchBehavior, BatchDragAcceptsArcEndpointTargetBeforeCircularityIsRestored) {
    auto arc = sketch_->addArc({0, 0}, {5, 0}, {0, 5});
    ASSERT_TRUE(arc);
    const std::vector<DragRequest> requests{{{arc.value(), SubElement::Start}, {3, 5}}};
    auto dragged = sketch_->dragPoints(requests);
    ASSERT_TRUE(dragged);
    EXPECT_EQ(dragged.value().status, SolveStatus::Converged);
    auto queried = sketch_->entity(arc.value());
    ASSERT_TRUE(queried);
    const auto& geometry = std::get<Arc2>(queried.value().geometry);
    expectPosition(geometry.start, {3, 5});
    EXPECT_NEAR(std::hypot(geometry.start.x - geometry.center.x, geometry.start.y - geometry.center.y),
                std::hypot(geometry.end.x - geometry.center.x, geometry.end.y - geometry.center.y), 1e-4);
}

TEST_P(SketchBehavior, BatchDragPrevalidatesAllTargetsWithoutMovingEarlierPoints) {
    auto point = sketch_->addPoint({1, 2});
    auto line = sketch_->addLine({0, 0}, {5, 0});
    ASSERT_TRUE(point);
    ASSERT_TRUE(line);
    std::vector<DragRequest> requests{
        {{point.value(), SubElement::Whole}, {7, 9}},
        {{line.value(), SubElement::Whole}, {3, 4}},
    };
    expectError(sketch_->dragPoints(requests), ErrorCode::InvalidArgument);
    requests[1] = requests[0];
    expectError(sketch_->dragPoints(requests), ErrorCode::InvalidArgument);
    requests[1] = {{line.value(), SubElement::Start}, {std::numeric_limits<double>::infinity(), 0}};
    expectError(sketch_->dragPoints(requests), ErrorCode::InvalidArgument);
    requests[1] = {{EntityId(99999), SubElement::Whole}, {3, 4}};
    expectError(sketch_->dragPoints(requests), ErrorCode::NotFound);
    auto position = sketch_->pointPosition({point.value(), SubElement::Whole});
    ASSERT_TRUE(position);
    expectPosition(position.value(), {1, 2});
    auto empty = sketch_->dragPoints({});
    ASSERT_TRUE(empty);
    EXPECT_EQ(empty.value().status, SolveStatus::Converged);
    position = sketch_->pointPosition({point.value(), SubElement::Whole});
    ASSERT_TRUE(position);
    expectPosition(position.value(), {1, 2});
}

TEST_P(SketchBehavior, BatchDragRespectsFixedPointsWithoutAddingConstraints) {
    auto fixed = sketch_->addPoint({1, 2});
    auto free = sketch_->addPoint({5, 6});
    ASSERT_TRUE(fixed);
    ASSERT_TRUE(free);
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{fixed.value(), SubElement::Whole}}, std::nullopt, Vec2{1, 2}}));
    const std::vector<DragRequest> requests{
        {{fixed.value(), SubElement::Whole}, {7, 9}},
        {{free.value(), SubElement::Whole}, {11, 13}},
    };
    auto dragged = sketch_->dragPoints(requests);
    ASSERT_TRUE(dragged);
    EXPECT_EQ(dragged.value().status, SolveStatus::Converged);
    auto position = sketch_->pointPosition({fixed.value(), SubElement::Whole});
    ASSERT_TRUE(position);
    expectPosition(position.value(), {1, 2});
    position = sketch_->pointPosition({free.value(), SubElement::Whole});
    ASSERT_TRUE(position);
    expectPosition(position.value(), {11, 13});
    EXPECT_EQ(sketch_->constraintCount(), 1U);
}

TEST_P(SketchBehavior, DragPreservesExistingDistanceConstraint) {
    auto first = sketch_->addPoint({0, 0});
    auto second = sketch_->addPoint({5, 0});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ConstraintDefinition distance{ConstraintType::Distance, {{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}}, 5.0, std::nullopt};
    auto relation = sketch_->addConstraint(distance);
    ASSERT_TRUE(relation);
    auto dragged = sketch_->drag({{first.value(), SubElement::Whole}, {2, 2}});
    ASSERT_TRUE(dragged);
    EXPECT_EQ(dragged.value().status, SolveStatus::Converged);
    auto a = sketch_->entity(first.value());
    auto b = sketch_->entity(second.value());
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    const Vec2 p = std::get<Point2>(a.value().geometry).position;
    const Vec2 q = std::get<Point2>(b.value().geometry).position;
    EXPECT_NEAR(std::hypot(q.x - p.x, q.y - p.y), 5.0, 1e-4);
    auto constraints = sketch_->constraints();
    ASSERT_TRUE(constraints);
    ASSERT_EQ(constraints.value().size(), 1U);
    EXPECT_EQ(constraints.value().front().id, relation.value());
}

TEST_P(SketchBehavior, ExplicitFixTargetSurvivesGeometryEditsAndStateTransfer) {
    auto point = sketch_->addPoint({1, 2});
    ASSERT_TRUE(point);
    ConstraintDefinition fixed{ConstraintType::Fix, {{point.value(), SubElement::Whole}}, std::nullopt, Vec2{4, 5}};
    auto relation = sketch_->addConstraint(fixed);
    ASSERT_TRUE(relation);
    ASSERT_TRUE(sketch_->updateEntity(point.value(), Point2{{9, 10}}));
    auto state = sketch_->snapshot();
    ASSERT_TRUE(state);
    ASSERT_EQ(state.value().constraints.size(), 1U);
    EXPECT_EQ(state.value().constraints.front().definition.fixedPosition, fixed.fixedPosition);
    for (BackendKind backend : Sketch::availableBackends()) {
        auto created = Sketch::create(backend);
        ASSERT_TRUE(created);
        auto& imported = *created.value();
        ASSERT_TRUE(imported.replaceState(state.value()));
        auto solved = imported.solve();
        ASSERT_TRUE(solved);
        EXPECT_EQ(solved.value().status, SolveStatus::Converged);
        auto queried = imported.entity(point.value());
        ASSERT_TRUE(queried);
        expectGeometry(queried.value().geometry, Point2{{4, 5}});
        auto queriedRelation = imported.constraint(relation.value());
        ASSERT_TRUE(queriedRelation);
        EXPECT_EQ(queriedRelation.value().definition.fixedPosition, fixed.fixedPosition);
    }
}

TEST_P(SketchBehavior, InconsistentFixConstraintsReportFailedConvergence) {
    auto point = sketch_->addPoint({0, 0});
    ASSERT_TRUE(point);
    ConstraintDefinition first{ConstraintType::Fix, {{point.value(), SubElement::Whole}}, std::nullopt, Vec2{0, 0}};
    ConstraintDefinition second{ConstraintType::Fix, {{point.value(), SubElement::Whole}}, std::nullopt, Vec2{1, 1}};
    auto a = sketch_->addConstraint(first);
    auto b = sketch_->addConstraint(second);
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Failed);
    EXPECT_TRUE(sketch_->entity(point.value()));
    EXPECT_TRUE(sketch_->constraint(a.value()));
    EXPECT_TRUE(sketch_->constraint(b.value()));
}

TEST_P(SketchBehavior, UnsupportedConstraintIsStructuredAndDoesNotMutate) {
    auto line = sketch_->addLine({0, 1}, {3, 1});
    auto circle = sketch_->addCircle({0, 0}, 1);
    ASSERT_TRUE(line);
    ASSERT_TRUE(circle);
    ConstraintDefinition tangent{ConstraintType::Tangent, {{line.value(), SubElement::Whole}, {circle.value(), SubElement::Whole}}, std::nullopt, std::nullopt};
    EXPECT_FALSE(sketch_->capabilities().supports(ConstraintType::Tangent));
    expectError(sketch_->supportsConstraint(tangent), ErrorCode::Unsupported);
    expectError(sketch_->addConstraint(tangent), ErrorCode::Unsupported);
    auto relation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(relation);
    expectError(sketch_->updateConstraint(relation.value(), tangent), ErrorCode::Unsupported);
    auto queried = sketch_->constraint(relation.value());
    ASSERT_TRUE(queried);
    EXPECT_EQ(queried.value().definition.type, ConstraintType::Horizontal);
    auto all = sketch_->constraints();
    ASSERT_TRUE(all);
    EXPECT_EQ(all.value().size(), 1U);
}

TEST_P(SketchBehavior, DeletedEntityAndConstraintIdsAreNeverReused) {
    auto first = sketch_->addLine({0, 0}, {1, 0});
    ASSERT_TRUE(first);
    auto relation = sketch_->addConstraint(horizontal(first.value()));
    ASSERT_TRUE(relation);
    ASSERT_TRUE(sketch_->removeEntity(first.value()));
    auto second = sketch_->addLine({0, 0}, {2, 0});
    ASSERT_TRUE(second);
    auto nextRelation = sketch_->addConstraint(horizontal(second.value()));
    ASSERT_TRUE(nextRelation);
    EXPECT_GT(second.value().get(), first.value().get());
    EXPECT_GT(nextRelation.value().get(), relation.value().get());
}

TEST_P(SketchBehavior, InvalidGeometryAndEntityKindChangesLeaveStateUnchanged) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    expectError(sketch_->addPoint({nan, 0}), ErrorCode::InvalidArgument);
    expectError(sketch_->addCircle({0, 0}, -1), ErrorCode::InvalidArgument);
    expectError(sketch_->addLine({1, 1}, {1, 1}), ErrorCode::InvalidArgument);
    expectError(sketch_->addArc({0, 0}, {1, 0}, {0, 2}), ErrorCode::InvalidArgument);
    auto point = sketch_->addPoint({1, 2});
    ASSERT_TRUE(point);
    expectError(sketch_->updateEntity(point.value(), Circle2{{0, 0}, 2}), ErrorCode::InvalidArgument);
    expectError(sketch_->updateEntity(point.value(), Point2{{0, nan}}), ErrorCode::InvalidArgument);
    auto queried = sketch_->entity(point.value());
    ASSERT_TRUE(queried);
    expectGeometry(queried.value().geometry, Point2{{1, 2}});
    auto all = sketch_->entities();
    ASSERT_TRUE(all);
    EXPECT_EQ(all.value().size(), 1U);
}

TEST_P(SketchBehavior, RejectsMissingEntitiesAndInvalidSubElementReferences) {
    auto line = sketch_->addLine({0, 0}, {1, 1});
    auto point = sketch_->addPoint({2, 2});
    ASSERT_TRUE(line);
    ASSERT_TRUE(point);
    expectError(sketch_->addConstraint(horizontal(EntityId(10000))), ErrorCode::NotFound);
    auto invalid = horizontal(point.value());
    expectError(sketch_->addConstraint(invalid), ErrorCode::InvalidArgument);
    ConstraintDefinition coincident{
        ConstraintType::Coincident, {{line.value(), SubElement::Center}, {point.value(), SubElement::Whole}}, std::nullopt, std::nullopt};
    expectError(sketch_->supportsConstraint(coincident), ErrorCode::InvalidArgument);
    expectError(sketch_->addConstraint(coincident), ErrorCode::InvalidArgument);
    expectError(sketch_->drag({{line.value(), SubElement::Whole}, {3, 4}}), ErrorCode::InvalidArgument);
    expectError(sketch_->entity(EntityId()), ErrorCode::InvalidArgument);
    expectError(sketch_->constraint(ConstraintId(10000)), ErrorCode::NotFound);
    auto all = sketch_->constraints();
    ASSERT_TRUE(all);
    EXPECT_TRUE(all.value().empty());
}

TEST_P(SketchBehavior, RejectsInvalidConstraintUpdatesWithoutLosingOriginal) {
    auto line = sketch_->addLine({0, 0}, {3, 4});
    ASSERT_TRUE(line);
    auto added = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(added);
    ConstraintDefinition invalid{ConstraintType::Length, {{line.value(), SubElement::Whole}}, -2.0, std::nullopt};
    expectError(sketch_->updateConstraint(added.value(), invalid), ErrorCode::InvalidArgument);
    auto queried = sketch_->constraint(added.value());
    ASSERT_TRUE(queried);
    EXPECT_EQ(queried.value().definition.type, ConstraintType::Horizontal);
    expectError(sketch_->updateConstraint(ConstraintId(10000), horizontal(line.value())), ErrorCode::NotFound);
}

TEST_P(SketchBehavior, NeutralSnapshotRoundTripsAndRetainsDeletedIdWatermarks) {
    auto line = sketch_->addLine({1, 2}, {5, 2});
    auto discarded = sketch_->addPoint({8, 9});
    ASSERT_TRUE(line);
    ASSERT_TRUE(discarded);
    auto relation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(relation);
    auto discardedRelation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(discardedRelation);
    ASSERT_TRUE(sketch_->removeConstraint(discardedRelation.value()));
    ASSERT_TRUE(sketch_->removeEntity(discarded.value()));
    auto state = sketch_->snapshot();
    ASSERT_TRUE(state);
    EXPECT_GE(state.value().lastEntityId, discarded.value().get());
    EXPECT_GE(state.value().lastConstraintId, discardedRelation.value().get());
    auto created = Sketch::create(GetParam());
    ASSERT_TRUE(created);
    auto& imported = *created.value();
    ASSERT_TRUE(imported.replaceState(state.value()));
    auto restored = imported.entity(line.value());
    ASSERT_TRUE(restored);
    expectGeometry(restored.value().geometry, Line2{{1, 2}, {5, 2}});
    auto restoredRelation = imported.constraint(relation.value());
    ASSERT_TRUE(restoredRelation);
    EXPECT_EQ(restoredRelation.value().definition.refs, horizontal(line.value()).refs);
    auto next = imported.addPoint({10, 11});
    auto nextRelation = imported.addConstraint(horizontal(line.value()));
    ASSERT_TRUE(next);
    ASSERT_TRUE(nextRelation);
    EXPECT_GT(next.value().get(), discarded.value().get());
    EXPECT_GT(nextRelation.value().get(), discardedRelation.value().get());
}

TEST_P(SketchBehavior, ReplacingWithOlderStatePreservesDestinationIdWatermarks) {
    auto old = sketch_->snapshot();
    ASSERT_TRUE(old);
    auto line = sketch_->addLine({0, 0}, {1, 0});
    ASSERT_TRUE(line);
    auto relation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(relation);
    ASSERT_TRUE(sketch_->replaceState(old.value()));
    expectError(sketch_->entity(line.value()), ErrorCode::NotFound);
    auto replacement = sketch_->addLine({0, 0}, {2, 0});
    ASSERT_TRUE(replacement);
    auto replacementRelation = sketch_->addConstraint(horizontal(replacement.value()));
    ASSERT_TRUE(replacementRelation);
    EXPECT_GT(replacement.value().get(), line.value().get());
    EXPECT_GT(replacementRelation.value().get(), relation.value().get());
}

TEST_P(SketchBehavior, InvalidStateReplacementIsAtomic) {
    auto point = sketch_->addPoint({1, 2});
    ASSERT_TRUE(point);
    auto state = sketch_->snapshot();
    ASSERT_TRUE(state);
    auto duplicate = state.value();
    duplicate.entities.push_back(duplicate.entities.front());
    expectError(sketch_->replaceState(duplicate), ErrorCode::InvalidArgument);
    auto dangling = state.value();
    dangling.constraints.push_back({ConstraintId(1), horizontal(EntityId(10000))});
    dangling.lastConstraintId = 1;
    auto result = sketch_->replaceState(dangling);
    ASSERT_FALSE(result);
    EXPECT_TRUE(result.error().code == ErrorCode::NotFound || result.error().code == ErrorCode::InvalidArgument);
    auto queried = sketch_->entity(point.value());
    ASSERT_TRUE(queried);
    expectGeometry(queried.value().geometry, Point2{{1, 2}});
    EXPECT_EQ(sketch_->backendKind(), GetParam());
    auto all = sketch_->entities();
    ASSERT_TRUE(all);
    EXPECT_EQ(all.value().size(), 1U);
}

TEST_P(SketchBehavior, SeparateSketchesDoNotShareGeometryOrConstraintState) {
    auto created = Sketch::create(GetParam());
    ASSERT_TRUE(created);
    auto& other = *created.value();
    auto first = sketch_->addLine({0, 0}, {5, 3});
    auto second = other.addLine({20, 30}, {24, 30});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(sketch_->addConstraint(horizontal(first.value())));
    ConstraintDefinition vertical{ConstraintType::Vertical, {{second.value(), SubElement::Whole}}, std::nullopt, std::nullopt};
    ASSERT_TRUE(other.addConstraint(vertical));
    ASSERT_TRUE(sketch_->solve());
    auto before = sketch_->entity(first.value());
    ASSERT_TRUE(before);
    ASSERT_TRUE(other.solve());
    auto after = sketch_->entity(first.value());
    ASSERT_TRUE(after);
    expectGeometry(after.value().geometry, before.value().geometry);
    auto secondGeometry = other.entity(second.value());
    ASSERT_TRUE(secondGeometry);
    const auto& line = std::get<Line2>(secondGeometry.value().geometry);
    EXPECT_NEAR(line.start.x, line.end.x, 1e-4);
    ASSERT_TRUE(other.removeEntity(second.value()));
    EXPECT_TRUE(sketch_->entity(first.value()));
}

TEST_P(SketchBehavior, RuntimeBackendSwitchPreservesPublicIdentityAndGeometry) {
    auto line = sketch_->addLine({1, 2}, {5, 2});
    auto circle = sketch_->addCircle({8, 9}, 3);
    ASSERT_TRUE(line);
    ASSERT_TRUE(circle);
    auto relation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(relation);
    for (BackendKind backend : Sketch::availableBackends()) {
        ASSERT_TRUE(sketch_->switchBackend(backend));
        EXPECT_EQ(sketch_->backendKind(), backend);
        auto queried = sketch_->entity(line.value());
        auto queriedCircle = sketch_->entity(circle.value());
        auto queriedConstraint = sketch_->constraint(relation.value());
        ASSERT_TRUE(queried);
        ASSERT_TRUE(queriedCircle);
        ASSERT_TRUE(queriedConstraint);
        expectGeometry(queried.value().geometry, Line2{{1, 2}, {5, 2}});
        expectGeometry(queriedCircle.value().geometry, Circle2{{8, 9}, 3});
        EXPECT_EQ(queriedConstraint.value().definition.refs, horizontal(line.value()).refs);
        auto solved = sketch_->solve();
        ASSERT_TRUE(solved);
        EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    }
    auto next = sketch_->addPoint({0, 0});
    ASSERT_TRUE(next);
    EXPECT_GT(next.value().get(), circle.value().get());
}

TEST_P(SketchBehavior, FilteredAndNamedQueriesReturnOrderedDetachedPublicEntities) {
    for (const auto& empty : {sketch_->points(), sketch_->lines(), sketch_->circles(), sketch_->arcs()}) {
        ASSERT_TRUE(empty);
        EXPECT_TRUE(empty.value().empty());
    }
    for (auto kind : {EntityKind::Point, EntityKind::Line, EntityKind::Circle, EntityKind::Arc}) {
        auto empty = sketch_->entities(kind);
        ASSERT_TRUE(empty);
        EXPECT_TRUE(empty.value().empty());
        ASSERT_TRUE(sketch_->entityCount(kind));
        EXPECT_EQ(sketch_->entityCount(kind).value(), 0U);
    }
    const std::array<EntityCreation, 5> input{{{Line2{{0, 0}, {2, 0}}, true},
                                               {Point2{{1, 2}}, true},
                                               {Circle2{{4, 5}, 2}, false},
                                               {Arc2{{0, 0}, {3, 0}, {0, 3}}, true},
                                               {Line2{{10, 0}, {12, 0}}, false}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    ASSERT_EQ(added.value().size(), input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        auto queried = sketch_->entity(added.value()[i]);
        ASSERT_TRUE(queried);
        expectGeometry(queried.value().geometry, input[i].geometry);
        EXPECT_EQ(queried.value().construction, input[i].construction);
        if (i != 0) {
            EXPECT_LT(added.value()[i - 1].get(), added.value()[i].get());
        }
    }
    const std::array named{sketch_->points(), sketch_->lines(), sketch_->circles(), sketch_->arcs()};
    for (size_t i = 0; i < named.size(); ++i) {
        const auto kind = static_cast<EntityKind>(i);
        auto filtered = sketch_->entities(kind);
        ASSERT_TRUE(filtered);
        ASSERT_TRUE(named[i]);
        ASSERT_EQ(filtered.value().size(), named[i].value().size());
        EXPECT_EQ(filtered.value().size(), sketch_->entityCount(kind).value());
        for (size_t j = 0; j < filtered.value().size(); ++j) {
            EXPECT_EQ(entityKind(filtered.value()[j].geometry), kind);
            EXPECT_EQ(filtered.value()[j].id, named[i].value()[j].id);
            expectGeometry(filtered.value()[j].geometry, named[i].value()[j].geometry);
            EXPECT_EQ(filtered.value()[j].construction, named[i].value()[j].construction);
        }
    }
    EXPECT_EQ(sketch_->entityCount(), input.size());
    EXPECT_EQ(sketch_->constraintCount(), 0U);
    auto lines = sketch_->lines();
    ASSERT_TRUE(lines);
    ASSERT_EQ(lines.value().size(), 2U);
    EXPECT_EQ(lines.value()[0].id, added.value()[0]);
    EXPECT_EQ(lines.value()[1].id, added.value()[4]);
    lines.value()[0].geometry = Line2{{100, 100}, {200, 200}};
    lines.value()[0].construction = false;
    expectGeometry(sketch_->entity(added.value()[0]).value().geometry, input[0].geometry);
    EXPECT_TRUE(sketch_->entity(added.value()[0]).value().construction);
    ASSERT_TRUE(sketch_->removeEntity(added.value()[4]));
    EXPECT_EQ(lines.value().size(), 2U);
    expectError(sketch_->entities(static_cast<EntityKind>(99)), ErrorCode::InvalidArgument);
    expectError(sketch_->entityCount(static_cast<EntityKind>(99)), ErrorCode::InvalidArgument);
}

TEST_P(SketchBehavior, PointElementScopesAndPositionsUsePublicReferenceIdentity) {
    auto empty = sketch_->pointElements();
    ASSERT_TRUE(empty);
    EXPECT_TRUE(empty.value().empty());
    for (auto scope : {PointElementScope::StandalonePoints, PointElementScope::CurveSubElements, PointElementScope::All}) {
        auto scoped = sketch_->pointElements(scope);
        ASSERT_TRUE(scoped);
        EXPECT_TRUE(scoped.value().empty());
    }
    const std::array<EntityCreation, 4> input{{{Point2{{2, 0}}}, {Line2{{0, 0}, {2, 0}}}, {Circle2{{2, 0}, 1}}, {Arc2{{0, 0}, {2, 0}, {0, 2}}}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    const std::vector<PointElement> expected{{{ids[0], SubElement::Whole}, {2, 0}},  {{ids[1], SubElement::Start}, {0, 0}}, {{ids[1], SubElement::End}, {2, 0}},
                                             {{ids[2], SubElement::Center}, {2, 0}}, {{ids[3], SubElement::Start}, {2, 0}}, {{ids[3], SubElement::End}, {0, 2}},
                                             {{ids[3], SubElement::Center}, {0, 0}}};
    auto all = sketch_->pointElements();
    auto standalone = sketch_->pointElements(PointElementScope::StandalonePoints);
    auto curves = sketch_->pointElements(PointElementScope::CurveSubElements);
    auto explicitAll = sketch_->pointElements(PointElementScope::All);
    ASSERT_TRUE(all);
    ASSERT_TRUE(standalone);
    ASSERT_TRUE(curves);
    ASSERT_TRUE(explicitAll);
    ASSERT_EQ(all.value().size(), expected.size());
    ASSERT_EQ(standalone.value().size(), 1U);
    ASSERT_EQ(curves.value().size(), 6U);
    ASSERT_EQ(explicitAll.value().size(), expected.size());
    EXPECT_EQ(standalone.value()[0].ref, expected[0].ref);
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(all.value()[i].ref, expected[i].ref);
        EXPECT_EQ(explicitAll.value()[i].ref, expected[i].ref);
        expectPosition(explicitAll.value()[i].position, expected[i].position);
        expectPosition(all.value()[i].position, expected[i].position);
        auto position = sketch_->pointPosition(expected[i].ref);
        ASSERT_TRUE(position);
        expectPosition(position.value(), expected[i].position);
        if (i != 0) {
            EXPECT_EQ(curves.value()[i - 1].ref, expected[i].ref);
            expectPosition(curves.value()[i - 1].position, expected[i].position);
        }
    }
    for (auto id : ids) {
        auto elements = sketch_->pointElements(id);
        ASSERT_TRUE(elements);
        for (auto sub : {SubElement::Whole, SubElement::Start, SubElement::End, SubElement::Center, static_cast<SubElement>(99)}) {
            const GeometryRef ref{id, sub};
            const auto found = std::find_if(expected.begin(), expected.end(), [ref](const auto& p) { return p.ref == ref; });
            auto position = sketch_->pointPosition(ref);
            if (found == expected.end()) {
                expectError(position, ErrorCode::InvalidArgument);
            } else {
                ASSERT_TRUE(position);
                expectPosition(position.value(), found->position);
            }
        }
        const auto count = std::count_if(expected.begin(), expected.end(), [id](const auto& p) { return p.ref.entity == id; });
        ASSERT_EQ(elements.value().size(), static_cast<size_t>(count));
        size_t index = 0;
        for (const auto& p : expected) {
            if (p.ref.entity == id) {
                EXPECT_EQ(elements.value()[index].ref, p.ref);
                expectPosition(elements.value()[index++].position, p.position);
            }
        }
    }
    all.value()[0].position = {99, 99};
    expectPosition(sketch_->pointPosition(expected[0].ref).value(), {2, 0});
    ASSERT_TRUE(sketch_->updateEntity(ids[1], Line2{{10, 0}, {12, 0}}));
    expectPosition(all.value()[1].position, {0, 0});
    expectError(sketch_->pointPosition({EntityId(), SubElement::Whole}), ErrorCode::InvalidArgument);
    expectError(sketch_->pointPosition({EntityId(9999), SubElement::Start}), ErrorCode::NotFound);
    expectError(sketch_->pointElements(EntityId()), ErrorCode::InvalidArgument);
    expectError(sketch_->pointElements(EntityId(9999)), ErrorCode::NotFound);
    expectError(sketch_->pointElements(static_cast<PointElementScope>(99)), ErrorCode::InvalidArgument);
}

TEST_P(SketchBehavior, CoincidentPointElementsRemainDistinctReferences) {
    const std::array<EntityCreation, 4> input{{{Point2{{2, 0}}}, {Line2{{0, 0}, {2, 0}}}, {Circle2{{2, 0}, 1}}, {Arc2{{0, 0}, {2, 0}, {0, 2}}}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    const std::array refs{GeometryRef{ids[0], SubElement::Whole}, GeometryRef{ids[1], SubElement::End}, GeometryRef{ids[2], SubElement::Center},
                          GeometryRef{ids[3], SubElement::Start}};
    for (size_t i = 1; i < refs.size(); ++i) {
        ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {refs[0], refs[i]}, std::nullopt, std::nullopt}));
    }
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    ASSERT_EQ(solved.value().status, SolveStatus::Converged);
    auto all = sketch_->pointElements();
    ASSERT_TRUE(all);
    EXPECT_EQ(all.value().size(), 7U);
    for (auto ref : refs) {
        EXPECT_EQ(std::count_if(all.value().begin(), all.value().end(), [ref](const auto& p) { return p.ref == ref; }), 1);
        auto position = sketch_->pointPosition(ref);
        ASSERT_TRUE(position);
        expectPosition(position.value(), sketch_->pointPosition(refs[0]).value());
    }
}

TEST_P(SketchBehavior, MixedBatchUpdatesPreserveConstructionUnlessExplicitlyChanged) {
    const std::array<EntityCreation, 4> input{
        {{Point2{{1, 2}}, true}, {Line2{{0, 0}, {2, 0}}, true}, {Circle2{{4, 5}, 2}, false}, {Arc2{{0, 0}, {3, 0}, {0, 3}}, true}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    const std::array<EntityUpdate, 4> updates{{{ids[0], Point2{{5, 6}}, std::nullopt},
                                               {ids[1], Line2{{1, 1}, {4, 1}}, false},
                                               {ids[2], Circle2{{8, 9}, 3}, true},
                                               {ids[3], Arc2{{1, 1}, {5, 1}, {1, 5}}, std::nullopt}}};
    ASSERT_TRUE(sketch_->updateEntities(updates));
    for (size_t i = 0; i < updates.size(); ++i) {
        auto queried = sketch_->entity(ids[i]);
        ASSERT_TRUE(queried);
        expectGeometry(queried.value().geometry, updates[i].geometry);
        EXPECT_EQ(queried.value().construction, updates[i].construction.value_or(input[i].construction));
    }
    ASSERT_TRUE(sketch_->updateEntity(ids[0], Point2{{7, 8}}));
    EXPECT_TRUE(sketch_->entity(ids[0]).value().construction);
    ASSERT_TRUE(sketch_->updateEntity(ids[0], Point2{{9, 10}}, false));
    EXPECT_FALSE(sketch_->entity(ids[0]).value().construction);
    // Even an unsolved fixed point must not be projected by a metadata edit.
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{ids[0], SubElement::Whole}}, std::nullopt, Vec2{0, 0}}));
    auto before = sketch_->entity(ids[0]);
    ASSERT_TRUE(before);
    ASSERT_TRUE(sketch_->setConstruction(ids[0], true));
    expectGeometry(sketch_->entity(ids[0]).value().geometry, before.value().geometry);
    EXPECT_TRUE(sketch_->entity(ids[0]).value().construction);
    EXPECT_EQ(sketch_->constraintCount(), 1U);
    expectError(sketch_->setConstruction(EntityId(), true), ErrorCode::InvalidArgument);
    expectError(sketch_->setConstruction(EntityId(9999), true), ErrorCode::NotFound);
}

TEST_P(SketchBehavior, MovesAllGeometryKindsPreservingIdentityMetadataAndConstraintsWithoutSolving) {
    const std::array<EntityCreation, 4> input{
        {{Point2{{1, 2}}, true}, {Line2{{0, 0}, {2, 0}}, false}, {Circle2{{4, 5}, 2}, true}, {Arc2{{0, 0}, {3, 0}, {0, 3}}, false}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    auto untouched = sketch_->addPoint({20, 30});
    ASSERT_TRUE(untouched);
    auto length = sketch_->addConstraint({ConstraintType::Length, {{ids[1], SubElement::Whole}}, 10.0, std::nullopt});
    ASSERT_TRUE(length);
    ASSERT_TRUE(sketch_->moveEntities(ids, {5, -2}));
    // The line retains length 2 despite its length-10 constraint: no solve occurred.
    const std::array<SketchGeometry, 4> expected{{Point2{{6, 0}}, Line2{{5, -2}, {7, -2}}, Circle2{{9, 3}, 2}, Arc2{{5, -2}, {8, -2}, {5, 1}}}};
    for (size_t i = 0; i < ids.size(); ++i) {
        auto current = sketch_->entity(ids[i]);
        ASSERT_TRUE(current);
        EXPECT_EQ(current.value().id, ids[i]);
        EXPECT_EQ(current.value().construction, input[i].construction);
        expectGeometry(current.value().geometry, expected[i]);
    }
    expectGeometry(sketch_->entity(untouched.value()).value().geometry, Point2{{20, 30}});
    EXPECT_EQ(sketch_->entityCount(), 5U);
    EXPECT_EQ(sketch_->constraintCount(), 1U);
    auto constraint = sketch_->constraint(length.value());
    ASSERT_TRUE(constraint);
    EXPECT_EQ(constraint.value().definition.type, ConstraintType::Length);
    EXPECT_EQ(constraint.value().definition.refs, (std::vector<GeometryRef>{{ids[1], SubElement::Whole}}));
    ASSERT_TRUE(constraint.value().definition.value);
    EXPECT_DOUBLE_EQ(*constraint.value().definition.value, 10.0);
}

TEST_P(SketchBehavior, MoveRejectsInvalidOffsetsAndTargetsWithoutChangingGeometry) {
    auto added = sketch_->addEntity(Point2{{1, 2}}, true);
    ASSERT_TRUE(added);
    const auto id = added.value();
    const std::array ids{id};
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const std::array offsets{Vec2{nan, 0}, Vec2{0, nan}, Vec2{infinity, 0}, Vec2{0, -infinity}};
    for (auto offset : offsets) {
        expectError(sketch_->moveEntities(ids, offset), ErrorCode::InvalidArgument);
    }
    const std::array missing{id, EntityId(9999)};
    const std::array duplicate{id, id};
    const std::array invalid{id, EntityId()};
    expectError(sketch_->moveEntities(missing, {3, 4}), ErrorCode::NotFound);
    expectError(sketch_->moveEntities(duplicate, {3, 4}), ErrorCode::InvalidArgument);
    expectError(sketch_->moveEntities(invalid, {3, 4}), ErrorCode::InvalidArgument);
    // Zero offsets still validate targets; empty batches succeed without editing.
    expectError(sketch_->moveEntities(missing, {}), ErrorCode::NotFound);
    ASSERT_TRUE(sketch_->moveEntities(ids, {}));
    ASSERT_TRUE(sketch_->moveEntities({}, {nan, infinity}));
    auto current = sketch_->entity(id);
    ASSERT_TRUE(current);
    expectGeometry(current.value().geometry, Point2{{1, 2}});
    EXPECT_TRUE(current.value().construction);
}

TEST_P(SketchBehavior, MovePrevalidatesResultingGeometryBeforeMutatingAnyTarget) {
    const std::array<EntityCreation, 2> input{{{Point2{{1, 2}}, true}, {Line2{{0, 0}, {2, 0}}, false}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    // This finite offset collapses the line endpoints through floating-point rounding.
    const double huge = std::numeric_limits<double>::max();
    expectError(sketch_->moveEntities(added.value(), {huge, 0}), ErrorCode::InvalidArgument);
    for (size_t i = 0; i < input.size(); ++i) {
        auto current = sketch_->entity(added.value()[i]);
        ASSERT_TRUE(current);
        expectGeometry(current.value().geometry, input[i].geometry);
        EXPECT_EQ(current.value().construction, input[i].construction);
    }
}

TEST_P(SketchBehavior, GeometryBatchesPrevalidateAllInputsAndRejectDuplicateTargets) {
    auto point = sketch_->addEntity(Point2{{1, 2}}, true);
    ASSERT_TRUE(point);
    const auto id = point.value();
    auto state = sketch_->snapshot();
    ASSERT_TRUE(state);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const std::array<EntityCreation, 2> invalidCreation{{{Line2{{0, 0}, {2, 0}}}, {Circle2{{0, 0}, -1}}}};
    expectError(sketch_->addEntities(invalidCreation), ErrorCode::InvalidArgument);
    const std::vector<std::vector<EntityUpdate>> invalidUpdates{{{id, Point2{{3, 4}}, false}, {EntityId(9999), Point2{{5, 6}}, std::nullopt}},
                                                                {{id, Point2{{3, 4}}, false}, {id, Point2{{5, 6}}, std::nullopt}},
                                                                {{id, Point2{{nan, 4}}, false}},
                                                                {{id, Line2{{0, 0}, {2, 0}}, false}},
                                                                {{EntityId(), Point2{{3, 4}}, false}}};
    const std::array codes{ErrorCode::NotFound, ErrorCode::InvalidArgument, ErrorCode::InvalidArgument, ErrorCode::InvalidArgument, ErrorCode::InvalidArgument};
    for (size_t i = 0; i < invalidUpdates.size(); ++i) {
        expectError(sketch_->updateEntities(invalidUpdates[i]), codes[i]);
        expectGeometry(sketch_->entity(id).value().geometry, Point2{{1, 2}});
        EXPECT_TRUE(sketch_->entity(id).value().construction);
    }
    const std::array missing{id, EntityId(9999)};
    const std::array duplicate{id, id};
    const std::array invalid{id, EntityId()};
    expectError(sketch_->removeEntities(missing), ErrorCode::NotFound);
    expectError(sketch_->removeEntities(duplicate), ErrorCode::InvalidArgument);
    expectError(sketch_->removeEntities(invalid), ErrorCode::InvalidArgument);
    auto empty = sketch_->addEntities({});
    ASSERT_TRUE(empty);
    EXPECT_TRUE(empty.value().empty());
    ASSERT_TRUE(sketch_->updateEntities({}));
    ASSERT_TRUE(sketch_->removeEntities({}));
    auto after = sketch_->snapshot();
    ASSERT_TRUE(after);
    EXPECT_EQ(sketch_->entityCount(), 1U);
    EXPECT_EQ(after.value().lastEntityId, state.value().lastEntityId);
    EXPECT_EQ(after.value().lastConstraintId, state.value().lastConstraintId);
    // Both current adapters support every geometry kind; no valid creation input
    // exercises Unsupported. Unsupported constraint behavior is covered above.
}

TEST_P(SketchBehavior, BatchRemovalCascadesOnlyReferencingConstraints) {
    const std::array<EntityCreation, 3> input{{{Line2{{0, 0}, {2, 0}}}, {Point2{{2, 0}}}, {Line2{{10, 0}, {12, 0}}}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    auto joined = sketch_->addConstraint({ConstraintType::Coincident, {{ids[0], SubElement::End}, {ids[1], SubElement::Whole}}, std::nullopt, std::nullopt});
    auto removed = sketch_->addConstraint(horizontal(ids[0]));
    auto retained = sketch_->addConstraint(horizontal(ids[2]));
    ASSERT_TRUE(joined);
    ASSERT_TRUE(removed);
    ASSERT_TRUE(retained);
    EXPECT_EQ(sketch_->constraintCount(), 3U);
    const std::array removal{ids[1], ids[0]};
    ASSERT_TRUE(sketch_->removeEntities(removal));
    EXPECT_EQ(sketch_->entityCount(), 1U);
    EXPECT_EQ(sketch_->entityCount(EntityKind::Line).value(), 1U);
    EXPECT_EQ(sketch_->constraintCount(), 1U);
    expectError(sketch_->constraint(joined.value()), ErrorCode::NotFound);
    expectError(sketch_->constraint(removed.value()), ErrorCode::NotFound);
    EXPECT_TRUE(sketch_->constraint(retained.value()));
    auto next = sketch_->addPoint({0, 0});
    ASSERT_TRUE(next);
    EXPECT_GT(next.value().get(), ids.back().get());
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
}

TEST_P(SketchBehavior, BatchUpdatesDeferSolvingAndDefineCoincidentProposalOrder) {
    const std::array<EntityCreation, 3> input{{{Point2{{0, 0}}}, {Point2{{0, 0}}}, {Line2{{10, 0}, {14, 0}}}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {{ids[0], SubElement::Whole}, {ids[1], SubElement::Whole}}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch_->addConstraint(horizontal(ids[2])));
    // Reverse ID order intentionally: input order, rather than map order, wins.
    const std::array<EntityUpdate, 3> updates{
        {{ids[1], Point2{{2, 3}}, std::nullopt}, {ids[0], Point2{{5, 6}}, std::nullopt}, {ids[2], Line2{{10, 0}, {14, 2}}, std::nullopt}}};
    ASSERT_TRUE(sketch_->updateEntities(updates));
    expectGeometry(sketch_->entity(ids[2]).value().geometry, updates[2].geometry);
    expectPosition(sketch_->pointPosition({ids[0], SubElement::Whole}).value(), {5, 6});
    expectPosition(sketch_->pointPosition({ids[1], SubElement::Whole}).value(), GetParam() == BackendKind::Dcm ? Vec2{5, 6} : Vec2{2, 3});
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    ASSERT_EQ(solved.value().status, SolveStatus::Converged);
    expectPosition(sketch_->pointPosition({ids[0], SubElement::Whole}).value(), sketch_->pointPosition({ids[1], SubElement::Whole}).value());
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{ids[0], SubElement::Whole}}, std::nullopt, Vec2{1, 1}}));
    solved = sketch_->solve();
    ASSERT_TRUE(solved);
    ASSERT_EQ(solved.value().status, SolveStatus::Converged);
    const std::array<EntityUpdate, 2> fixedUpdates{{{ids[0], Point2{{7, 8}}, std::nullopt}, {ids[1], Point2{{9, 10}}, std::nullopt}}};
    ASSERT_TRUE(sketch_->updateEntities(fixedUpdates));
    expectPosition(sketch_->pointPosition({ids[0], SubElement::Whole}).value(), GetParam() == BackendKind::Dcm ? Vec2{1, 1} : Vec2{7, 8});
    expectPosition(sketch_->pointPosition({ids[1], SubElement::Whole}).value(), GetParam() == BackendKind::Dcm ? Vec2{1, 1} : Vec2{9, 10});
    solved = sketch_->solve();
    ASSERT_TRUE(solved);
    ASSERT_EQ(solved.value().status, SolveStatus::Converged);
    expectPosition(sketch_->pointPosition({ids[0], SubElement::Whole}).value(), {1, 1});
    expectPosition(sketch_->pointPosition({ids[1], SubElement::Whole}).value(), {1, 1});
}

TEST_P(SketchBehavior, MixedCurveBatchProposalsRespectCoincidentReferenceOrder) {
    const std::array<EntityCreation, 3> input{{{Line2{{0, 0}, {2, 0}}}, {Circle2{{0, 0}, 1}}, {Arc2{{0, 0}, {2, 0}, {0, 2}}}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {{ids[0], SubElement::Start}, {ids[1], SubElement::Center}}, std::nullopt, std::nullopt}));
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {{ids[1], SubElement::Center}, {ids[2], SubElement::Center}}, std::nullopt, std::nullopt}));
    const std::array<EntityUpdate, 3> updates{
        {{ids[2], Arc2{{1, 1}, {3, 1}, {1, 3}}, std::nullopt}, {ids[1], Circle2{{4, 4}, 2}, std::nullopt}, {ids[0], Line2{{5, 6}, {7, 6}}, std::nullopt}}};
    ASSERT_TRUE(sketch_->updateEntities(updates));
    const bool projected = GetParam() == BackendKind::Dcm;
    expectPosition(sketch_->pointPosition({ids[0], SubElement::Start}).value(), {5, 6});
    expectPosition(sketch_->pointPosition({ids[1], SubElement::Center}).value(), projected ? Vec2{5, 6} : Vec2{4, 4});
    expectPosition(sketch_->pointPosition({ids[2], SubElement::Center}).value(), projected ? Vec2{5, 6} : Vec2{1, 1});
    // Intrinsic circularity is deferred, too: the projected DCM arc now has
    // unequal radii. Both adapters retain the endpoint proposals without solve.
    expectPosition(sketch_->pointPosition({ids[2], SubElement::Start}).value(), {3, 1});
    expectPosition(sketch_->pointPosition({ids[2], SubElement::End}).value(), {1, 3});
    EXPECT_EQ(sketch_->entityCount(), 3U);
    EXPECT_EQ(sketch_->constraintCount(), 2U);
}

TEST_P(SketchBehavior, ClearRetainsBackendAndWatermarksWithoutAffectingOtherSketches) {
    ASSERT_TRUE(sketch_->clear());
    auto other = Sketch::create(GetParam());
    ASSERT_TRUE(other);
    auto otherPoint = other.value()->addPoint({20, 30});
    ASSERT_TRUE(otherPoint);
    ASSERT_TRUE(other.value()->addConstraint({ConstraintType::Fix, {{otherPoint.value(), SubElement::Whole}}, std::nullopt, Vec2{20, 30}}));
    const std::array<EntityCreation, 2> input{{{Arc2{{0, 0}, {2, 0}, {0, 2}}, true}, {Point2{{2, 0}}}}};
    auto added = sketch_->addEntities(input);
    ASSERT_TRUE(added);
    const auto& ids = added.value();
    auto joined = sketch_->addConstraint({ConstraintType::Coincident, {{ids[0], SubElement::Start}, {ids[1], SubElement::Whole}}, std::nullopt, std::nullopt});
    auto fixed = sketch_->addConstraint({ConstraintType::Fix, {{ids[1], SubElement::Whole}}, std::nullopt, Vec2{2, 0}});
    ASSERT_TRUE(joined);
    ASSERT_TRUE(fixed);
    auto before = sketch_->snapshot();
    ASSERT_TRUE(before);
    ASSERT_TRUE(sketch_->clear());
    ASSERT_TRUE(sketch_->clear());
    EXPECT_EQ(sketch_->backendKind(), GetParam());
    EXPECT_EQ(sketch_->entityCount(), 0U);
    EXPECT_EQ(sketch_->constraintCount(), 0U);
    EXPECT_TRUE(sketch_->entities().value().empty());
    EXPECT_TRUE(sketch_->constraints().value().empty());
    EXPECT_TRUE(sketch_->pointElements().value().empty());
    for (auto kind : {EntityKind::Point, EntityKind::Line, EntityKind::Circle, EntityKind::Arc}) {
        EXPECT_EQ(sketch_->entityCount(kind).value(), 0U);
    }
    auto after = sketch_->snapshot();
    ASSERT_TRUE(after);
    EXPECT_EQ(after.value().lastEntityId, before.value().lastEntityId);
    EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
    expectError(sketch_->entity(ids[0]), ErrorCode::NotFound);
    expectError(sketch_->constraint(fixed.value()), ErrorCode::NotFound);
    EXPECT_EQ(other.value()->entityCount(), 1U);
    EXPECT_EQ(other.value()->constraintCount(), 1U);
    expectPosition(other.value()->pointPosition({otherPoint.value(), SubElement::Whole}).value(), {20, 30});
    auto line = sketch_->addLine({10, 10}, {14, 12});
    ASSERT_TRUE(line);
    auto relation = sketch_->addConstraint(horizontal(line.value()));
    ASSERT_TRUE(relation);
    EXPECT_GT(line.value().get(), before.value().lastEntityId);
    EXPECT_GT(relation.value().get(), before.value().lastConstraintId);
    auto solved = sketch_->solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
    EXPECT_EQ(sketch_->entityCount(), 1U);
    EXPECT_EQ(sketch_->constraintCount(), 1U);
}

TEST_P(SketchBehavior, BatchIdCapacityIsCheckedBeforeAllocationAndClearDoesNotResetIt) {
    SketchSnapshot state;
    state.lastEntityId = std::numeric_limits<int64_t>::max() - 1;
    ASSERT_TRUE(sketch_->replaceState(state));
    const std::array<EntityCreation, 2> tooMany{{{Point2{{0, 0}}}, {Point2{{1, 1}}}}};
    expectError(sketch_->addEntities(tooMany), ErrorCode::BackendFailure);
    EXPECT_EQ(sketch_->entityCount(), 0U);
    auto last = sketch_->addPoint({0, 0});
    ASSERT_TRUE(last);
    EXPECT_EQ(last.value().get(), std::numeric_limits<int64_t>::max());
    ASSERT_TRUE(sketch_->clear());
    expectError(sketch_->addEntities(tooMany), ErrorCode::BackendFailure);
    EXPECT_EQ(sketch_->entityCount(), 0U);
    EXPECT_EQ(sketch_->snapshot().value().lastEntityId, std::numeric_limits<int64_t>::max());
}

INSTANTIATE_TEST_SUITE_P(AvailableBackends, SketchBehavior, testing::ValuesIn(Sketch::availableBackends()),
                         [](const testing::TestParamInfo<BackendKind>& info) { return info.param == BackendKind::Dcm ? "DCM" : "SolveSpace"; });

TEST(SketchConfiguration, DefaultBackendIsAvailableAndConstructible) {
    const auto available = Sketch::availableBackends();
    EXPECT_FALSE(available.empty());
    EXPECT_NE(std::find(available.begin(), available.end(), Sketch::defaultBackend()), available.end());
    auto created = Sketch::create();
    ASSERT_TRUE(created);
    EXPECT_EQ(created.value()->backendKind(), Sketch::defaultBackend());
    for (BackendKind backend : {BackendKind::Dcm, BackendKind::SolveSpace}) {
        if (std::find(available.begin(), available.end(), backend) == available.end()) {
            expectError(Sketch::create(backend), ErrorCode::Unsupported);
        }
    }
}

TEST(SketchStateTransfer, UnsupportedMigrationPreservesSourceBackendAndState) {
    const auto available = Sketch::availableBackends();
    if (std::find(available.begin(), available.end(), BackendKind::Dcm) == available.end() ||
        std::find(available.begin(), available.end(), BackendKind::SolveSpace) == available.end()) {
        GTEST_SKIP() << "Requires both backends";
    }
    auto created = Sketch::create(BackendKind::SolveSpace);
    ASSERT_TRUE(created);
    auto& sketch = *created.value();
    auto circle = sketch.addCircle({2, 3}, 4);
    ASSERT_TRUE(circle);
    ConstraintDefinition radius{ConstraintType::Radius, {{circle.value(), SubElement::Whole}}, 4.0, std::nullopt};
    auto relation = sketch.addConstraint(radius);
    ASSERT_TRUE(relation);
    expectError(sketch.switchBackend(BackendKind::Dcm), ErrorCode::Unsupported);
    EXPECT_EQ(sketch.backendKind(), BackendKind::SolveSpace);
    auto queried = sketch.entity(circle.value());
    auto queriedConstraint = sketch.constraint(relation.value());
    ASSERT_TRUE(queried);
    ASSERT_TRUE(queriedConstraint);
    expectGeometry(queried.value().geometry, Circle2{{2, 3}, 4});
    EXPECT_EQ(queriedConstraint.value().definition.type, ConstraintType::Radius);
    auto solved = sketch.solve();
    ASSERT_TRUE(solved);
    EXPECT_EQ(solved.value().status, SolveStatus::Converged);
}

}  // namespace
}  // namespace core::sketch
