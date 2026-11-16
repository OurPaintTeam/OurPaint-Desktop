#include <gtest/gtest.h>

#include <cmath>
#include <initializer_list>
#include <limits>
#include <memory>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

#include "../editor/ConstraintActions.h"
#include "sketch/Sketch.h"

namespace {
using namespace core::sketch;
using PreparationState = ConstraintPreparation::State;

class ConstraintActionsBehavior : public testing::Test {
protected:
    void SetUp() override {
        sketch_ = std::make_unique<Sketch>();

        const auto line = sketch_->addLine({0, 0}, {3, 4});
        const auto otherLine = sketch_->addLine({10, 0}, {12, 3});
        const auto point = sketch_->addPoint({9, 8});
        const auto circle = sketch_->addCircle({5, 5}, 2);
        const auto arc = sketch_->addArc({0, 10}, {2, 10}, {0, 12});
        ASSERT_TRUE(line);
        ASSERT_TRUE(otherLine);
        ASSERT_TRUE(point);
        ASSERT_TRUE(circle);
        ASSERT_TRUE(arc);
        line_ = {line.value(), SubElement::Whole};
        otherLine_ = {otherLine.value(), SubElement::Whole};
        point_ = {point.value(), SubElement::Whole};
        circle_ = {circle.value(), SubElement::Whole};
        arc_ = {arc.value(), SubElement::Whole};
        start_ = {line.value(), SubElement::Start};
        end_ = {line.value(), SubElement::End};
    }

    ConstraintPreparation prepare(ConstraintAction action, std::initializer_list<GeometryRef> refs = {}, std::optional<double> value = std::nullopt) {
        return ConstraintActions(*sketch_).prepare({action, value}, {refs.begin(), refs.size()});
    }

    std::unique_ptr<Sketch> sketch_;
    GeometryRef line_, otherLine_, point_, circle_, arc_, start_, end_;
};

TEST_F(ConstraintActionsBehavior, NurbsControlsRemainDistinctPointOperands) {
    const auto curve = sketch_->addEntity(NurbsCurve2{3, {{{0, 0}}, {{2, 3}}, {{4, -2}}, {{6, 0}}}, {1, 1, 1, 1}, {0, 0, 0, 0, 1, 1, 1, 1}});
    ASSERT_TRUE(curve);
    const GeometryRef first{curve.value(), SubElement::Control, 1};
    const GeometryRef second{curve.value(), SubElement::Control, 2};
    const auto coincidence = prepare(ConstraintAction::Coincident, {first, second});
    ASSERT_EQ(coincidence.state, PreparationState::Ready);
    ASSERT_TRUE(coincidence.definition);
    EXPECT_EQ(coincidence.definition->refs, (std::vector<GeometryRef>{first, second}));
    EXPECT_EQ(prepare(ConstraintAction::Fix, {second}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Fix, {{curve.value(), SubElement::Control, 4}}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Fix, {{line_.entity, SubElement::Start, 1}}).state, PreparationState::InvalidSelection);
}

TEST_F(ConstraintActionsBehavior, ConcentricAcceptsWholeCircularPairsAndPartialInput) {
    const auto otherCircle = sketch_->addCircle({-5, -5}, 3);
    const auto otherArc = sketch_->addArc({-4, 10}, {-1, 10}, {-4, 13});
    ASSERT_TRUE(otherCircle);
    ASSERT_TRUE(otherArc);
    const GeometryRef circle{otherCircle.value(), SubElement::Whole};
    const GeometryRef arc{otherArc.value(), SubElement::Whole};
    EXPECT_EQ(prepare(ConstraintAction::Concentric).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Concentric, {circle_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Concentric, {arc_}).state, PreparationState::NeedsMoreInput);
    for (const auto& refs : {std::vector<GeometryRef>{circle_, circle}, std::vector<GeometryRef>{circle, circle_}, std::vector<GeometryRef>{arc_, arc},
                             std::vector<GeometryRef>{arc, arc_}, std::vector<GeometryRef>{circle_, arc_}, std::vector<GeometryRef>{arc_, circle_}}) {
        const auto prepared = ConstraintActions(*sketch_).prepare({ConstraintAction::Concentric, std::nullopt}, refs);
        ASSERT_EQ(prepared.state, PreparationState::Ready);
        ASSERT_TRUE(prepared.definition);
        EXPECT_EQ(prepared.definition->type, ConstraintType::Concentric);
        EXPECT_EQ(prepared.definition->refs, refs);
        EXPECT_FALSE(prepared.definition->value);
        EXPECT_FALSE(prepared.definition->fixedPosition);
    }
}

TEST_F(ConstraintActionsBehavior, SymmetricNormalizesAxisAtAnySelectionPosition) {
    for (const auto& refs : {std::vector<GeometryRef>{otherLine_, start_, point_}, std::vector<GeometryRef>{start_, otherLine_, point_},
                             std::vector<GeometryRef>{start_, point_, otherLine_}}) {
        const auto prepared = ConstraintActions(*sketch_).prepare({ConstraintAction::Symmetric, std::nullopt}, refs);
        ASSERT_EQ(prepared.state, PreparationState::Ready);
        ASSERT_TRUE(prepared.definition);
        EXPECT_EQ(prepared.definition->type, ConstraintType::Symmetric);
        EXPECT_EQ(prepared.definition->refs, (std::vector<GeometryRef>{start_, point_, otherLine_}));
        EXPECT_FALSE(prepared.definition->value);
        EXPECT_FALSE(prepared.definition->fixedPosition);
    }
}

TEST_F(ConstraintActionsBehavior, MidpointNormalizesEitherOrderAndAcceptsPointElements) {
    EXPECT_EQ(prepare(ConstraintAction::Midpoint).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Midpoint, {line_}).state, PreparationState::NeedsMoreInput);
    for (const auto point : {point_, GeometryRef{otherLine_.entity, SubElement::Start}, GeometryRef{otherLine_.entity, SubElement::End},
                             GeometryRef{circle_.entity, SubElement::Center}, GeometryRef{arc_.entity, SubElement::Start},
                             GeometryRef{arc_.entity, SubElement::End}, GeometryRef{arc_.entity, SubElement::Center}}) {
        EXPECT_EQ(prepare(ConstraintAction::Midpoint, {point}).state, PreparationState::NeedsMoreInput);
        for (const auto& refs : {std::vector<GeometryRef>{point, line_}, std::vector<GeometryRef>{line_, point}}) {
            const auto prepared = ConstraintActions(*sketch_).prepare({ConstraintAction::Midpoint, std::nullopt}, refs);
            ASSERT_EQ(prepared.state, PreparationState::Ready);
            ASSERT_TRUE(prepared.definition);
            EXPECT_EQ(prepared.definition->type, ConstraintType::Midpoint);
            EXPECT_EQ(prepared.definition->refs, (std::vector<GeometryRef>{point, line_}));
            EXPECT_FALSE(prepared.definition->value);
            EXPECT_FALSE(prepared.definition->fixedPosition);
        }
    }
}

TEST_F(ConstraintActionsBehavior, SymmetricKeepsLastPointAsCenterAndAcceptsCurvePointElements) {
    const GeometryRef circleCenter{circle_.entity, SubElement::Center};
    const GeometryRef arcEnd{arc_.entity, SubElement::End};
    const auto prepared = prepare(ConstraintAction::Symmetric, {start_, arcEnd, circleCenter});
    ASSERT_EQ(prepared.state, PreparationState::Ready);
    ASSERT_TRUE(prepared.definition);
    EXPECT_EQ(prepared.definition->type, ConstraintType::Symmetric);
    EXPECT_EQ(prepared.definition->refs, (std::vector<GeometryRef>{start_, arcEnd, circleCenter}));
    EXPECT_FALSE(prepared.definition->value);
    EXPECT_FALSE(prepared.definition->fixedPosition);
}

TEST_F(ConstraintActionsBehavior, SymmetricDistinguishesPartialInputFromInvalidSelections) {
    EXPECT_EQ(prepare(ConstraintAction::Symmetric).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {point_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {line_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {start_, point_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {line_, point_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {point_, line_}).state, PreparationState::NeedsMoreInput);

    for (const auto& refs :
         {std::vector<GeometryRef>{line_, otherLine_}, std::vector<GeometryRef>{line_, point_, otherLine_}, std::vector<GeometryRef>{start_, start_},
          std::vector<GeometryRef>{start_, start_, point_}, std::vector<GeometryRef>{start_, point_, start_}, std::vector<GeometryRef>{point_, start_, start_},
          std::vector<GeometryRef>{circle_}, std::vector<GeometryRef>{start_, point_, arc_}, std::vector<GeometryRef>{start_, end_, point_, otherLine_}}) {
        const auto prepared = ConstraintActions(*sketch_).prepare({ConstraintAction::Symmetric, std::nullopt}, refs);
        EXPECT_EQ(prepared.state, PreparationState::InvalidSelection);
        EXPECT_EQ(prepared.rejection, ActionRejection::InvalidSelection);
        EXPECT_FALSE(prepared.definition);
    }
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {start_, end_, point_}, 1.0).rejection, ActionRejection::InvalidValue);
    const auto missing = prepare(ConstraintAction::Symmetric, {start_, point_, {EntityId(9999), SubElement::Whole}});
    EXPECT_EQ(missing.state, PreparationState::InvalidSelection);
    ASSERT_TRUE(missing.error);
    EXPECT_EQ(missing.error->code, ErrorCode::NotFound);
}

TEST_F(ConstraintActionsBehavior, EqualityAndTangencyValidateSelections) {
    EXPECT_EQ(prepare(ConstraintAction::Equal).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Equal, {line_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Equal, {circle_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Equal, {line_, otherLine_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Equal, {circle_, arc_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Equal, {line_, circle_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Tangent).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Tangent, {line_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Tangent, {line_, circle_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Tangent, {circle_, arc_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Tangent, {line_, otherLine_}).state, PreparationState::InvalidSelection);
}

TEST_F(ConstraintActionsBehavior, RejectionsPreserveSelectionAndValueReasonsWithoutMutation) {
    const auto invalidValue = prepare(ConstraintAction::Dimension, {line_}, -1.0);
    EXPECT_EQ(invalidValue.rejection, ActionRejection::InvalidValue);
    const auto invalidSelection = prepare(ConstraintAction::Horizontal, {point_});
    EXPECT_EQ(invalidSelection.rejection, ActionRejection::InvalidSelection);
    const auto unsupported = prepare(ConstraintAction::Unsupported);
    EXPECT_EQ(unsupported.state, PreparationState::Unsupported);
    EXPECT_EQ(unsupported.rejection, ActionRejection::Unsupported);

    const auto missing = prepare(ConstraintAction::Horizontal, {{EntityId(9999), SubElement::Whole}});
    ASSERT_TRUE(missing.error);
    EXPECT_EQ(missing.error->code, ErrorCode::NotFound);
    const auto report = ConstraintActions::rejectedReport(missing);
    EXPECT_EQ(report.change, ModelChange::Unchanged);
    EXPECT_TRUE(report.constraintIds.empty());
    EXPECT_FALSE(report.solve);
    ASSERT_TRUE(report.operationError);
    EXPECT_EQ(report.operationError->code, missing.error->code);
    EXPECT_EQ(report.operationError->message, missing.error->message);
    EXPECT_EQ(sketch_->constraintCount(), 0);
}

TEST_F(ConstraintActionsBehavior, ApplicationPreflightFailureDoesNotClaimInsertionOrSolving) {
    ConstraintActions actions(*sketch_);
    const ConstraintDefinition invalid{ConstraintType::Coincident, {point_}, std::nullopt, std::nullopt};
    const auto report = actions.apply(invalid);
    EXPECT_EQ(report.change, ModelChange::Unchanged);
    EXPECT_TRUE(report.constraintIds.empty());
    ASSERT_TRUE(report.operationError);
    EXPECT_EQ(report.operationError->code, ErrorCode::InvalidArgument);
    EXPECT_FALSE(report.solve);
    EXPECT_FALSE(report.solveError);
    EXPECT_EQ(sketch_->constraintCount(), 0);
}

TEST(ConstraintActionsReports, FailedReadOnlyPreparationKeepsModelUnchanged) {
    ConstraintPreparation preparation{PreparationState::InvalidSelection, std::nullopt, ActionRejection::InvalidSelection,
                                      SketchError{ErrorCode::BackendFailure, "Read-only geometry query failed"}};
    const auto report = ConstraintActions::rejectedReport(preparation);
    EXPECT_EQ(report.change, ModelChange::Unchanged);
    ASSERT_TRUE(report.operationError);
    EXPECT_EQ(report.operationError->code, ErrorCode::BackendFailure);
    EXPECT_TRUE(report.constraintIds.empty());
    EXPECT_FALSE(report.solve);
}

}  // namespace
