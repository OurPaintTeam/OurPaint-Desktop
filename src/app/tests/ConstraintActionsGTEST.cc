#include <gtest/gtest.h>

#include <cmath>
#include <initializer_list>
#include <limits>
#include <memory>
#include <numbers>
#include <optional>
#include <utility>

#include "../editor/ConstraintActions.h"
#include "sketch/Sketch.h"

namespace {
using namespace core::sketch;
using PreparationState = ConstraintPreparation::State;

class ConstraintActionsBehavior : public testing::TestWithParam<BackendKind> {
protected:
    void SetUp() override {
        auto created = Sketch::create(GetParam());
        ASSERT_TRUE(created);
        sketch_ = std::move(created.value());

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

TEST_P(ConstraintActionsBehavior, DimensionDistinguishesWholeLineFromItsEndpoints) {
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

TEST_P(ConstraintActionsBehavior, BinaryConstraintDistinguishesIncompleteAndInvalidSelections) {
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

TEST_P(ConstraintActionsBehavior, UnaryConstraintsRequireExactArityAndValidReferences) {
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

TEST_P(ConstraintActionsBehavior, ValuesAreRequiredOnlyForDimensionsAndRespectTheirRanges) {
    EXPECT_EQ(prepare(ConstraintAction::Dimension).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Horizontal, {line_}, 5.0).state, PreparationState::InvalidSelection);

    for (double value : {-1.0, std::numeric_limits<double>::infinity(), std::numeric_limits<double>::quiet_NaN()}) {
        EXPECT_EQ(prepare(ConstraintAction::Dimension, {line_}, value).state, PreparationState::InvalidSelection);
        EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, value).state, PreparationState::InvalidSelection);
    }
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {line_}, 0.0).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Dimension, {start_, end_}, 0.0).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, 0.0).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, std::numbers::pi).state, PreparationState::Ready);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_, otherLine_}, std::numbers::pi + 0.1).state, PreparationState::InvalidSelection);
    EXPECT_EQ(prepare(ConstraintAction::Angle, {line_}, std::numbers::pi / 2).state, PreparationState::NeedsMoreInput);
}

TEST_P(ConstraintActionsBehavior, FixCapturesCurrentPositionOfStandalonePointsAndCurveElements) {
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

TEST_P(ConstraintActionsBehavior, EqualityAndTangencyRespectBackendCapabilities) {
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

TEST_P(ConstraintActionsBehavior, PreparationDoesNotMoveGeometryAddConstraintsOrConsumeIds) {
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

TEST_P(ConstraintActionsBehavior, ApplyingCoincidencePreservesMidpointPlacementAndSolves) {
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
    ASSERT_TRUE(actions.apply(*prepared.definition));
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

TEST_P(ConstraintActionsBehavior, ApplyingPerpendicularAddsConstraintAndSolves) {
    const auto prepared = prepare(ConstraintAction::Perpendicular, {line_, otherLine_});
    ASSERT_EQ(prepared.state, PreparationState::Ready);
    ASSERT_TRUE(prepared.definition);

    ConstraintActions actions(*sketch_);
    const auto countBefore = sketch_->constraintCount();
    ASSERT_TRUE(actions.apply(*prepared.definition));
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

INSTANTIATE_TEST_SUITE_P(AvailableBackends, ConstraintActionsBehavior, testing::ValuesIn(Sketch::availableBackends()),
                         [](const testing::TestParamInfo<BackendKind>& info) { return info.param == BackendKind::Dcm ? "DCM" : "SolveSpace"; });
}  // namespace
