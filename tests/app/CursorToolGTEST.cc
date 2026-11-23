#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "CursorTool.h"
#include "RectSelectionMode.h"
#include "render/constraints/ConstraintLayout.h"

namespace {
using namespace core::sketch;
using app::RectSelectionMode;

bool contains(const std::vector<GeometryRef>& picked, GeometryRef ref) {
    return std::find(picked.begin(), picked.end(), ref) != picked.end();
}

class MarqueeSelection : public testing::Test {
protected:
    core::Document document_;
    Sketch& sketch_ = document_.sketch();
    render::Camera2D camera_;
    Cpu2dPicker picker_{sketch_, camera_};
};

TEST_F(MarqueeSelection, CrossingIncludesTangentCircleAndWindowRequiresItsFullExtent) {
    const auto circle = sketch_.addCircle({0, 0}, 5);
    ASSERT_TRUE(circle);
    const GeometryRef whole{circle.value()};
    const auto tangent = picker_.pickInRectAtWorld(5, -1, 6, 1, RectSelectionMode::Crossing);
    EXPECT_TRUE(contains(tangent, whole));
    const auto insideCircle = picker_.pickInRectAtWorld(-1, -1, 1, 1, RectSelectionMode::Crossing);
    EXPECT_FALSE(contains(insideCircle, whole));
    EXPECT_TRUE(contains(insideCircle, {circle.value(), SubElement::Center}));
    EXPECT_FALSE(contains(picker_.pickInRectAtWorld(-5, -5, 4, 5, RectSelectionMode::Window), whole));
    EXPECT_TRUE(contains(picker_.pickInRectAtWorld(5, 5, -5, -5, RectSelectionMode::Window), whole));
}

TEST_F(MarqueeSelection, WindowIncludesArcOnlyIfItsSweepFitsEvenWhenCenterIsOutside) {
    const auto arc = sketch_.addArc({0, 0}, {3, -4}, {3, 4});
    ASSERT_TRUE(arc);
    const GeometryRef whole{arc.value()};
    // Both endpoints have X = 3. The arc crosses the positive X axis at (5, 0).
    const auto clipped = picker_.pickInRectAtWorld(2.5, -4.5, 4.5, 4.5, RectSelectionMode::Window);
    EXPECT_FALSE(contains(clipped, whole));
    EXPECT_TRUE(contains(clipped, {arc.value(), SubElement::Start}));
    EXPECT_TRUE(contains(clipped, {arc.value(), SubElement::End}));
    EXPECT_TRUE(contains(picker_.pickInRectAtWorld(2.5, -4.5, 4.5, 4.5, RectSelectionMode::Crossing), whole));
    EXPECT_TRUE(contains(picker_.pickInRectAtWorld(2.5, -4.5, 5.5, 4.5, RectSelectionMode::Window), whole));
}

TEST_F(MarqueeSelection, ReversingDragChangesPreviewAndReleaseUsesFinalDirection) {
    camera_.setViewport(800, 600, 2.0);
    camera_.setZoom(100.0);
    camera_.setCenter({0, 0});
    const auto longLine = sketch_.addLine({-4, 0}, {0.8, 0});
    const auto shortLine = sketch_.addLine({0.4, 0.5}, {1, 0.5});
    const auto point = sketch_.addPoint({0, -2});
    ASSERT_TRUE(longLine);
    ASSERT_TRUE(shortLine);
    ASSERT_TRUE(point);

    app::ConstraintLayout layout;
    app::ConstraintPresentationState presentation;
    snap::SnapSystem snap{sketch_, camera_};
    OverlayModel overlay{};
    app::CursorTool tool{document_, camera_, snap, picker_, overlay, layout, presentation};
    auto& selection = overlay.selection_.model;
    const GeometryRef prior{point.value()};
    const GeometryRef longWhole{longLine.value()};
    const GeometryRef longEnd{longLine.value(), SubElement::End};
    const GeometryRef shortWhole{shortLine.value()};
    selection.replace(prior);

    using input::MouseButton;
    using input::MouseButtonAction;
    const auto shift = input::Modifiers::Shift;
    tool.onMouseButton({400, 200, MouseButton::Left, MouseButtonAction::Press, MouseButton::Left, shift});
    tool.onMouseMove({600, 400, MouseButton::Left, shift});
    ASSERT_TRUE(overlay.selectionRect_);
    EXPECT_EQ(overlay.selectionRect_->mode, RectSelectionMode::Window);
    EXPECT_TRUE(selection.contains(shortWhole));
    EXPECT_FALSE(selection.contains(longWhole));
    EXPECT_TRUE(selection.contains(longEnd));
    EXPECT_TRUE(selection.contains(prior));

    tool.onMouseMove({200, 400, MouseButton::Left, shift});
    ASSERT_TRUE(overlay.selectionRect_);
    EXPECT_EQ(overlay.selectionRect_->mode, RectSelectionMode::Crossing);
    EXPECT_FALSE(selection.contains(shortWhole));
    EXPECT_TRUE(selection.contains(longWhole));
    EXPECT_FALSE(selection.contains(longEnd));
    EXPECT_TRUE(selection.contains(prior));

    tool.onMouseButton({600, 400, MouseButton::Left, MouseButtonAction::Release, MouseButton::None, shift});
    EXPECT_FALSE(overlay.selectionRect_);
    EXPECT_TRUE(selection.contains(shortWhole));
    EXPECT_FALSE(selection.contains(longWhole));
    EXPECT_TRUE(selection.contains(longEnd));
    EXPECT_TRUE(selection.contains(prior));
}
}  // namespace
