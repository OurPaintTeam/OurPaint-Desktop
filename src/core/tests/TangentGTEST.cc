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

TEST(TangentConstraint, InfersSignedLineSideWithEitherRefOrderAndLineDirectionForCircleAndArcSupports) {
    for (bool arc : {false, true}) {
        for (bool below : {false, true}) {
            for (bool reverseLine : {false, true}) {
                for (bool reverseRefs : {false, true}) {
                    Sketch sketch;
                    const Vec2 start{-4, 0}, end{4, 0};
                    const auto line = sketch.addLine(reverseLine ? end : start, reverseLine ? start : end);
                    const auto curve = sketch.addEntity(circularGeometry({0, below ? -2.0 : 2.0}, 1, arc));
                    ASSERT_TRUE(line);
                    ASSERT_TRUE(curve);
                    ASSERT_TRUE(sketch.capabilities().supports(ConstraintType::Tangent));
                    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{line.value()}}, std::nullopt, std::nullopt}));
                    const auto definition = reverseRefs ? tangent(curve.value(), line.value()) : tangent(line.value(), curve.value());
                    ASSERT_TRUE(sketch.supportsConstraint(definition));
                    const auto added = sketch.addConstraint(definition);
                    ASSERT_TRUE(added);
                    const auto branch = below != reverseLine ? TangencyBranch::NegativeSide : TangencyBranch::PositiveSide;
                    expectBranch(sketch, added.value(), branch);
                    expectConverged(sketch.solve());
                    expectLineTangency(sketch, line.value(), curve.value(), branch);
                }
            }
        }
    }
}

TEST(TangentConstraint, InfersAndSolvesAllCircularSupportPairingsAndBranches) {
    for (bool firstArc : {false, true}) {
        for (bool secondArc : {false, true}) {
            for (const auto branch : {TangencyBranch::External, TangencyBranch::FirstContainsSecond, TangencyBranch::SecondContainsFirst}) {
                Sketch sketch;
                const double firstRadius = branch == TangencyBranch::SecondContainsFirst ? 1 : 3;
                const double secondRadius = branch == TangencyBranch::SecondContainsFirst ? 3 : 1;
                const double distance = branch == TangencyBranch::External ? 4.3 : 2.3;
                const auto first = sketch.addEntity(circularGeometry({0, 0}, firstRadius, firstArc));
                const auto second = sketch.addEntity(circularGeometry({distance, 0}, secondRadius, secondArc));
                ASSERT_TRUE(first);
                ASSERT_TRUE(second);
                const auto definition = tangent(first.value(), second.value());
                ASSERT_TRUE(sketch.supportsConstraint(definition));
                const auto added = sketch.addConstraint(definition);
                ASSERT_TRUE(added);
                expectBranch(sketch, added.value(), branch);
                expectConverged(sketch.solve());
                expectCircularTangency(sketch, first.value(), second.value(), branch);
            }
        }
    }
}

TEST(TangentConstraint, SameReferencesPreserveBranchRetargetingInfersAndSnapshotRetainsExplicitChoice) {
    Sketch sketch;
    const auto line = sketch.addLine({-4, 0}, {4, 0});
    const auto first = sketch.addCircle({0, 1}, 1);
    const auto second = sketch.addCircle({0, -1}, 1);
    ASSERT_TRUE(line);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const auto added = sketch.addConstraint(tangent(line.value(), first.value()));
    ASSERT_TRUE(added);
    expectBranch(sketch, added.value(), TangencyBranch::PositiveSide);
    ASSERT_TRUE(sketch.updateCircle(first.value(), {{0, -1}, 1}));
    ASSERT_TRUE(sketch.updateConstraint(added.value(), tangent(line.value(), first.value())));
    expectBranch(sketch, added.value(), TangencyBranch::PositiveSide);
    const auto state = sketch.snapshot();
    ASSERT_TRUE(state);
    Sketch restored;
    ASSERT_TRUE(restored.replaceState(state.value()));
    const auto transferred = restored.snapshot();
    ASSERT_TRUE(transferred);
    EXPECT_EQ(transferred.value().lastEntityId, state.value().lastEntityId);
    EXPECT_EQ(transferred.value().lastConstraintId, state.value().lastConstraintId);
    expectBranch(restored, added.value(), TangencyBranch::PositiveSide);
    ASSERT_TRUE(restored.updateConstraint(added.value(), tangent(line.value(), second.value())));
    expectBranch(restored, added.value(), TangencyBranch::NegativeSide);
    ASSERT_TRUE(restored.updateConstraint(added.value(), tangent(line.value(), second.value(), TangencyBranch::PositiveSide)));
    expectBranch(restored, added.value(), TangencyBranch::PositiveSide);
    ASSERT_TRUE(restored.removeConstraint(added.value()));
    expectError(restored.constraint(added.value()), ErrorCode::NotFound);
    ASSERT_TRUE(restored.updateCircle(second.value(), {{0, -3}, 1}));
    expectConverged(restored.solve());
    const auto position = restored.pointPosition({second.value(), SubElement::Center});
    ASSERT_TRUE(position);
    EXPECT_EQ(position.value(), (Vec2{0, -3}));
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

TEST(TangentConstraint, RejectsWrongBranchesAndFieldsBeforeMutationOrIdAllocation) {
    Sketch sketch;
    const auto line = sketch.addLine({-4, 0}, {4, 0});
    const auto first = sketch.addCircle({0, 1}, 1);
    const auto second = sketch.addCircle({4, 1}, 1);
    ASSERT_TRUE(line);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const auto added = sketch.addConstraint(tangent(line.value(), first.value()));
    ASSERT_TRUE(added);
    const auto before = sketch.snapshot();
    ASSERT_TRUE(before);
    auto dimensional = tangent(line.value(), first.value());
    dimensional.value = 1;
    auto fixed = tangent(line.value(), first.value());
    fixed.fixedPosition = Vec2{0, 0};
    auto subelement = tangent(line.value(), first.value());
    subelement.refs[1].sub = SubElement::Center;
    const std::array cases{tangent(line.value(), first.value(), TangencyBranch::External),
                           tangent(line.value(), first.value(), TangencyBranch::FirstContainsSecond),
                           tangent(first.value(), second.value(), TangencyBranch::PositiveSide),
                           tangent(first.value(), second.value(), TangencyBranch::NegativeSide),
                           tangent(first.value(), second.value(), TangencyBranch::FirstContainsSecond),
                           tangent(first.value(), second.value(), TangencyBranch::SecondContainsFirst),
                           tangent(line.value(), first.value(), static_cast<TangencyBranch>(999)),
                           ConstraintDefinition{ConstraintType::Horizontal, {{line.value()}}, std::nullopt, std::nullopt, TangencyBranch::PositiveSide},
                           dimensional,
                           fixed,
                           subelement};
    for (const auto& definition : cases) {
        expectError(sketch.supportsConstraint(definition));
        expectError(sketch.addConstraint(definition));
        expectError(sketch.updateConstraint(added.value(), definition));
        const auto after = sketch.snapshot();
        ASSERT_TRUE(after);
        EXPECT_EQ(after.value().lastEntityId, before.value().lastEntityId);
        EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
        ASSERT_EQ(after.value().entities.size(), 3U);
        EXPECT_EQ(std::get<Line2>(after.value().entities[0].geometry).start, (Vec2{-4, 0}));
        EXPECT_EQ(std::get<Line2>(after.value().entities[0].geometry).end, (Vec2{4, 0}));
        EXPECT_EQ(std::get<Circle2>(after.value().entities[1].geometry).center, (Vec2{0, 1}));
        EXPECT_DOUBLE_EQ(std::get<Circle2>(after.value().entities[1].geometry).radius, 1);
        EXPECT_EQ(std::get<Circle2>(after.value().entities[2].geometry).center, (Vec2{4, 1}));
        ASSERT_EQ(after.value().constraints.size(), 1U);
        EXPECT_EQ(after.value().constraints[0].id, added.value());
        EXPECT_EQ(after.value().constraints[0].definition.refs, before.value().constraints[0].definition.refs);
        expectBranch(sketch, added.value(), TangencyBranch::PositiveSide);
    }
    const auto next = sketch.addConstraint(tangent(first.value(), second.value()));
    ASSERT_TRUE(next);
    EXPECT_EQ(next.value().get(), before.value().lastConstraintId + 1);
}

class TangentDrag : public testing::Test {
protected:
    void SetUp() override {
        const auto lineResult = sketch.addLine({-4, 0}, {4, 0});
        const auto circleResult = sketch.addCircle({0, 1}, 1);
        const auto reference = sketch.addCircle({10, 10}, 1);
        ASSERT_TRUE(lineResult);
        ASSERT_TRUE(circleResult);
        ASSERT_TRUE(reference);
        line = lineResult.value();
        circle = circleResult.value();
        const auto fixed = sketch.addConstraint({ConstraintType::Fix, {{line}}, std::nullopt, std::nullopt});
        ASSERT_TRUE(fixed);
        fixedLine = fixed.value();
        ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{reference.value()}}, std::nullopt, std::nullopt}));
        ASSERT_TRUE(sketch.addConstraint({ConstraintType::Equal, {{circle}, {reference.value()}}, std::nullopt, std::nullopt}));
        const auto added = sketch.addConstraint(tangent(line, circle));
        ASSERT_TRUE(added);
        relation = added.value();
    }

    Sketch sketch;
    EntityId line, circle;
    ConstraintId fixedLine, relation;
};

TEST_F(TangentDrag, DefaultsToPreservingBranchAndExplicitPolicySwitchesItAfterCrossing) {
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, -2}}));
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::PositiveSide);
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, -2}}, {TangencyDragPolicy::SwitchLineSide, 0}));
    expectBranch(sketch, relation, TangencyBranch::NegativeSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::NegativeSide);
}

TEST_F(TangentDrag, UsesDistanceHysteresisOnBothSidesOfTheLine) {
    const DragOptions options{TangencyDragPolicy::SwitchLineSide, 0.25};
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, -0.1}}, options));
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, -0.5}}, options));
    expectBranch(sketch, relation, TangencyBranch::NegativeSide);
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, 0.1}}, options));
    expectBranch(sketch, relation, TangencyBranch::NegativeSide);
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, 0.5}}, options));
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::PositiveSide);
}

TEST_F(TangentDrag, ShortLineStillSwitchesSideWhenCenterCrossesDistanceThreshold) {
    ASSERT_TRUE(sketch.removeConstraint(fixedLine));
    ASSERT_TRUE(sketch.updateLine(line, {{-0.05, 0}, {0.05, 0}}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{line}}, std::nullopt, std::nullopt}));
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, -0.5}}, {TangencyDragPolicy::SwitchLineSide, 0.25}));
    expectBranch(sketch, relation, TangencyBranch::NegativeSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::NegativeSide);
}

TEST_F(TangentDrag, FixedCircleIgnoresRawCenterTargetWhenChoosingSide) {
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{circle}}, std::nullopt, std::nullopt}));
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, -3}}, {TangencyDragPolicy::SwitchLineSide, 0}));
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::PositiveSide);
}

TEST_F(TangentDrag, FixedCoincidenceGroupIgnoresRawCenterTargetWhenChoosingSide) {
    const auto anchor = sketch.addPoint({0, 1});
    ASSERT_TRUE(anchor);
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{anchor.value()}}, std::nullopt, Vec2{0, 1}}));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Coincident, {{circle, SubElement::Center}, {anchor.value()}}, std::nullopt, std::nullopt}));
    expectConverged(sketch.drag({{circle, SubElement::Center}, {0, -3}}, {TangencyDragPolicy::SwitchLineSide, 0}));
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::PositiveSide);
}

TEST_F(TangentDrag, FixedLineIgnoresRawEndpointTargetsWhenChoosingSide) {
    const std::array requests{DragRequest{{line, SubElement::Start}, {-4, 2}}, DragRequest{{line, SubElement::End}, {4, 2}}};
    expectConverged(sketch.dragPoints(requests, {TangencyDragPolicy::SwitchLineSide, 0}));
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::PositiveSide);
}

TEST_F(TangentDrag, MovingLineAcrossFixedCircleCenterSwitchesSide) {
    ASSERT_TRUE(sketch.removeConstraint(fixedLine));
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{circle}}, std::nullopt, std::nullopt}));
    const std::array requests{DragRequest{{line, SubElement::Start}, {-4, 2}}, DragRequest{{line, SubElement::End}, {4, 2}}};
    expectConverged(sketch.dragPoints(requests, {TangencyDragPolicy::SwitchLineSide, 0}));
    expectBranch(sketch, relation, TangencyBranch::NegativeSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::NegativeSide);
    const auto center = sketch.pointPosition({circle, SubElement::Center});
    ASSERT_TRUE(center);
    EXPECT_EQ(center.value(), (Vec2{0, 1}));
}

TEST_F(TangentDrag, JointTranslationUsesAllProposedCoordinatesWithoutFlippingSide) {
    ASSERT_TRUE(sketch.removeConstraint(fixedLine));
    const std::array requests{DragRequest{{line, SubElement::Start}, {-4, 3}}, DragRequest{{line, SubElement::End}, {4, 3}},
                              DragRequest{{circle, SubElement::Center}, {0, 4}}};
    expectConverged(sketch.dragPoints(requests, {TangencyDragPolicy::SwitchLineSide, 0}));
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    expectLineTangency(sketch, line, circle, TangencyBranch::PositiveSide);
}

TEST_F(TangentDrag, DegenerateProposedLinePreservesBranchAndReportsFailedTangency) {
    ASSERT_TRUE(sketch.removeConstraint(fixedLine));
    const std::array requests{DragRequest{{line, SubElement::Start}, {0, 2}}, DragRequest{{line, SubElement::End}, {0, 2}}};
    const auto dragged = sketch.dragPoints(requests, {TangencyDragPolicy::SwitchLineSide, 0});
    ASSERT_TRUE(dragged);
    EXPECT_EQ(dragged.value().status, SolveStatus::Failed);
    expectBranch(sketch, relation, TangencyBranch::PositiveSide);
}

TEST_F(TangentDrag, RejectsInvalidOptionsBeforeChangingGeometryOrBranch) {
    const std::array options{
        DragOptions{TangencyDragPolicy::SwitchLineSide, -1}, DragOptions{TangencyDragPolicy::SwitchLineSide, std::numeric_limits<double>::quiet_NaN()},
        DragOptions{TangencyDragPolicy::SwitchLineSide, std::numeric_limits<double>::infinity()}, DragOptions{static_cast<TangencyDragPolicy>(999), 0}};
    for (const auto& option : options) {
        expectError(sketch.drag({{circle, SubElement::Center}, {0, -3}}, option));
        const auto position = sketch.pointPosition({circle, SubElement::Center});
        ASSERT_TRUE(position);
        EXPECT_EQ(position.value(), (Vec2{0, 1}));
        expectBranch(sketch, relation, TangencyBranch::PositiveSide);
    }
}

TEST(TangentDragPolicy, SwitchingLineSideDoesNotReclassifyCircularTangency) {
    Sketch sketch;
    const auto first = sketch.addCircle({0, 0}, 3);
    const auto second = sketch.addCircle({4, 0}, 1);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(sketch.addConstraint({ConstraintType::Fix, {{first.value()}}, std::nullopt, std::nullopt}));
    const auto added = sketch.addConstraint(tangent(first.value(), second.value()));
    ASSERT_TRUE(added);
    const auto dragged = sketch.drag({{second.value(), SubElement::Center}, {1, 0}}, {TangencyDragPolicy::SwitchLineSide, 0});
    ASSERT_TRUE(dragged);
    expectBranch(sketch, added.value(), TangencyBranch::External);
    if (dragged.value().status == SolveStatus::Converged) {
        expectCircularTangency(sketch, first.value(), second.value(), TangencyBranch::External);
    }
}

}  // namespace
}  // namespace core::sketch
