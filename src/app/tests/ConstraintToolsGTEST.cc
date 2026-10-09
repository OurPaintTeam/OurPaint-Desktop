#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
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
using ToolParameter = std::tuple<BackendKind, ConstraintToolKind>;

class ConstraintToolsBehavior : public testing::TestWithParam<ToolParameter> {
protected:
    void SetUp() override {
        auto created = Sketch::create(std::get<0>(GetParam()));
        ASSERT_TRUE(created);
        sketch_ = std::move(created.value());
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

    bool isDimension() const { return std::get<1>(GetParam()) == ConstraintToolKind::Dimension; }

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

std::vector<ToolParameter> toolParameters() {
    std::vector<ToolParameter> parameters;
    for (const auto backend : Sketch::availableBackends()) {
        parameters.emplace_back(backend, ConstraintToolKind::Relation);
        parameters.emplace_back(backend, ConstraintToolKind::Dimension);
    }
    return parameters;
}

INSTANTIATE_TEST_SUITE_P(AvailableBackendsAndTools, ConstraintToolsBehavior, testing::ValuesIn(toolParameters()),
                         [](const testing::TestParamInfo<ToolParameter>& info) {
                             const std::string backend = std::get<0>(info.param) == BackendKind::Dcm ? "DCM" : "SolveSpace";
                             return backend + (std::get<1>(info.param) == ConstraintToolKind::Relation ? "Relation" : "Dimension");
                         });
}  // namespace
