#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "../../src/app/editor/tool/CursorTool.h"
#include "../../src/app/viewport/constraints/ConstraintLayout.h"
#include "../editor/ConstraintActions.h"

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

TEST_F(CursorToolReports, NurbsPickingSelectionAndSinglePointDragPreserveControlIndices) {
    const NurbsCurve2 curve{3, {{{0, 0}}, {{2, 3}}, {{4, -2}}, {{6, 0}}}, {1, 2, 1, 1}, {0, 0, 0, 0, 1, 1, 1, 1}};
    const auto added = sketch().addEntity(curve);
    ASSERT_TRUE(added);
    for (size_t i = 0; i < curve.controlPoints.size(); ++i) {
        const auto position = curve.controlPoints[i].position;
        const auto picked = picker_->pickAtWorld(position.x, position.y);
        ASSERT_TRUE(picked);
        EXPECT_EQ(picked->ref, (GeometryRef{added.value(), SubElement::Control, i}));
        overlay_.selection_.model.add(picked->ref);
    }
    EXPECT_EQ(overlay_.selection_.model.items().size(), 4U);
    const GeometryRef first{added.value(), SubElement::Control, 1};
    const GeometryRef second{added.value(), SubElement::Control, 2};
    const auto rectangle = picker_->pickInRectAtWorld(1, -3, 5, 4);
    EXPECT_EQ(rectangle, (std::vector<GeometryRef>{first, second}));
    overlay_.selection_.model.replace(rectangle);
    overlay_.selection_.model.toggle(first);
    EXPECT_FALSE(overlay_.selection_.model.contains(first));
    EXPECT_TRUE(overlay_.selection_.model.contains(second));
    overlay_.selection_.model.clear();

    EXPECT_FALSE(button({2, 3}, input::MouseButtonAction::Press));
    EXPECT_EQ(overlay_.selection_.model.items(), (std::vector<GeometryRef>{first}));
    EXPECT_FALSE(move({3, 5}, true));
    const auto report = button({3, 5}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_EQ(report->change, ModelChange::Changed);
    EXPECT_EQ(report->entityIds, (std::vector<EntityId>{added.value()}));
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solveError);
    auto expected = curve;
    expected.controlPoints[1].position = {3, 5};
    const auto current = sketch().entity(added.value());
    ASSERT_TRUE(current);
    const auto& actual = std::get<NurbsCurve2>(current.value().geometry);
    for (size_t i = 0; i < expected.controlPoints.size(); ++i) {
        EXPECT_NEAR(actual.controlPoints[i].position.x, expected.controlPoints[i].position.x, 1e-6);
        EXPECT_NEAR(actual.controlPoints[i].position.y, expected.controlPoints[i].position.y, 1e-6);
    }
    EXPECT_EQ(actual.weights, curve.weights);
    EXPECT_EQ(actual.knots, curve.knots);
}

TEST_F(CursorToolReports, NurbsMultiPointAndWholeSelectionsDragEveryTargetOnce) {
    const auto added = sketch().addEntity(NurbsCurve2{3, {{{0, 0}}, {{2, 3}}, {{4, -2}}, {{6, 0}}}, {1, 1, 1, 1}, {0, 0, 0, 0, 1, 1, 1, 1}});
    ASSERT_TRUE(added);
    const GeometryRef first{added.value(), SubElement::Control, 1};
    const GeometryRef second{added.value(), SubElement::Control, 2};
    overlay_.selection_.model.replace(std::vector<GeometryRef>{first, second});
    EXPECT_FALSE(button({2, 3}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({3, 5}, true));
    auto report = button({3, 5}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solveError);
    const auto firstPosition = sketch().pointPosition(first);
    const auto secondPosition = sketch().pointPosition(second);
    ASSERT_TRUE(firstPosition);
    ASSERT_TRUE(secondPosition);
    EXPECT_NEAR(firstPosition.value().x, 3, 1e-6);
    EXPECT_NEAR(firstPosition.value().y, 5, 1e-6);
    EXPECT_NEAR(secondPosition.value().x, 5, 1e-6);
    EXPECT_NEAR(secondPosition.value().y, 0, 1e-6);

    const auto before = sketch().pointElements(added.value());
    ASSERT_TRUE(before);
    overlay_.selection_.model.replace(std::vector<GeometryRef>{{added.value(), SubElement::Whole}, first});
    EXPECT_FALSE(button({3, 5}, input::MouseButtonAction::Press));
    EXPECT_FALSE(move({5, 6}, true));
    report = button({5, 6}, input::MouseButtonAction::Release);
    ASSERT_TRUE(report);
    EXPECT_FALSE(report->operationError);
    EXPECT_FALSE(report->solveError);
    const auto after = sketch().pointElements(added.value());
    ASSERT_TRUE(after);
    ASSERT_EQ(after.value().size(), before.value().size());
    for (size_t i = 0; i < before.value().size(); ++i) {
        EXPECT_EQ(after.value()[i].ref, before.value()[i].ref);
        EXPECT_NEAR(after.value()[i].position.x, before.value()[i].position.x + 2, 1e-6);
        EXPECT_NEAR(after.value()[i].position.y, before.value()[i].position.y + 1, 1e-6);
    }
}

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

}  // namespace
