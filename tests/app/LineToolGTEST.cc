#include <gtest/gtest.h>

#include <vector>

#include "DocumentView.h"

namespace {
using namespace core::sketch;

class PolylineDrawing : public testing::Test {
protected:
    void SetUp() override {
        view_.camera2D_.setViewport(800, 600);
        view_.camera2D_.setCenter({0, 0});
        view_.editorSession_.selectTool(ToolId::Polyline);
    }

    void click(double x, double y, input::Modifiers modifiers = input::Modifiers::None) {
        view_.viewportController_.onMouseButton({x, y, input::MouseButton::Left, input::MouseButtonAction::Press, input::MouseButton::Left, modifiers});
    }

    core::Document document_;
    Sketch& sketch_ = document_.sketch();
    app::DocumentView view_{document_, 0};
};

TEST_F(PolylineDrawing, RepeatedVertexKeepsChainAndShiftSegmentHasCoincidentStart) {
    click(100, 100);
    click(300, 100);
    click(300, 100);
    ASSERT_EQ(sketch_.entityCount(), 1U);
    click(310, 300, input::Modifiers::Shift);

    const auto lines = sketch_.lines();
    const auto joints = sketch_.constraints();
    ASSERT_TRUE(lines);
    ASSERT_TRUE(joints);
    ASSERT_EQ(lines.value().size(), 2U);
    ASSERT_EQ(joints.value().size(), 1U);
    const auto& first = lines.value()[0];
    const auto& second = lines.value()[1];
    const std::vector<GeometryRef> expected{{first.id, SubElement::End}, {second.id, SubElement::Start}};
    EXPECT_EQ(joints.value()[0].definition.type, ConstraintType::Coincident);
    EXPECT_EQ(joints.value()[0].definition.refs, expected);
    const auto& segment = std::get<Line2>(second.geometry);
    EXPECT_NEAR(segment.start.x, segment.end.x, 1e-8);
    ASSERT_EQ(view_.overlay_.lines_.size(), 1U);
    EXPECT_FLOAT_EQ(view_.overlay_.lines_[0].x1, static_cast<float>(segment.end.x));
    EXPECT_FLOAT_EQ(view_.overlay_.lines_[0].y1, static_cast<float>(segment.end.y));

    view_.viewportController_.onKey({input::KeyCode::Escape, input::KeyAction::Press});
    EXPECT_TRUE(view_.overlay_.lines_.empty());
    click(500, 400);
    click(600, 400);
    EXPECT_EQ(sketch_.entityCount(), 3U);
    EXPECT_EQ(sketch_.constraintCount(), 1U);
}

TEST_F(PolylineDrawing, UndoAndRedoEndPendingChainAndRestoreOneSegmentAtATime) {
    click(100, 100);
    click(300, 100);
    click(300, 300);

    view_.viewportController_.onKey({input::KeyCode::Z, input::KeyAction::Press, input::Modifiers::Ctrl});
    EXPECT_EQ(sketch_.entityCount(), 1U);
    EXPECT_EQ(sketch_.constraintCount(), 0U);
    EXPECT_TRUE(view_.overlay_.lines_.empty());
    click(500, 400);
    EXPECT_EQ(sketch_.entityCount(), 1U);

    view_.viewportController_.onKey({input::KeyCode::Z, input::KeyAction::Press, input::Modifiers::Ctrl | input::Modifiers::Shift});
    EXPECT_EQ(sketch_.entityCount(), 2U);
    EXPECT_EQ(sketch_.constraintCount(), 1U);
    EXPECT_TRUE(view_.overlay_.lines_.empty());

    click(500, 400);
    view_.editorSession_.selectTool(ToolId::Line);
    click(600, 400);
    EXPECT_EQ(sketch_.entityCount(), 2U);
    click(700, 400);
    EXPECT_EQ(sketch_.entityCount(), 3U);
    EXPECT_EQ(sketch_.constraintCount(), 1U);
    EXPECT_TRUE(view_.overlay_.lines_.empty());
}
}  // namespace
