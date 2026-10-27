#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "../editor/ConstraintActions.h"
#include "../editor/tools/CursorTool.h"
#include "../viewport/constraints/ConstraintLayout.h"

namespace {
using namespace core::sketch;

class CursorToolReports : public testing::Test {
protected:
    void SetUp() override {
        document_ = std::make_unique<Document>();
        camera_.setViewport(800, 600);
        picker_ = std::make_unique<Cpu2dPicker>(document_->sketch(), camera_);
        snap_ = std::make_unique<snap::SnapSystem>(document_->sketch(), camera_);
        actions_ = std::make_unique<ConstraintActions>(document_->sketch());
        tool_ = std::make_unique<CursorTool>(*document_, camera_, *snap_, *picker_, overlay_, *actions_, layout_, presentation_);
    }

    std::optional<ActionReport> key(input::KeyCode code, input::Modifiers modifiers = input::Modifiers::None) {
        return tool_->onKey({code, input::KeyAction::Press, modifiers});
    }

    std::optional<ActionReport> button(Vec2 world, input::MouseButtonAction action, input::Modifiers modifiers = input::Modifiers::None) {
        const auto screen = camera_.worldToScreenLogical({world.x, world.y});
        const auto buttons = action == input::MouseButtonAction::Press ? input::MouseButton::Left : input::MouseButton::None;
        return tool_->onMouseButton({screen.x, screen.y, input::MouseButton::Left, action, buttons, modifiers});
    }

    std::optional<ActionReport> move(Vec2 world, bool dragging = false) {
        const auto screen = camera_.worldToScreenLogical({world.x, world.y});
        return tool_->onMouseMove({screen.x, screen.y, dragging ? input::MouseButton::Left : input::MouseButton::None});
    }

    Sketch& sketch() { return document_->sketch(); }

    std::unique_ptr<Document> document_;
    Camera2D camera_;
    OverlayModel overlay_;
    std::unique_ptr<Cpu2dPicker> picker_;
    std::unique_ptr<ConstraintActions> actions_;
    std::unique_ptr<snap::SnapSystem> snap_;
    app::ConstraintLayout layout_;
    app::ConstraintPresentationState presentation_;
    std::unique_ptr<CursorTool> tool_;
};

TEST_F(CursorToolReports, EmptyDeletionAndPasteRemainSilent) {
    EXPECT_FALSE(key(input::KeyCode::Delete));
    EXPECT_FALSE(key(input::KeyCode::V, input::Modifiers::Ctrl));
    EXPECT_EQ(sketch().entityCount(), 0);
}

TEST_F(CursorToolReports, BatchDeletionReportsEachOwnerOnceAndCascadesConstraints) {
    const auto line = sketch().addLine({0, 0}, {3, 4});
    const auto point = sketch().addPoint({6, 8});
    ASSERT_TRUE(line);
    ASSERT_TRUE(point);
    ASSERT_TRUE(sketch().addConstraint(
        {ConstraintType::Coincident, {{line.value(), SubElement::Start}, {point.value(), SubElement::Whole}}, std::nullopt, std::nullopt}));
    overlay_.selection_.model.replace(
        std::vector<GeometryRef>{{line.value(), SubElement::Whole}, {line.value(), SubElement::Start}, {point.value(), SubElement::Whole}});

    const auto report = key(input::KeyCode::Delete);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->action, ActionKind::Delete);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_EQ(report->requestedEntities, 2);
    EXPECT_EQ(report->entityIds, (std::vector<EntityId>{line.value(), point.value()}));
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solve);
    EXPECT_TRUE(overlay_.selection_.model.empty());
    EXPECT_EQ(sketch().entityCount(), 0);
    EXPECT_EQ(sketch().constraintCount(), 0);
    EXPECT_FALSE(key(input::KeyCode::Delete));
}

TEST_F(CursorToolReports, RejectedDeletionPreservesTheOriginalErrorAndModel) {
    const auto point = sketch().addPoint({0, 0});
    ASSERT_TRUE(point);
    overlay_.selection_.model.replace(GeometryRef{EntityId(9999), SubElement::Whole});
    const auto report = key(input::KeyCode::Delete);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Unchanged);
    EXPECT_TRUE(report->entityIds.empty());
    ASSERT_TRUE(report->operationError);
    EXPECT_EQ(report->operationError->code, ErrorCode::NotFound);
    EXPECT_TRUE(sketch().entity(point.value()));
}

TEST_F(CursorToolReports, PasteReportsConfirmedEntityAndConstraintIdsInOneOutcome) {
    const auto first = sketch().addEntity(Point2{{0, 0}}, true);
    const auto second = sketch().addPoint({3, 4});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(sketch().addConstraint(
        {ConstraintType::Coincident, {{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}}, std::nullopt, std::nullopt}));
    overlay_.selection_.model.replace(std::vector<GeometryRef>{{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}});
    EXPECT_FALSE(key(input::KeyCode::C, input::Modifiers::Ctrl));
    EXPECT_FALSE(move({10, 10}));

    const auto report = key(input::KeyCode::V, input::Modifiers::Ctrl);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->action, ActionKind::Paste);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_EQ(report->requestedEntities, 2);
    EXPECT_EQ(report->requestedConstraints, 1);
    ASSERT_EQ(report->entityIds.size(), 2);
    ASSERT_EQ(report->constraintIds.size(), 1);
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solveError);
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    EXPECT_EQ(sketch().entityCount(), 4);
    EXPECT_EQ(sketch().constraintCount(), 2);
    const auto pastedFirst = sketch().entity(report->entityIds[0]);
    ASSERT_TRUE(pastedFirst);
    EXPECT_TRUE(pastedFirst.value().construction);
    const auto pastedConstraint = sketch().constraint(report->constraintIds[0]);
    ASSERT_TRUE(pastedConstraint);
    EXPECT_EQ(pastedConstraint.value().definition.refs[0].entity, report->entityIds[0]);
    EXPECT_EQ(pastedConstraint.value().definition.refs[1].entity, report->entityIds[1]);
}

TEST_F(CursorToolReports, PasteRetainsConfirmedInsertionsWhenTheSolverDoesNotConverge) {
    const auto point = sketch().addPoint({0, 0});
    ASSERT_TRUE(point);
    const GeometryRef ref{point.value(), SubElement::Whole};
    ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {ref}, std::nullopt, Vec2{0, 0}}));
    ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {ref}, std::nullopt, Vec2{1, 1}}));
    overlay_.selection_.model.replace(ref);
    EXPECT_FALSE(key(input::KeyCode::C, input::Modifiers::Ctrl));
    EXPECT_FALSE(move({10, 10}));

    const auto report = key(input::KeyCode::V, input::Modifiers::Ctrl);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->entityIds.size(), 1);
    ASSERT_EQ(report->constraintIds.size(), 2);
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solveError);
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Failed);
    EXPECT_TRUE(sketch().entity(report->entityIds[0]));
    EXPECT_TRUE(sketch().constraint(report->constraintIds[0]));
    EXPECT_TRUE(sketch().constraint(report->constraintIds[1]));
}

TEST_F(CursorToolReports, DragProducesOneSummaryOnReleaseAndNoReportsDuringMoves) {
    const auto point = sketch().addPoint({0, 0});
    ASSERT_TRUE(point);
    EXPECT_FALSE(button({0, 0}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({1, 1}, true));
    EXPECT_FALSE(move({2, 1}, true));
    const auto report = button({2, 1}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->action, ActionKind::Drag);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_EQ(report->entityIds, (std::vector<EntityId>{point.value()}));
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    const auto position = sketch().pointPosition({point.value(), SubElement::Whole});
    ASSERT_TRUE(position);
    EXPECT_NEAR(position.value().x, 2, 1e-6);
    EXPECT_NEAR(position.value().y, 1, 1e-6);
    EXPECT_FALSE(button({2, 1}, input::MouseButtonAction::Release));
}

TEST_F(CursorToolReports, NoDisplacementAndUnchangedFixedPointGesturesRemainSilent) {
    const auto point = sketch().addPoint({0, 0});
    ASSERT_TRUE(point);
    EXPECT_FALSE(button({0, 0}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({0, 0}, true));
    EXPECT_FALSE(button({0, 0}, input::MouseButtonAction::Release));

    const GeometryRef ref{point.value(), SubElement::Whole};
    ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {ref}, std::nullopt, Vec2{0, 0}}));
    const auto solved = sketch().solve();
    ASSERT_TRUE(solved);
    ASSERT_EQ(solved.value().status, SolveStatus::Converged);
    EXPECT_FALSE(button({0, 0}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({1, 1}, true));
    EXPECT_FALSE(button({1, 1}, input::MouseButtonAction::Release));
    const auto position = sketch().pointPosition(ref);
    ASSERT_TRUE(position);
    EXPECT_EQ(position.value(), (Vec2{0, 0}));
}

TEST_F(CursorToolReports, FailedCircleDragStopsFurtherMutationAndReportsOnceAtRelease) {
    const auto circle = sketch().addCircle({0, 0}, 2);
    ASSERT_TRUE(circle);
    EXPECT_FALSE(button({2, 0}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({0, 0}, true));
    EXPECT_FALSE(move({3, 0}, true));
    const auto report = button({3, 0}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Unchanged);
    ASSERT_TRUE(report->operationError);
    EXPECT_EQ(report->operationError->code, ErrorCode::InvalidArgument);
    EXPECT_TRUE(report->entityIds.empty());
    EXPECT_FALSE(report->solve);
    const auto entity = sketch().entity(circle.value());
    ASSERT_TRUE(entity);
    EXPECT_DOUBLE_EQ(std::get<Circle2>(entity.value().geometry).radius, 2);
    EXPECT_FALSE(button({3, 0}, input::MouseButtonAction::Release));
}

TEST_F(CursorToolReports, CancellingADragDeliversItsOutcomeAndRetainsGeometry) {
    const auto point = sketch().addPoint({0, 0});
    ASSERT_TRUE(point);
    EXPECT_FALSE(button({0, 0}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({1, 2}, true));
    const auto cancelled = tool_->cancel();
    EXPECT_TRUE(cancelled.handled);
    ASSERT_TRUE(cancelled.report);
    EXPECT_EQ(cancelled.report->change, ModelChange::Changed);
    EXPECT_EQ(cancelled.report->entityIds, (std::vector<EntityId>{point.value()}));
    const auto position = sketch().pointPosition({point.value(), SubElement::Whole});
    ASSERT_TRUE(position);
    EXPECT_NEAR(position.value().x, 1, 1e-6);
    EXPECT_NEAR(position.value().y, 2, 1e-6);
    const auto repeated = tool_->cancel();
    EXPECT_FALSE(repeated.handled);
    EXPECT_FALSE(repeated.report);
}

class CursorToolTangency : public CursorToolReports {
protected:
    void SetUp() override {
        CursorToolReports::SetUp();
        const auto line = sketch().addLine({-3, 0}, {3, 0});
        const auto circle = sketch().addCircle({0, 1}, 1);
        ASSERT_TRUE(line);
        ASSERT_TRUE(circle);
        line_ = {line.value(), SubElement::Whole};
        circle_ = {circle.value(), SubElement::Whole};
        center_ = {circle.value(), SubElement::Center};
        ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {line_}, std::nullopt, std::nullopt}));
        const auto tangent = sketch().addConstraint({ConstraintType::Tangent, {line_, circle_}, std::nullopt, std::nullopt});
        ASSERT_TRUE(tangent);
        tangent_ = tangent.value();
        const auto solved = sketch().solve();
        ASSERT_TRUE(solved);
        ASSERT_EQ(solved.value().status, SolveStatus::Converged);
    }

    void keepRadiusOne() {
        const auto reference = sketch().addCircle({10, 10}, 1);
        ASSERT_TRUE(reference);
        const GeometryRef ref{reference.value(), SubElement::Whole};
        ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {ref}, std::nullopt, std::nullopt}));
        ASSERT_TRUE(sketch().addConstraint({ConstraintType::Equal, {circle_, ref}, std::nullopt, std::nullopt}));
    }

    GeometryRef line_, circle_, center_;
    ConstraintId tangent_;
};

TEST_F(CursorToolTangency, DragCrossingRespectsPixelDeadzoneAndReportsChangedTangencyBranch) {
    camera_.setZoom(200);
    ASSERT_NO_FATAL_FAILURE(keepRadiusOne());
    EXPECT_FALSE(button({0, 1}, input::MouseButtonAction::Press));
    // Two logical pixels beyond the line stay inside the four-pixel switch deadzone.
    EXPECT_FALSE(move({0, -0.01}, true));
    const auto withinDeadzone = sketch().constraint(tangent_);
    ASSERT_TRUE(withinDeadzone);
    EXPECT_EQ(withinDeadzone.value().definition.tangencyBranch, TangencyBranch::PositiveSide);
    EXPECT_FALSE(move({0, -1}, true));
    const auto report = button({0, -1}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->action, ActionKind::Drag);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_EQ(report->entityIds, (std::vector<EntityId>{circle_.entity}));
    EXPECT_EQ(report->constraintIds, (std::vector<ConstraintId>{tangent_}));
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    const auto tangent = sketch().constraint(tangent_);
    ASSERT_TRUE(tangent);
    EXPECT_EQ(tangent.value().definition.tangencyBranch, TangencyBranch::NegativeSide);
    const auto center = sketch().pointPosition(center_);
    ASSERT_TRUE(center);
    EXPECT_NEAR(center.value().y, -1, 1e-6);
    EXPECT_FALSE(button({0, -1}, input::MouseButtonAction::Release));
}

TEST_F(CursorToolTangency, CircleRadiusDragKeepsSavedLineSide) {
    EXPECT_FALSE(button({0, 2}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({0, 3}, true));
    const auto report = button({0, 3}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_TRUE(report->constraintIds.empty());
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Converged);
    const auto tangent = sketch().constraint(tangent_);
    ASSERT_TRUE(tangent);
    EXPECT_EQ(tangent.value().definition.tangencyBranch, TangencyBranch::PositiveSide);
    const auto circle = sketch().entity(circle_.entity);
    ASSERT_TRUE(circle);
    const auto& geometry = std::get<Circle2>(circle.value().geometry);
    EXPECT_NEAR(geometry.radius, 2, 1e-6);
    EXPECT_NEAR(geometry.center.y, 2, 1e-6);
}

TEST_F(CursorToolTangency, CancellationRetainsAndReportsSwitchedSide) {
    ASSERT_NO_FATAL_FAILURE(keepRadiusOne());
    EXPECT_FALSE(button({0, 1}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({0, -1}, true));
    const auto cancellation = tool_->cancel();
    EXPECT_TRUE(cancellation.handled);
    ASSERT_TRUE(cancellation.report);
    EXPECT_EQ(cancellation.report->change, ModelChange::Changed);
    EXPECT_EQ(cancellation.report->constraintIds, (std::vector<ConstraintId>{tangent_}));
    const auto tangent = sketch().constraint(tangent_);
    ASSERT_TRUE(tangent);
    EXPECT_EQ(tangent.value().definition.tangencyBranch, TangencyBranch::NegativeSide);
    EXPECT_FALSE(tool_->cancel().report);
}

TEST_F(CursorToolTangency, BranchOnlyConstraintChangeDuringDragProducesSummary) {
    ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {circle_}, std::nullopt, std::nullopt}));
    EXPECT_FALSE(button({0, 1}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({0, 2}, true));
    const auto tangent = sketch().constraint(tangent_);
    ASSERT_TRUE(tangent);
    auto definition = tangent.value().definition;
    definition.tangencyBranch = TangencyBranch::NegativeSide;
    ASSERT_TRUE(sketch().updateConstraint(tangent_, definition));

    const auto report = button({0, 2}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_TRUE(report->entityIds.empty());
    EXPECT_EQ(report->constraintIds, (std::vector<ConstraintId>{tangent_}));
}

TEST_F(CursorToolReports, CursorCoincidenceReportsInsufficientTargetsAndConfirmedInsertion) {
    const auto missing = button({0, 0}, input::MouseButtonAction::Press, input::Modifiers::Alt);
    ASSERT_TRUE(missing);
    EXPECT_EQ(missing->change, ModelChange::Unchanged);
    EXPECT_EQ(missing->rejection, ActionRejection::InsufficientTargets);

    ASSERT_TRUE(sketch().addPoint({0, 0}));
    ASSERT_TRUE(sketch().addPoint({6, 0}));
    const auto added = button({3, 0}, input::MouseButtonAction::Press, input::Modifiers::Alt);
    ASSERT_TRUE(added);
    EXPECT_EQ(added->action, ActionKind::ApplyConstraint);
    EXPECT_EQ(added->change, ModelChange::Changed);
    ASSERT_EQ(added->constraintIds.size(), 1);
    EXPECT_TRUE(sketch().constraint(added->constraintIds[0]));
    ASSERT_TRUE(added->solve);
    EXPECT_EQ(added->solve->status, SolveStatus::Converged);
    const auto repeated = button({3, 0}, input::MouseButtonAction::Press, input::Modifiers::Alt);
    ASSERT_TRUE(repeated);
    EXPECT_EQ(repeated->rejection, ActionRejection::InsufficientTargets);
    EXPECT_EQ(sketch().constraintCount(), 1);
}

TEST_F(CursorToolReports, FailedCoincidenceSolveDoesNotPermitRetryToInsertTheSamePair) {
    const auto first = sketch().addPoint({0, 0});
    const auto second = sketch().addPoint({6, 0});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {{first.value(), SubElement::Whole}}, std::nullopt, Vec2{0, 0}}));
    ASSERT_TRUE(sketch().addConstraint({ConstraintType::Fix, {{second.value(), SubElement::Whole}}, std::nullopt, Vec2{6, 0}}));

    const auto report = button({3, 0}, input::MouseButtonAction::Press, input::Modifiers::Alt);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    ASSERT_EQ(report->constraintIds.size(), 1);
    ASSERT_TRUE(report->solve);
    EXPECT_EQ(report->solve->status, SolveStatus::Failed);
    EXPECT_TRUE(sketch().constraint(report->constraintIds[0]));
    const auto count = sketch().constraintCount();
    const auto repeated = button({3, 0}, input::MouseButtonAction::Press, input::Modifiers::Alt);
    ASSERT_TRUE(repeated);
    EXPECT_EQ(repeated->rejection, ActionRejection::InsufficientTargets);
    EXPECT_TRUE(repeated->constraintIds.empty());
    EXPECT_EQ(sketch().constraintCount(), count);
}

}  // namespace
