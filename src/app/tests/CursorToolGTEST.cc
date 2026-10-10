#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <vector>

#include "../editor/ConstraintActions.h"
#include "../editor/tools/CursorTool.h"

namespace {
using namespace core::sketch;

class CursorToolReports : public testing::TestWithParam<BackendKind> {
protected:
    void SetUp() override {
        const auto backends = Sketch::availableBackends();
        if (std::find(backends.begin(), backends.end(), BackendKind::SolveSpace) == backends.end()) {
            GTEST_SKIP() << "Document currently constructs the SolveSpace backend";
        }
        document_ = std::make_unique<Document>();
        ASSERT_TRUE(document_->sketch().switchBackend(GetParam()));
        camera_.setViewport(800, 600);
        picker_ = std::make_unique<Cpu2dPicker>(document_->sketch(), camera_);
        actions_ = std::make_unique<ConstraintActions>(document_->sketch());
        tool_ = std::make_unique<CursorTool>(*document_, camera_, *picker_, overlay_, *actions_);
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
    std::unique_ptr<CursorTool> tool_;
};

TEST_P(CursorToolReports, EmptyDeletionAndPasteRemainSilent) {
    EXPECT_FALSE(key(input::KeyCode::Delete));
    EXPECT_FALSE(key(input::KeyCode::V, input::Modifiers::Ctrl));
    EXPECT_EQ(sketch().entityCount(), 0);
}

TEST_P(CursorToolReports, BatchDeletionReportsEachOwnerOnceAndCascadesConstraints) {
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

TEST_P(CursorToolReports, RejectedDeletionPreservesTheOriginalErrorAndModel) {
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

TEST_P(CursorToolReports, PasteReportsConfirmedEntityAndConstraintIdsInOneOutcome) {
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

TEST_P(CursorToolReports, PasteRetainsConfirmedInsertionsWhenTheSolverDoesNotConverge) {
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

TEST_P(CursorToolReports, PasteRetainsCreatedObjectsWhenACopiedConstraintBecomesUnsupported) {
    if (GetParam() != BackendKind::SolveSpace) {
        GTEST_SKIP() << "A supported source backend is required to copy Equal";
    }
    const auto backends = Sketch::availableBackends();
    if (std::find(backends.begin(), backends.end(), BackendKind::Dcm) == backends.end()) {
        GTEST_SKIP() << "The unsupported destination backend is not compiled";
    }
    const auto first = sketch().addLine({0, 0}, {3, 4});
    const auto second = sketch().addLine({10, 0}, {13, 4});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const auto equal =
        sketch().addConstraint({ConstraintType::Equal, {{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}}, std::nullopt, std::nullopt});
    ASSERT_TRUE(equal);
    overlay_.selection_.model.replace(std::vector<GeometryRef>{{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}});
    EXPECT_FALSE(key(input::KeyCode::C, input::Modifiers::Ctrl));
    ASSERT_TRUE(sketch().removeConstraint(equal.value()));
    ASSERT_TRUE(sketch().switchBackend(BackendKind::Dcm));

    const auto report = key(input::KeyCode::V, input::Modifiers::Ctrl);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_EQ(report->requestedEntities, 2);
    EXPECT_EQ(report->requestedConstraints, 1);
    EXPECT_EQ(report->entityIds.size(), 2);
    EXPECT_TRUE(report->constraintIds.empty());
    ASSERT_TRUE(report->operationError);
    EXPECT_EQ(report->operationError->code, ErrorCode::Unsupported);
    EXPECT_FALSE(report->solve);
    EXPECT_EQ(sketch().entityCount(), 4);
}

TEST_P(CursorToolReports, DragProducesOneSummaryOnReleaseAndNoReportsDuringMoves) {
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

TEST_P(CursorToolReports, NoDisplacementAndUnchangedFixedPointGesturesRemainSilent) {
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

TEST_P(CursorToolReports, FailedCircleDragStopsFurtherMutationAndReportsOnceAtRelease) {
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

TEST_P(CursorToolReports, CancellingADragDeliversItsOutcomeAndRetainsGeometry) {
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

TEST_P(CursorToolReports, CursorCoincidenceReportsInsufficientTargetsAndConfirmedInsertion) {
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

TEST_P(CursorToolReports, FailedCoincidenceSolveDoesNotPermitRetryToInsertTheSamePair) {
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

INSTANTIATE_TEST_SUITE_P(AvailableBackends, CursorToolReports, testing::ValuesIn(Sketch::availableBackends()),
                         [](const testing::TestParamInfo<BackendKind>& info) { return info.param == BackendKind::Dcm ? "DCM" : "SolveSpace"; });
}  // namespace
