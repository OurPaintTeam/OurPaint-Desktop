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

TEST_F(ConstraintActionsBehavior, DimensionDistinguishesWholeLineFromItsEndpoints) {
    const auto length = prepare(ConstraintAction::Dimension, {line_}, 7.0);
    ASSERT_EQ(length.state, PreparationState::Ready);
    ASSERT_TRUE(length.definition);
    EXPECT_EQ(length.definition->type, ConstraintType::Length);
    ASSERT_EQ(length.definition->refs.size(), 1);
    EXPECT_EQ(length.definition->refs[0], line_);
    EXPECT_EQ(length.definition->value, 7.0);

    const auto incomplete = prepare(ConstraintAction::Dimension, {end_}, 7.0);
    EXPECT_EQ(incomplete.state, PreparationState::NeedsMoreInput);
    EXPECT_FALSE(incomplete.definition);

    const auto distance = prepare(ConstraintAction::Dimension, {start_, end_}, 7.0);
    ASSERT_EQ(distance.state, PreparationState::Ready);
    ASSERT_TRUE(distance.definition);
    EXPECT_EQ(distance.definition->type, ConstraintType::Distance);
    ASSERT_EQ(distance.definition->refs.size(), 2);
    EXPECT_EQ(distance.definition->refs[0], start_);
    EXPECT_EQ(distance.definition->refs[1], end_);

    EXPECT_EQ(prepare(ConstraintAction::Dimension, {end_, point_}, 7.0).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {line_, point_}, 7.0).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {point_, line_}, 7.0).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {circle_}, 7.0).state, PreparationState::InvalidSelection);
}

TEST_F(ConstraintActionsBehavior, BinaryConstraintDistinguishesIncompleteAndInvalidSelections) {
    EXPECT_EQ(prepare(ConstraintAction::Perpendicular).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Perpendicular, {line_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Perpendicular, {line_, otherLine_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Perpendicular, {start_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Perpendicular, {line_, point_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Perpendicular, {line_, line_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Perpendicular, {line_, otherLine_, point_}).state, PreparationState::InvalidSelection);

    EXPECT_EQ(prepare(ConstraintAction::Coincident, {start_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Coincident, {start_, end_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Coincident, {start_, start_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Coincident, {line_}).state, PreparationState::InvalidSelection);
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

TEST_F(ConstraintActionsBehavior, ConcentricRejectsNonWholeCircularReferencesWithoutMutation) {
    const auto before = sketch_->snapshot();
    const auto pointsBefore = sketch_->pointElements();
    ASSERT_TRUE(before);
    ASSERT_TRUE(pointsBefore);
    const GeometryRef circleCenter{circle_.entity, SubElement::Center};
    const GeometryRef arcCenter{arc_.entity, SubElement::Center};
    const GeometryRef arcStart{arc_.entity, SubElement::Start};
    const GeometryRef arcEnd{arc_.entity, SubElement::End};
    for (const auto& refs :
         {std::vector<GeometryRef>{point_}, std::vector<GeometryRef>{line_}, std::vector<GeometryRef>{start_}, std::vector<GeometryRef>{end_},
          std::vector<GeometryRef>{circleCenter}, std::vector<GeometryRef>{arcCenter}, std::vector<GeometryRef>{arcStart}, std::vector<GeometryRef>{arcEnd},
          std::vector<GeometryRef>{circle_, point_}, std::vector<GeometryRef>{point_, arc_}, std::vector<GeometryRef>{circle_, line_},
          std::vector<GeometryRef>{line_, arc_}, std::vector<GeometryRef>{circle_, circleCenter}, std::vector<GeometryRef>{arcEnd, circle_},
          std::vector<GeometryRef>{circle_, circle_}, std::vector<GeometryRef>{arc_, arc_}, std::vector<GeometryRef>{circle_, arc_, point_}}) {
        const auto prepared = ConstraintActions(*sketch_).prepare({ConstraintAction::Concentric, std::nullopt}, refs);
        EXPECT_EQ(prepared.state, PreparationState::InvalidSelection);
        EXPECT_EQ(prepared.rejection, ActionRejection::InvalidSelection);
        EXPECT_FALSE(prepared.definition);
    }
    EXPECT_EQ(prepare(ConstraintAction::Concentric, {circle_, arc_}, 1.0).rejection, ActionRejection::InvalidValue);
    EXPECT_EQ(prepare(ConstraintAction::Concentric, {circle_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Concentric, {circle_, arc_}).state, PreparationState::Ready);
    const auto missing = prepare(ConstraintAction::Concentric, {circle_, {EntityId(9999), SubElement::Whole}});
    EXPECT_EQ(missing.state, PreparationState::InvalidSelection);
    ASSERT_TRUE(missing.error);
    EXPECT_EQ(missing.error->code, ErrorCode::NotFound);

    const auto after = sketch_->snapshot();
    const auto pointsAfter = sketch_->pointElements();
    ASSERT_TRUE(after);
    ASSERT_TRUE(pointsAfter);
    EXPECT_EQ(after.value().lastEntityId, before.value().lastEntityId);
    EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
    EXPECT_EQ(after.value().entities.size(), before.value().entities.size());
    EXPECT_EQ(after.value().constraints.size(), before.value().constraints.size());
    EXPECT_DOUBLE_EQ(std::get<Circle2>(after.value().entities[3].geometry).radius, std::get<Circle2>(before.value().entities[3].geometry).radius);
    ASSERT_EQ(pointsAfter.value().size(), pointsBefore.value().size());
    for (size_t i = 0; i < pointsBefore.value().size(); ++i) {
        EXPECT_EQ(pointsAfter.value()[i].ref, pointsBefore.value()[i].ref);
        EXPECT_EQ(pointsAfter.value()[i].position, pointsBefore.value()[i].position);
    }
    const auto prepared = prepare(ConstraintAction::Concentric, {circle_, arc_});
    ASSERT_TRUE(prepared.definition);
    const auto added = sketch_->addConstraint(*prepared.definition);
    ASSERT_TRUE(added);
    EXPECT_EQ(added.value().get(), before.value().lastConstraintId + 1);
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

TEST_F(ConstraintActionsBehavior, MidpointRejectsInvalidSelectionsAndPreparationPreservesGeometry) {
    const auto before = sketch_->snapshot();
    const auto pointsBefore = sketch_->pointElements();
    ASSERT_TRUE(before);
    ASSERT_TRUE(pointsBefore);
    for (const auto& refs : {std::vector<GeometryRef>{circle_}, std::vector<GeometryRef>{arc_}, std::vector<GeometryRef>{point_, start_},
                             std::vector<GeometryRef>{line_, otherLine_}, std::vector<GeometryRef>{point_, circle_}, std::vector<GeometryRef>{arc_, line_},
                             std::vector<GeometryRef>{point_, point_}, std::vector<GeometryRef>{line_, line_}, std::vector<GeometryRef>{point_, line_, end_},
                             std::vector<GeometryRef>{{circle_.entity, SubElement::Start}}, std::vector<GeometryRef>{{line_.entity, SubElement::Center}}}) {
        const auto prepared = ConstraintActions(*sketch_).prepare({ConstraintAction::Midpoint, std::nullopt}, refs);
        EXPECT_EQ(prepared.state, PreparationState::InvalidSelection);
        EXPECT_EQ(prepared.rejection, ActionRejection::InvalidSelection);
        EXPECT_FALSE(prepared.definition);
    }
    EXPECT_EQ(prepare(ConstraintAction::Midpoint, {point_, line_}, 1.0).rejection, ActionRejection::InvalidValue);
    const auto missing = prepare(ConstraintAction::Midpoint, {point_, {EntityId(9999), SubElement::Whole}});
    EXPECT_EQ(missing.state, PreparationState::InvalidSelection);
    ASSERT_TRUE(missing.error);
    EXPECT_EQ(missing.error->code, ErrorCode::NotFound);
    EXPECT_EQ(prepare(ConstraintAction::Midpoint, {line_, point_}).state, PreparationState::Ready);

    const auto after = sketch_->snapshot();
    const auto pointsAfter = sketch_->pointElements();
    ASSERT_TRUE(after);
    ASSERT_TRUE(pointsAfter);
    EXPECT_EQ(after.value().lastEntityId, before.value().lastEntityId);
    EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
    EXPECT_EQ(after.value().entities.size(), before.value().entities.size());
    EXPECT_EQ(after.value().constraints.size(), before.value().constraints.size());
    ASSERT_EQ(pointsAfter.value().size(), pointsBefore.value().size());
    for (size_t i = 0; i < pointsBefore.value().size(); ++i) {
        EXPECT_EQ(pointsAfter.value()[i].ref, pointsBefore.value()[i].ref);
        EXPECT_EQ(pointsAfter.value()[i].position, pointsBefore.value()[i].position);
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

TEST_F(ConstraintActionsBehavior, HVDistanceChoosesDominantAxisAndOrdersPointsIndependentlyOfClicks) {
    const auto vertical = prepare(ConstraintAction::HVDistance, {end_, start_}, 7.0);
    ASSERT_EQ(vertical.state, PreparationState::Ready);
    ASSERT_TRUE(vertical.definition);
    EXPECT_EQ(vertical.definition->type, ConstraintType::VerticalDistance);
    EXPECT_EQ(vertical.definition->refs, (std::vector<GeometryRef>{start_, end_}));
    EXPECT_EQ(vertical.definition->value, 7.0);

    const auto horizontal = prepare(ConstraintAction::HVDistance, {point_, end_}, 7.0);
    ASSERT_EQ(horizontal.state, PreparationState::Ready);
    ASSERT_TRUE(horizontal.definition);
    EXPECT_EQ(horizontal.definition->type, ConstraintType::HorizontalDistance);
    EXPECT_EQ(horizontal.definition->refs, (std::vector<GeometryRef>{end_, point_}));

    const GeometryRef center{circle_.entity, SubElement::Center};
    const auto diagonal = prepare(ConstraintAction::HVDistance, {start_, center}, 0.0);
    ASSERT_EQ(diagonal.state, PreparationState::Ready);
    ASSERT_TRUE(diagonal.definition);
    EXPECT_EQ(diagonal.definition->type, ConstraintType::HorizontalDistance);
    EXPECT_EQ(prepare(ConstraintAction::HVDistance, {}, 7.0).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::HVDistance, {start_}, 7.0).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::HVDistance, {line_}, 7.0).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::HVDistance, {start_, start_}, 7.0).state, PreparationState::InvalidSelection);
}

TEST_F(ConstraintActionsBehavior, UnaryConstraintsRequireExactArityAndValidReferences) {
    EXPECT_EQ(prepare(ConstraintAction::Horizontal).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Horizontal, {line_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Vertical, {line_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Horizontal, {end_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Vertical, {line_, otherLine_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Fix, {line_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Fix, {start_, end_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Horizontal, {{EntityId(9999), SubElement::Whole}}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Coincident, {{line_.entity, SubElement::Center}}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Coincident, {{point_.entity, SubElement::Start}}).state, PreparationState::InvalidSelection);
}

TEST_F(ConstraintActionsBehavior, ValuesAreRequiredOnlyForDimensionsAndRespectTheirRanges) {
    EXPECT_EQ(prepare(ConstraintAction::Dimension).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::HVDistance, {start_, end_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Horizontal, {line_}, 5.0).state, PreparationState::InvalidSelection);

    for (double value : {-1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        EXPECT_EQ(prepare(ConstraintAction::Dimension, {line_}, value).state, PreparationState::InvalidSelection);
        EXPECT_EQ(prepare(ConstraintAction::HVDistance, {start_, end_}, value).state, PreparationState::InvalidSelection);
        EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, value).state, PreparationState::InvalidSelection);
    }
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {line_}, 0.0).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {start_, end_}, 0.0).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, 0.0).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, std::numbers::pi).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, std::numbers::pi + 0.1).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_}, std::numbers::pi / 2).state, PreparationState::NeedsMoreInput);
}

TEST_F(ConstraintActionsBehavior, FixCapturesCurrentPositionOfStandalonePointsAndCurveElements) {
    const auto fixedPoint = prepare(ConstraintAction::Fix, {point_});
    ASSERT_EQ(fixedPoint.state, PreparationState::Ready);
    ASSERT_TRUE(fixedPoint.definition);
    ASSERT_TRUE(fixedPoint.definition->fixedPosition);
    EXPECT_EQ(*fixedPoint.definition->fixedPosition, (Vec2{9, 8}));
    EXPECT_FALSE(fixedPoint.definition->value);

    const auto fixedEnd = prepare(ConstraintAction::Fix, {end_});
    ASSERT_EQ(fixedEnd.state, PreparationState::Ready);
    ASSERT_TRUE(fixedEnd.definition);
    ASSERT_TRUE(fixedEnd.definition->fixedPosition);
    EXPECT_EQ(*fixedEnd.definition->fixedPosition, (Vec2{3, 4}));
}

TEST_F(ConstraintActionsBehavior, EqualityAndTangencyRespectSolverCapabilities) {
    const auto capabilities = sketch_->capabilities();
    if (capabilities.supports(ConstraintType::Equal)) {
        EXPECT_EQ(prepare(ConstraintAction::Equal, {line_}).state, PreparationState::NeedsMoreInput);
        EXPECT_EQ(prepare(ConstraintAction::Equal, {circle_}).state, PreparationState::NeedsMoreInput);
        EXPECT_EQ(prepare(ConstraintAction::Equal, {line_, otherLine_}).state, PreparationState::Ready);
        EXPECT_EQ(prepare(ConstraintAction::Equal, {circle_, arc_}).state, PreparationState::Ready);
        EXPECT_EQ(prepare(ConstraintAction::Equal, {line_, circle_}).state, PreparationState::InvalidSelection);
    } else {
        EXPECT_EQ(prepare(ConstraintAction::Equal).state, PreparationState::Unsupported);
        EXPECT_EQ(prepare(ConstraintAction::Equal, {line_, otherLine_}).state, PreparationState::Unsupported);
    }
    if (!capabilities.supports(ConstraintType::Tangent)) {
        EXPECT_EQ(prepare(ConstraintAction::Tangent).state, PreparationState::Unsupported);
        EXPECT_EQ(prepare(ConstraintAction::Tangent, {line_, circle_}).state, PreparationState::Unsupported);
    }
}

TEST_F(ConstraintActionsBehavior, PreparationDoesNotMoveGeometryAddConstraintsOrConsumeIds) {
    const auto before = sketch_->snapshot();
    const auto pointsBefore = sketch_->pointElements();
    ASSERT_TRUE(before);
    ASSERT_TRUE(pointsBefore);

    const auto coincident = prepare(ConstraintAction::Coincident, {start_, point_});
    ASSERT_EQ(coincident.state, PreparationState::Ready);
    ASSERT_TRUE(coincident.definition);
    EXPECT_EQ(prepare(ConstraintAction::Fix, {end_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {line_}, 9.0).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Parallel, {line_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Horizontal, {point_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {start_, point_, otherLine_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {start_, point_, end_}).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {point_, otherLine_}).state, PreparationState::NeedsMoreInput);
    EXPECT_EQ(prepare(ConstraintAction::Symmetric, {line_, otherLine_}).state, PreparationState::InvalidSelection);
    if (!sketch_->capabilities().supports(ConstraintType::Tangent)) {
        EXPECT_EQ(prepare(ConstraintAction::Tangent).state, PreparationState::Unsupported);
    }

    const auto after = sketch_->snapshot();
    const auto pointsAfter = sketch_->pointElements();
    ASSERT_TRUE(after);
    ASSERT_TRUE(pointsAfter);
    EXPECT_EQ(after.value().lastEntityId, before.value().lastEntityId);
    EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
    EXPECT_EQ(after.value().entities.size(), before.value().entities.size());
    EXPECT_EQ(after.value().constraints.size(), before.value().constraints.size());
    EXPECT_DOUBLE_EQ(std::get<Circle2>(after.value().entities[3].geometry).radius, std::get<Circle2>(before.value().entities[3].geometry).radius);
    ASSERT_EQ(pointsAfter.value().size(), pointsBefore.value().size());
    for (size_t i = 0; i < pointsBefore.value().size(); ++i) {
        EXPECT_EQ(pointsAfter.value()[i].ref, pointsBefore.value()[i].ref);
        EXPECT_EQ(pointsAfter.value()[i].position, pointsBefore.value()[i].position);
    }

    const auto addedConstraint = sketch_->addConstraint(*coincident.definition);
    const auto addedPoint = sketch_->addPoint({20, 30});
    ASSERT_TRUE(addedConstraint);
    ASSERT_TRUE(addedPoint);
    EXPECT_EQ(addedConstraint.value().get(), before.value().lastConstraintId + 1);
    EXPECT_EQ(addedPoint.value().get(), before.value().lastEntityId + 1);
}

TEST_F(ConstraintActionsBehavior, ApplyingCoincidencePreservesMidpointPlacementAndSolves) {
    const auto a = sketch_->addPoint({0, 0});
    const auto b = sketch_->addPoint({6, 8});
    ASSERT_TRUE(a);
    ASSERT_TRUE(b);
    const GeometryRef first{a.value(), SubElement::Whole};
    const GeometryRef second{b.value(), SubElement::Whole};
    const auto prepared = prepare(ConstraintAction::Coincident, {first, second});
    ASSERT_EQ(prepared.state, PreparationState::Ready);
    ASSERT_TRUE(prepared.definition);

    ConstraintActions actions(*sketch_);
    const auto countBefore = sketch_->constraintCount();
    const auto report = actions.apply(*prepared.definition);
    EXPECT_EQ(report.action, ActionKind::ApplyConstraint);
    EXPECT_EQ(report.change, ModelChange::Changed);
    EXPECT_EQ(report.requestedConstraints, 1);
    ASSERT_EQ(report.constraintIds.size(), 1);
    EXPECT_TRUE(sketch_->constraint(report.constraintIds[0]));
    EXPECT_FALSE(report.operationError);
    EXPECT_FALSE(report.solveError);
    ASSERT_TRUE(report.solve);
    EXPECT_EQ(report.solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch_->constraintCount(), countBefore + 1);
    const auto firstPosition = sketch_->pointPosition(first);
    const auto secondPosition = sketch_->pointPosition(second);
    ASSERT_TRUE(firstPosition);
    ASSERT_TRUE(secondPosition);
    EXPECT_NEAR(firstPosition.value().x, 3, 1e-6);
    EXPECT_NEAR(firstPosition.value().y, 4, 1e-6);
    EXPECT_NEAR(secondPosition.value().x, 3, 1e-6);
    EXPECT_NEAR(secondPosition.value().y, 4, 1e-6);
}

TEST_F(ConstraintActionsBehavior, ApplyingPerpendicularAddsConstraintAndSolves) {
    const auto prepared = prepare(ConstraintAction::Perpendicular, {line_, otherLine_});
    ASSERT_EQ(prepared.state, PreparationState::Ready);
    ASSERT_TRUE(prepared.definition);

    ConstraintActions actions(*sketch_);
    const auto countBefore = sketch_->constraintCount();
    const auto report = actions.apply(*prepared.definition);
    EXPECT_EQ(report.change, ModelChange::Changed);
    ASSERT_EQ(report.constraintIds.size(), 1);
    EXPECT_TRUE(sketch_->constraint(report.constraintIds[0]));
    ASSERT_TRUE(report.solve);
    EXPECT_EQ(report.solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch_->constraintCount(), countBefore + 1);
    const auto first = sketch_->entity(line_.entity);
    const auto second = sketch_->entity(otherLine_.entity);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const auto& a = std::get<Line2>(first.value().geometry);
    const auto& b = std::get<Line2>(second.value().geometry);
    const Vec2 directionA{a.end.x - a.start.x, a.end.y - a.start.y};
    const Vec2 directionB{b.end.x - b.start.x, b.end.y - b.start.y};
    const double lengths = std::hypot(directionA.x, directionA.y) * std::hypot(directionB.x, directionB.y);
    ASSERT_GT(lengths, 0);
    EXPECT_NEAR(directionA.x * directionB.x + directionA.y * directionB.y, 0, 1e-6 * lengths);
}

TEST(ConstraintActionsTangency, ApplyingTangentSelectsAndStoresLineSideInEitherReferenceOrder) {
    for (const double y : {1.0, -1.0}) {
        for (const bool reversed : {false, true}) {
            Sketch sketch;
            const auto line = sketch.addLine({-3, 0}, {3, 0});
            const auto circle = sketch.addCircle({0, y}, 1);
            ASSERT_TRUE(line);
            ASSERT_TRUE(circle);
            std::vector<GeometryRef> refs{{line.value(), SubElement::Whole}, {circle.value(), SubElement::Whole}};
            if (reversed) {
                std::swap(refs[0], refs[1]);
            }
            ConstraintActions actions(sketch);
            const auto prepared = actions.prepare({ConstraintAction::Tangent, std::nullopt}, refs);
            ASSERT_EQ(prepared.state, PreparationState::Ready);
            ASSERT_TRUE(prepared.definition);
            EXPECT_FALSE(prepared.definition->tangencyBranch);

            const auto report = actions.apply(*prepared.definition);
            EXPECT_EQ(report.change, ModelChange::Changed);
            ASSERT_EQ(report.constraintIds.size(), 1);
            ASSERT_TRUE(report.solve);
            EXPECT_EQ(report.solve->status, SolveStatus::Converged);
            const auto inserted = sketch.constraint(report.constraintIds[0]);
            ASSERT_TRUE(inserted);
            EXPECT_EQ(inserted.value().definition.refs, refs);
            EXPECT_EQ(inserted.value().definition.tangencyBranch, y > 0 ? TangencyBranch::PositiveSide : TangencyBranch::NegativeSide);
        }
    }
}

TEST(ConstraintActionsTangency, ApplyingNestedCircleTangentStoresContainmentInEitherReferenceOrder) {
    for (const bool reversed : {false, true}) {
        Sketch sketch;
        const auto outer = sketch.addCircle({0, 0}, 3);
        const auto inner = sketch.addCircle({1, 0}, 2);
        ASSERT_TRUE(outer);
        ASSERT_TRUE(inner);
        std::vector<GeometryRef> refs{{outer.value(), SubElement::Whole}, {inner.value(), SubElement::Whole}};
        if (reversed) {
            std::swap(refs[0], refs[1]);
        }
        ConstraintActions actions(sketch);
        const auto prepared = actions.prepare({ConstraintAction::Tangent, std::nullopt}, refs);
        ASSERT_EQ(prepared.state, PreparationState::Ready);
        ASSERT_TRUE(prepared.definition);
        const auto report = actions.apply(*prepared.definition);
        EXPECT_EQ(report.change, ModelChange::Changed);
        ASSERT_EQ(report.constraintIds.size(), 1);
        ASSERT_TRUE(report.solve);
        EXPECT_EQ(report.solve->status, SolveStatus::Converged);
        const auto inserted = sketch.constraint(report.constraintIds[0]);
        ASSERT_TRUE(inserted);
        EXPECT_EQ(inserted.value().definition.tangencyBranch, reversed ? TangencyBranch::SecondContainsFirst : TangencyBranch::FirstContainsSecond);
    }
}

TEST_F(ConstraintActionsBehavior, ApplyingSymmetricConfirmsStoredDefinitionAndSolve) {
    const auto first = sketch_->addPoint({-2, 1});
    const auto second = sketch_->addPoint({2, 1});
    const auto center = sketch_->addPoint({0, 1});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(center);
    const std::vector<GeometryRef> refs{{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}, {center.value(), SubElement::Whole}};
    const auto prepared = ConstraintActions(*sketch_).prepare({ConstraintAction::Symmetric, std::nullopt}, refs);
    ASSERT_EQ(prepared.state, PreparationState::Ready);
    ASSERT_TRUE(prepared.definition);
    const auto before = sketch_->snapshot();
    ASSERT_TRUE(before);

    const auto report = ConstraintActions(*sketch_).apply(*prepared.definition);
    EXPECT_EQ(report.action, ActionKind::ApplyConstraint);
    EXPECT_EQ(report.change, ModelChange::Changed);
    EXPECT_EQ(report.requestedConstraints, 1);
    ASSERT_EQ(report.constraintIds.size(), 1);
    EXPECT_EQ(report.constraintIds[0].get(), before.value().lastConstraintId + 1);
    EXPECT_FALSE(report.operationError);
    EXPECT_FALSE(report.solveError);
    ASSERT_TRUE(report.solve);
    EXPECT_EQ(report.solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch_->constraintCount(), before.value().constraints.size() + 1);
    const auto inserted = sketch_->constraint(report.constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Symmetric);
    EXPECT_EQ(inserted.value().definition.refs, refs);
    EXPECT_FALSE(inserted.value().definition.value);
    EXPECT_FALSE(inserted.value().definition.fixedPosition);
}

TEST_F(ConstraintActionsBehavior, ApplyingMidpointStoresConstraintAndSolvesPointAtFixedSegmentCenter) {
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {start_}, std::nullopt, Vec2{0, 0}}));
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {end_}, std::nullopt, Vec2{3, 4}}));
    const auto prepared = prepare(ConstraintAction::Midpoint, {line_, point_});
    ASSERT_EQ(prepared.state, PreparationState::Ready);
    ASSERT_TRUE(prepared.definition);
    const auto before = sketch_->snapshot();
    ASSERT_TRUE(before);

    const auto report = ConstraintActions(*sketch_).apply(*prepared.definition);
    EXPECT_EQ(report.action, ActionKind::ApplyConstraint);
    EXPECT_EQ(report.change, ModelChange::Changed);
    EXPECT_EQ(report.requestedConstraints, 1);
    ASSERT_EQ(report.constraintIds.size(), 1);
    EXPECT_EQ(report.constraintIds[0].get(), before.value().lastConstraintId + 1);
    EXPECT_FALSE(report.operationError);
    EXPECT_FALSE(report.solveError);
    ASSERT_TRUE(report.solve);
    EXPECT_EQ(report.solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch_->constraintCount(), before.value().constraints.size() + 1);
    const auto inserted = sketch_->constraint(report.constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Midpoint);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{point_, line_}));
    EXPECT_FALSE(inserted.value().definition.value);
    EXPECT_FALSE(inserted.value().definition.fixedPosition);
    const auto position = sketch_->pointPosition(point_);
    ASSERT_TRUE(position);
    EXPECT_NEAR(position.value().x, 1.5, 1e-6);
    EXPECT_NEAR(position.value().y, 2, 1e-6);
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

TEST_F(ConstraintActionsBehavior, FailedConvergenceRetainsConfirmedConstraintInsertion) {
    const auto point = sketch_->addPoint({0, 0});
    ASSERT_TRUE(point);
    const GeometryRef ref{point.value(), SubElement::Whole};
    ConstraintActions actions(*sketch_);
    const auto first = actions.apply({ConstraintType::Fix, {ref}, std::nullopt, Vec2{0, 0}});
    ASSERT_EQ(first.constraintIds.size(), 1);
    const auto countBefore = sketch_->constraintCount();

    const auto report = actions.apply({ConstraintType::Fix, {ref}, std::nullopt, Vec2{1, 1}});
    EXPECT_EQ(report.change, ModelChange::Changed);
    EXPECT_FALSE(report.operationError);
    EXPECT_FALSE(report.solveError);
    ASSERT_TRUE(report.solve);
    EXPECT_EQ(report.solve->status, SolveStatus::Failed);
    ASSERT_EQ(report.constraintIds.size(), 1);
    const auto inserted = sketch_->constraint(report.constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.fixedPosition, (std::optional<Vec2>{{1, 1}}));
    EXPECT_EQ(sketch_->constraintCount(), countBefore + 1);
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
