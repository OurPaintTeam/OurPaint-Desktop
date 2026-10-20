#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <numbers>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "../editor/ConstraintActions.h"
#include "../editor/tools/ConstraintTool.h"
#include "../editor/tools/DimensionTool.h"
#include "../viewport/OverlayModel.h"
#include "../viewport/picking/Cpu2dPicker.h"
#include "Camera2D.h"
#include "sketch/Sketch.h"

namespace {
using namespace core::sketch;

enum class ConstraintToolKind { Relation, Dimension };

class ConstraintToolsBehavior : public testing::TestWithParam<ConstraintToolKind> {
protected:
    void SetUp() override {
        sketch_ = std::make_unique<Sketch>();
        const auto first = sketch_->addPoint({-2, 0});
        const auto second = sketch_->addPoint({2, 0});
        ASSERT_TRUE(first);
        ASSERT_TRUE(second);
        first_ = {first.value(), SubElement::Whole};
        second_ = {second.value(), SubElement::Whole};

        camera_.setViewport(1000, 1000, 2);
        camera_.setZoom(100);
        actions_ = std::make_unique<ConstraintActions>(*sketch_);
        picker_ = std::make_unique<Cpu2dPicker>(*sketch_, camera_);
        relationTool_ = std::make_unique<ConstraintTool>(*actions_, *picker_, overlay_);
        dimensionTool_ = std::make_unique<DimensionTool>(*actions_, *picker_, overlay_);
        overlay_.selection_.model.replace(first_);
    }

    bool isDimension() const { return GetParam() == ConstraintToolKind::Dimension; }

    IInteractionTool& tool() {
        if (isDimension()) {
            return *dimensionTool_;
        }
        return *relationTool_;
    }

    void begin(const ConstraintRequest& request) {
        if (isDimension()) {
            dimensionTool_->begin(request);
        } else {
            relationTool_->begin(request);
        }
    }

    void beginOrdinary(double distance = 4) {
        begin(isDimension() ? ConstraintRequest{ConstraintAction::Dimension, distance} : ConstraintRequest{ConstraintAction::Coincident, std::nullopt});
    }

    input::MouseButtonEvent pressAt(Vec2 world) const {
        const auto screen = camera_.worldToScreenLogical({world.x, world.y});
        return {screen.x, screen.y, input::MouseButton::Left, input::MouseButtonAction::Press, input::MouseButton::Left};
    }

    std::unique_ptr<Sketch> sketch_;
    Camera2D camera_;
    OverlayModel overlay_;
    std::unique_ptr<ConstraintActions> actions_;
    std::unique_ptr<Cpu2dPicker> picker_;
    std::unique_ptr<ConstraintTool> relationTool_;
    std::unique_ptr<DimensionTool> dimensionTool_;
    GeometryRef first_, second_;
};

TEST_P(ConstraintToolsBehavior, PartialInputAndRejectedPickRetainValidOperandWithoutMutation) {
    beginOrdinary();
    EXPECT_FALSE(tool().onMouseButton(pressAt({-2, 0})));
    EXPECT_EQ(overlay_.constraintRefs_, std::vector<GeometryRef>{first_});
    EXPECT_EQ(sketch_->constraintCount(), 0);

    const auto rejected = tool().onMouseButton(pressAt({-2, 0}));
    ASSERT_TRUE(rejected);
    EXPECT_EQ(rejected->change, ModelChange::Unchanged);
    EXPECT_EQ(rejected->rejection, ActionRejection::InvalidSelection);
    EXPECT_TRUE(rejected->constraintIds.empty());
    EXPECT_FALSE(rejected->solve);
    EXPECT_EQ(overlay_.constraintRefs_, std::vector<GeometryRef>{first_});
    EXPECT_EQ(sketch_->constraintCount(), 0);

    const auto added = tool().onMouseButton(pressAt({2, 0}));
    ASSERT_TRUE(added);
    EXPECT_EQ(added->change, ModelChange::Changed);
    ASSERT_EQ(added->constraintIds.size(), 1);
    EXPECT_TRUE(sketch_->constraint(added->constraintIds[0]));
    ASSERT_TRUE(added->solve);
    EXPECT_EQ(added->solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch_->constraintCount(), 1);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(overlay_.selection_.model.items(), std::vector<GeometryRef>{first_});
}

TEST_P(ConstraintToolsBehavior, FailedSolveRetainsInsertionAndClearsPendingOperands) {
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {first_}, std::nullopt, Vec2{-2, 0}}));
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {second_}, std::nullopt, Vec2{2, 0}}));
    // Both targets are fixed four units apart. Coincidence or distance one is
    // valid to insert but impossible to solve without violating those anchors.
    beginOrdinary(1);
    EXPECT_FALSE(tool().onMouseButton(pressAt({-2, 0})));
    ASSERT_EQ(overlay_.constraintRefs_, std::vector<GeometryRef>{first_});

    const auto report = tool().onMouseButton(pressAt({2, 0}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solveError);
    ASSERT_EQ(report->constraintIds.size(), 1);
    EXPECT_TRUE(sketch_->constraint(report->constraintIds[0]));
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Failed);
    EXPECT_EQ(sketch_->constraintCount(), 3);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(overlay_.selection_.model.items(), std::vector<GeometryRef>{first_});

    const auto cancellation = tool().cancel();
    EXPECT_FALSE(cancellation.handled);
    EXPECT_FALSE(cancellation.report);
    EXPECT_EQ(sketch_->constraintCount(), 3);
}

TEST_P(ConstraintToolsBehavior, UncertainInsertionFailureClearsOperandsWithoutClaimingInsertion) {
    const auto initial = sketch_->snapshot();
    ASSERT_TRUE(initial);
    auto state = initial.value();
    state.lastConstraintId = std::numeric_limits<int64_t>::max();
    ASSERT_TRUE(sketch_->replaceState(state));

    beginOrdinary();
    EXPECT_FALSE(tool().onMouseButton(pressAt({-2, 0})));
    const auto report = tool().onMouseButton(pressAt({2, 0}));
    ASSERT_TRUE(report);
    // An ordinary mutation's BackendFailure is conservatively uncertain at the
    // App boundary; it never confirms an ID or keeps an insertion pending.
    EXPECT_EQ(report->change, ModelChange::PotentiallyChanged);
    ASSERT_TRUE(report->operationError);
    EXPECT_EQ(report->operationError->code, ErrorCode::BackendFailure);
    EXPECT_TRUE(report->constraintIds.empty());
    EXPECT_FALSE(report->solve);
    EXPECT_EQ(sketch_->constraintCount(), 0);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_FALSE(tool().cancel().handled);
}

TEST_P(ConstraintToolsBehavior, UnsupportedRequestReportsRejectionWithoutMutation) {
    begin({ConstraintAction::Unsupported, std::nullopt});
    const auto report = tool().onMouseButton(pressAt({-2, 0}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Unchanged);
    EXPECT_EQ(report->rejection, ActionRejection::Unsupported);
    EXPECT_TRUE(report->constraintIds.empty());
    EXPECT_FALSE(report->solve);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(sketch_->constraintCount(), 0);
}

TEST_P(ConstraintToolsBehavior, CancellationDiscardsInputWithoutModelMutation) {
    beginOrdinary();
    EXPECT_FALSE(tool().onMouseButton(pressAt({-2, 0})));
    const auto before = sketch_->snapshot();
    ASSERT_TRUE(before);

    const auto cancellation = tool().cancel();
    EXPECT_TRUE(cancellation.handled);
    EXPECT_FALSE(cancellation.report);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(overlay_.selection_.model.items(), std::vector<GeometryRef>{first_});
    const auto after = sketch_->snapshot();
    ASSERT_TRUE(after);
    EXPECT_EQ(after.value().lastEntityId, before.value().lastEntityId);
    EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
    EXPECT_EQ(sketch_->constraintCount(), 0);
    const auto firstPosition = sketch_->pointPosition(first_);
    const auto secondPosition = sketch_->pointPosition(second_);
    ASSERT_TRUE(firstPosition);
    ASSERT_TRUE(secondPosition);
    EXPECT_EQ(firstPosition.value(), (Vec2{-2, 0}));
    EXPECT_EQ(secondPosition.value(), (Vec2{2, 0}));
}

TEST_P(ConstraintToolsBehavior, PreviewAndEmptyEventsStaySilent) {
    beginOrdinary();
    EXPECT_FALSE(tool().onMouseMove({200, 300}));
    EXPECT_FALSE(tool().onKey({input::KeyCode::Enter}));
    EXPECT_FALSE(tool().onMouseButton(pressAt({4, 4})));
    auto release = pressAt({-2, 0});
    release.action = input::MouseButtonAction::Release;
    EXPECT_FALSE(tool().onMouseButton(release));
    auto rightPress = pressAt({-2, 0});
    rightPress.button = input::MouseButton::Right;
    EXPECT_FALSE(tool().onMouseButton(rightPress));
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(sketch_->constraintCount(), 0);
}

TEST_P(ConstraintToolsBehavior, DimensionToolPicksPointsForHVDistanceAndWholeLinesForAngle) {
    const auto firstLine = sketch_->addLine({-2, 0}, {-1, 1});
    const auto secondLine = sketch_->addLine({2, 0}, {3, 0.5});
    ASSERT_TRUE(firstLine);
    ASSERT_TRUE(secondLine);

    dimensionTool_->begin({ConstraintAction::HVDistance, 4.0});
    // Whole-line hits must not become operands for a point coordinate dimension.
    EXPECT_FALSE(dimensionTool_->onMouseButton(pressAt({-1.5, 0.5})));
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_FALSE(dimensionTool_->onMouseButton(pressAt({-2, 0})));
    const auto distance = dimensionTool_->onMouseButton(pressAt({2, 0}));
    ASSERT_TRUE(distance);
    ASSERT_EQ(distance->constraintIds.size(), 1);
    const auto projected = sketch_->constraint(distance->constraintIds[0]);
    ASSERT_TRUE(projected);
    EXPECT_EQ(projected.value().definition.type, ConstraintType::HorizontalDistance);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());

    dimensionTool_->begin({ConstraintAction::Angle, std::numbers::pi / 2});
    // Endpoint clicks also hit standalone points, but Angle must select whole lines.
    EXPECT_FALSE(dimensionTool_->onMouseButton(pressAt({-2, 0})));
    const GeometryRef firstRef{firstLine.value(), SubElement::Whole};
    const GeometryRef secondRef{secondLine.value(), SubElement::Whole};
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{firstRef}));
    const auto angle = dimensionTool_->onMouseButton(pressAt({2, 0}));
    ASSERT_TRUE(angle);
    ASSERT_EQ(angle->constraintIds.size(), 1);
    const auto inserted = sketch_->constraint(angle->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Angle);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{firstRef, secondRef}));
    EXPECT_EQ(inserted.value().definition.value, std::numbers::pi / 2);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
}

class ConcentricConstraintToolBehavior : public testing::Test {
protected:
    void SetUp() override {
        const auto circle = sketch_.addCircle({-2, 0}, 1);
        const auto arc = sketch_.addArc({2, 0}, {3, 0}, {2, 1});
        const auto point = sketch_.addPoint({-3, 0});
        const auto line = sketch_.addLine({-1, -2}, {1, -2});
        ASSERT_TRUE(circle);
        ASSERT_TRUE(arc);
        ASSERT_TRUE(point);
        ASSERT_TRUE(line);
        circle_ = {circle.value(), SubElement::Whole};
        arc_ = {arc.value(), SubElement::Whole};
        point_ = {point.value(), SubElement::Whole};
        camera_.setViewport(1000, 1000, 2);
        camera_.setZoom(100);
        overlay_.selection_.model.replace(point_);
    }

    input::MouseButtonEvent pressAt(Vec2 world) const {
        const auto screen = camera_.worldToScreenLogical({world.x, world.y});
        return {screen.x, screen.y, input::MouseButton::Left, input::MouseButtonAction::Press, input::MouseButton::Left};
    }

    Sketch sketch_;
    Camera2D camera_;
    OverlayModel overlay_;
    ConstraintActions actions_{sketch_};
    Cpu2dPicker picker_{sketch_, camera_};
    ConstraintTool tool_{actions_, picker_, overlay_};
    GeometryRef circle_, arc_, point_;
};

TEST_F(ConcentricConstraintToolBehavior, OutlinePicksWholeCircleAndArcAndRetainsOperandsAfterRejection) {
    tool_.begin({ConstraintAction::Concentric, std::nullopt});
    EXPECT_FALSE(tool_.onMouseButton(pressAt({-2, 0})));
    EXPECT_FALSE(tool_.onMouseButton(pressAt({2, 0})));
    EXPECT_TRUE(overlay_.constraintRefs_.empty());

    // A standalone point overlaps the circle outline; Concentric needs the whole circle.
    EXPECT_FALSE(tool_.onMouseButton(pressAt({-3, 0})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{circle_}));
    EXPECT_EQ(sketch_.constraintCount(), 0);
    for (const Vec2 rejectedPosition : {Vec2{-3, 0}, Vec2{0, -2}}) {
        const auto rejected = tool_.onMouseButton(pressAt(rejectedPosition));
        ASSERT_TRUE(rejected);
        EXPECT_EQ(rejected->change, ModelChange::Unchanged);
        EXPECT_EQ(rejected->rejection, ActionRejection::InvalidSelection);
        EXPECT_TRUE(rejected->constraintIds.empty());
        EXPECT_FALSE(rejected->solve);
        EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{circle_}));
        EXPECT_EQ(sketch_.constraintCount(), 0);
    }

    // An arc endpoint is also a point hit; the curve picker must keep the whole arc.
    const auto report = tool_.onMouseButton(pressAt({3, 0}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solveError);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Concentric);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{circle_, arc_}));
    EXPECT_FALSE(inserted.value().definition.value);
    EXPECT_FALSE(inserted.value().definition.fixedPosition);
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    const auto circleCenter = sketch_.pointPosition({circle_.entity, SubElement::Center});
    const auto arcCenter = sketch_.pointPosition({arc_.entity, SubElement::Center});
    ASSERT_TRUE(circleCenter);
    ASSERT_TRUE(arcCenter);
    EXPECT_NEAR(circleCenter.value().x, arcCenter.value().x, 1e-6);
    EXPECT_NEAR(circleCenter.value().y, arcCenter.value().y, 1e-6);
    EXPECT_EQ(sketch_.constraintCount(), 1);
    EXPECT_EQ(sketch_.entityCount(), 4);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(overlay_.selection_.model.items(), (std::vector<GeometryRef>{point_}));
    EXPECT_FALSE(tool_.cancel().handled);
}

TEST_F(ConcentricConstraintToolBehavior, PreselectedArcCanBeCompletedByCircleOutline) {
    const std::vector<GeometryRef> initial{arc_};
    tool_.begin({ConstraintAction::Concentric, std::nullopt}, initial);
    EXPECT_EQ(overlay_.constraintRefs_, initial);
    const auto report = tool_.onMouseButton(pressAt({-3, 0}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Concentric);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{arc_, circle_}));
    EXPECT_EQ(sketch_.constraintCount(), 1);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
}

class MidpointConstraintToolBehavior : public testing::Test {
protected:
    void SetUp() override {
        const auto line = sketch_.addLine({-1, 1}, {1, 1});
        const auto point = sketch_.addPoint({-2, -1});
        const auto circle = sketch_.addCircle({2, -1}, 0.5);
        ASSERT_TRUE(line);
        ASSERT_TRUE(point);
        ASSERT_TRUE(circle);
        line_ = {line.value(), SubElement::Whole};
        point_ = {point.value(), SubElement::Whole};
        circleCenter_ = {circle.value(), SubElement::Center};
        camera_.setViewport(1000, 1000, 2);
        camera_.setZoom(100);
        overlay_.selection_.model.replace(point_);
    }

    input::MouseButtonEvent pressAt(Vec2 world) const {
        const auto screen = camera_.worldToScreenLogical({world.x, world.y});
        return {screen.x, screen.y, input::MouseButton::Left, input::MouseButtonAction::Press, input::MouseButton::Left};
    }

    Sketch sketch_;
    Camera2D camera_;
    OverlayModel overlay_;
    ConstraintActions actions_{sketch_};
    Cpu2dPicker picker_{sketch_, camera_};
    ConstraintTool tool_{actions_, picker_, overlay_};
    GeometryRef line_, point_, circleCenter_;
};

TEST_F(MidpointConstraintToolBehavior, PointThenLineEndpointPicksWholeLineAndPreservesRejectedInput) {
    tool_.begin({ConstraintAction::Midpoint, std::nullopt});
    EXPECT_FALSE(tool_.onMouseButton(pressAt({2.5, -1})));
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_FALSE(tool_.onMouseButton(pressAt({-2, -1})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{point_}));
    EXPECT_EQ(sketch_.constraintCount(), 0);

    const auto rejected = tool_.onMouseButton(pressAt({-2, -1}));
    ASSERT_TRUE(rejected);
    EXPECT_EQ(rejected->change, ModelChange::Unchanged);
    EXPECT_EQ(rejected->rejection, ActionRejection::InvalidSelection);
    EXPECT_TRUE(rejected->constraintIds.empty());
    EXPECT_FALSE(rejected->solve);
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{point_}));
    EXPECT_EQ(sketch_.constraintCount(), 0);

    const auto report = tool_.onMouseButton(pressAt({-1, 1}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Midpoint);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{point_, line_}));
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch_.constraintCount(), 1);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(overlay_.selection_.model.items(), (std::vector<GeometryRef>{point_}));
    EXPECT_FALSE(tool_.cancel().handled);
}

TEST_F(MidpointConstraintToolBehavior, LineFirstCanBeCompletedByCircleCenter) {
    tool_.begin({ConstraintAction::Midpoint, std::nullopt});
    EXPECT_FALSE(tool_.onMouseButton(pressAt({0.5, 1})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{line_}));
    const auto report = tool_.onMouseButton(pressAt({2, -1}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Midpoint);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{circleCenter_, line_}));
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
}

TEST_F(MidpointConstraintToolBehavior, PreselectedPointCompletesAtLineEndpointAndCancellationDiscardsInput) {
    const std::vector<GeometryRef> initial{point_};
    tool_.begin({ConstraintAction::Midpoint, std::nullopt}, initial);
    EXPECT_EQ(overlay_.constraintRefs_, initial);
    const auto cancellation = tool_.cancel();
    EXPECT_TRUE(cancellation.handled);
    EXPECT_FALSE(cancellation.report);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(sketch_.constraintCount(), 0);

    tool_.begin({ConstraintAction::Midpoint, std::nullopt}, initial);
    const auto report = tool_.onMouseButton(pressAt({1, 1}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{point_, line_}));
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
}

class SymmetricConstraintToolBehavior : public testing::Test {
protected:
    void SetUp() override {
        const auto first = sketch_.addPoint({-2, 1});
        const auto second = sketch_.addPoint({2, 1});
        const auto center = sketch_.addPoint({0, 1});
        const auto axis = sketch_.addLine({0, -2}, {0, 2});
        ASSERT_TRUE(first);
        ASSERT_TRUE(second);
        ASSERT_TRUE(center);
        ASSERT_TRUE(axis);
        first_ = {first.value(), SubElement::Whole};
        second_ = {second.value(), SubElement::Whole};
        center_ = {center.value(), SubElement::Whole};
        axis_ = {axis.value(), SubElement::Whole};
        camera_.setViewport(1000, 1000, 2);
        camera_.setZoom(100);
        overlay_.selection_.model.replace(first_);
    }

    input::MouseButtonEvent pressAt(Vec2 world) const {
        const auto screen = camera_.worldToScreenLogical({world.x, world.y});
        return {screen.x, screen.y, input::MouseButton::Left, input::MouseButtonAction::Press, input::MouseButton::Left};
    }

    Sketch sketch_;
    Camera2D camera_;
    OverlayModel overlay_;
    ConstraintActions actions_{sketch_};
    Cpu2dPicker picker_{sketch_, camera_};
    ConstraintTool tool_{actions_, picker_, overlay_};
    GeometryRef first_, second_, center_, axis_;
};

TEST_F(SymmetricConstraintToolBehavior, AxisAndTwoPointsWaitForCompleteInputAndPreserveRejectedOperands) {
    ASSERT_TRUE(sketch_.addCircle({-3, -2}, 0.5));
    tool_.begin({ConstraintAction::Symmetric, std::nullopt});
    EXPECT_FALSE(tool_.onMouseButton(pressAt({-2.5, -2})));
    EXPECT_TRUE(overlay_.constraintRefs_.empty());

    EXPECT_FALSE(tool_.onMouseButton(pressAt({0, -1})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{axis_}));
    EXPECT_FALSE(tool_.onMouseButton(pressAt({-2, 1})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{axis_, first_}));
    EXPECT_EQ(sketch_.constraintCount(), 0);

    const auto rejected = tool_.onMouseButton(pressAt({-2, 1}));
    ASSERT_TRUE(rejected);
    EXPECT_EQ(rejected->change, ModelChange::Unchanged);
    EXPECT_EQ(rejected->rejection, ActionRejection::InvalidSelection);
    EXPECT_TRUE(rejected->constraintIds.empty());
    EXPECT_FALSE(rejected->solve);
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{axis_, first_}));

    const auto report = tool_.onMouseButton(pressAt({2, 1}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Symmetric);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{first_, second_, axis_}));
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch_.constraintCount(), 1);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_EQ(overlay_.selection_.model.items(), (std::vector<GeometryRef>{first_}));
    EXPECT_FALSE(tool_.cancel().handled);
}

TEST_F(SymmetricConstraintToolBehavior, FinalPointIsCenterEvenWhenItAlsoHitsTheAxis) {
    tool_.begin({ConstraintAction::Symmetric, std::nullopt});
    EXPECT_FALSE(tool_.onMouseButton(pressAt({-2, 1})));
    EXPECT_FALSE(tool_.onMouseButton(pressAt({2, 1})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{first_, second_}));
    EXPECT_EQ(sketch_.constraintCount(), 0);

    const auto report = tool_.onMouseButton(pressAt({0, 1}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.type, ConstraintType::Symmetric);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{first_, second_, center_}));
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());

    EXPECT_FALSE(tool_.onMouseButton(pressAt({0, 1})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{center_}));
    EXPECT_EQ(sketch_.constraintCount(), 1);
}

TEST_F(SymmetricConstraintToolBehavior, InterleavedPreselectionCanBeCompletedByTheSecondPoint) {
    const std::vector<GeometryRef> initial{first_, axis_};
    tool_.begin({ConstraintAction::Symmetric, std::nullopt}, initial);
    EXPECT_EQ(overlay_.constraintRefs_, initial);
    const auto report = tool_.onMouseButton(pressAt({2, 1}));
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    const auto inserted = sketch_.constraint(report->constraintIds[0]);
    ASSERT_TRUE(inserted);
    EXPECT_EQ(inserted.value().definition.refs, (std::vector<GeometryRef>{first_, second_, axis_}));
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
}

TEST_F(SymmetricConstraintToolBehavior, CancellationAndRestartDiscardPartialInputWithoutMutation) {
    const auto before = sketch_.snapshot();
    const auto pointsBefore = sketch_.pointElements();
    ASSERT_TRUE(before);
    ASSERT_TRUE(pointsBefore);
    tool_.begin({ConstraintAction::Symmetric, std::nullopt});
    EXPECT_FALSE(tool_.onMouseButton(pressAt({-2, 1})));
    EXPECT_FALSE(tool_.onMouseButton(pressAt({2, 1})));
    const auto cancelled = tool_.cancel();
    EXPECT_TRUE(cancelled.handled);
    EXPECT_FALSE(cancelled.report);
    EXPECT_TRUE(overlay_.constraintRefs_.empty());

    const std::vector<GeometryRef> initial{first_, axis_};
    tool_.begin({ConstraintAction::Symmetric, std::nullopt}, initial);
    tool_.begin({ConstraintAction::Symmetric, std::nullopt});
    EXPECT_TRUE(overlay_.constraintRefs_.empty());
    EXPECT_FALSE(tool_.onMouseButton(pressAt({0, 1})));
    EXPECT_EQ(overlay_.constraintRefs_, (std::vector<GeometryRef>{center_}));
    const auto after = sketch_.snapshot();
    const auto pointsAfter = sketch_.pointElements();
    ASSERT_TRUE(after);
    ASSERT_TRUE(pointsAfter);
    EXPECT_EQ(after.value().lastEntityId, before.value().lastEntityId);
    EXPECT_EQ(after.value().lastConstraintId, before.value().lastConstraintId);
    EXPECT_EQ(sketch_.constraintCount(), 0);
    ASSERT_EQ(pointsAfter.value().size(), pointsBefore.value().size());
    for (size_t i = 0; i < pointsBefore.value().size(); ++i) {
        EXPECT_EQ(pointsAfter.value()[i].ref, pointsBefore.value()[i].ref);
        EXPECT_EQ(pointsAfter.value()[i].position, pointsBefore.value()[i].position);
    }
    EXPECT_EQ(overlay_.selection_.model.items(), (std::vector<GeometryRef>{first_}));
}

INSTANTIATE_TEST_SUITE_P(Tools, ConstraintToolsBehavior, testing::Values(ConstraintToolKind::Relation, ConstraintToolKind::Dimension),
                         [](const testing::TestParamInfo<ConstraintToolKind>& info) {
                             return std::string(info.param == ConstraintToolKind::Relation ? "Relation" : "Dimension");
                         });
}  // namespace
