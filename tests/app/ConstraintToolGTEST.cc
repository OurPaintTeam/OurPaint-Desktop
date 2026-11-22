#include <gtest/gtest.h>

#include <vector>

#include "Camera2D.h"
#include "ConstraintTool.h"
#include "Cpu2dPicker.h"
#include "Document.h"
#include "OverlayModel.h"
#include "SnapSystem.h"

namespace {
using namespace core::sketch;

class ConstraintToolBehavior : public testing::Test {
protected:
    core::Document document_;
    Sketch& sketch_ = document_.sketch();
    render::Camera2D camera_;
    snap::SnapSystem snap_{sketch_, camera_};
    OverlayModel overlay_{};
    Cpu2dPicker picker_{sketch_, camera_};
    ConstraintTool tool_{document_, camera_, snap_, overlay_, picker_};
};

TEST_F(ConstraintToolBehavior, AppliesHorizontalToBothSelectedLinesInOneUndoStep) {
    const auto first = sketch_.addLine({0, 0}, {4, 0});
    const auto second = sketch_.addLine({0, 3}, {5, 3});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    sketch_.pushState();

    const GeometryRef firstRef{first.value()};
    const GeometryRef secondRef{second.value()};
    overlay_.selection_.model.replace(std::vector<GeometryRef>{firstRef, secondRef});
    tool_.setValue(8.0);
    tool_.setSubToolType(ToolId::Horizontal);

    const auto constraints = sketch_.constraints();
    ASSERT_TRUE(constraints);
    ASSERT_EQ(constraints.value().size(), 2U);
    EXPECT_EQ(constraints.value()[0].definition.type, ConstraintType::Horizontal);
    EXPECT_EQ(constraints.value()[1].definition.type, ConstraintType::Horizontal);
    EXPECT_EQ(constraints.value()[0].definition.refs, std::vector<GeometryRef>{firstRef});
    EXPECT_EQ(constraints.value()[1].definition.refs, std::vector<GeometryRef>{secondRef});

    sketch_.undo();
    EXPECT_EQ(sketch_.entityCount(), 2U);
    EXPECT_EQ(sketch_.constraintCount(), 0U);
    sketch_.redo();
    EXPECT_EQ(sketch_.constraintCount(), 2U);
}

TEST_F(ConstraintToolBehavior, RollsBackFixBatchWithAStaleSelectionWithoutRecordingUndo) {
    const auto point = sketch_.addPoint({2, 3});
    ASSERT_TRUE(point);
    sketch_.pushState();

    const GeometryRef pointRef{point.value()};
    overlay_.selection_.model.replace(std::vector<GeometryRef>{pointRef, {EntityId(9999)}});
    tool_.setSubToolType(ToolId::Fix);

    EXPECT_EQ(sketch_.constraintCount(), 0U);
    const auto position = sketch_.pointPosition(pointRef);
    ASSERT_TRUE(position);
    EXPECT_EQ(position.value(), (Vec2{2, 3}));

    sketch_.undo();
    EXPECT_EQ(sketch_.entityCount(), 0U);
}
}  // namespace
