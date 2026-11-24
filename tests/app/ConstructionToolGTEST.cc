#include <gtest/gtest.h>

#include <algorithm>

#include "DocumentView.h"

namespace {
using namespace core::sketch;

class ConstructionEditing : public testing::Test {
protected:
    void click(glm::dvec2 world) {
        const auto screen = view_.camera2D_.worldToScreenLogical(world);
        view_.viewportController_.onMouseButton({screen.x, screen.y, input::MouseButton::Left, input::MouseButtonAction::Press, input::MouseButton::Left});
        view_.viewportController_.onMouseButton({screen.x, screen.y, input::MouseButton::Left, input::MouseButtonAction::Release});
    }

    core::Document document_;
    Sketch& sketch_ = document_.sketch();
    const app::ViewportStyle style_ = app::ViewportStyle::makeDefault();
    app::DocumentView view_{document_, 0, style_};
};

TEST_F(ConstructionEditing, MixedSelectionConvertsWholeCurvesTogetherAndPreservesSelection) {
    const auto line = sketch_.addLine({0, 0}, {2, 0});
    const auto arc = sketch_.addArc({0, 0}, {2, 0}, {0, 2});
    const auto circle = sketch_.addEntity(Circle2{{0, 0}, 3}, true);
    const auto point = sketch_.addPoint({4, 1});
    ASSERT_TRUE(line && arc && circle && point);
    sketch_.pushState();

    auto& editor = view_.editorSession_;
    auto& selection = view_.overlay_.selection_.model;
    selection.add(GeometryRef{point.value()});
    selection.add(GeometryRef{line.value(), SubElement::End});
    EXPECT_FALSE(editor.canToggleConstruction());
    selection.add(GeometryRef{line.value()});
    selection.add(GeometryRef{arc.value()});
    selection.add(GeometryRef{circle.value()});
    const auto selected = selection.items();
    EXPECT_TRUE(editor.canToggleConstruction());
    editor.selectTool(ToolId::Construction);
    EXPECT_EQ(editor.activeToolId(), ToolId::Cursor);
    EXPECT_EQ(selection.items(), selected);
    EXPECT_TRUE(sketch_.entity(line.value()).value().construction);
    EXPECT_TRUE(sketch_.entity(arc.value()).value().construction);
    EXPECT_FALSE(sketch_.entity(point.value()).value().construction);

    sketch_.undo();
    EXPECT_FALSE(sketch_.entity(line.value()).value().construction);
    EXPECT_FALSE(sketch_.entity(arc.value()).value().construction);
    EXPECT_TRUE(sketch_.entity(circle.value()).value().construction);
    sketch_.redo();
    editor.selectTool(ToolId::Construction);
    EXPECT_FALSE(sketch_.entity(circle.value()).value().construction);
    EXPECT_FALSE(sketch_.entity(line.value()).value().construction);
    EXPECT_FALSE(sketch_.entity(arc.value()).value().construction);
}

TEST_F(ConstructionEditing, ClickModeIgnoresPointsAndEndsOnEscape) {
    view_.camera2D_.setViewport(800, 600);
    view_.camera2D_.setCenter({0, 0});
    const auto line = sketch_.addLine({0, 0}, {2, 0});
    const auto point = sketch_.addPoint({1, 0});
    const auto circle = sketch_.addCircle({0, 1}, 0.5);
    ASSERT_TRUE(line && point && circle);
    sketch_.pushState();

    auto& editor = view_.editorSession_;
    EXPECT_TRUE(editor.canToggleConstruction());
    editor.selectTool(ToolId::Construction);
    ASSERT_EQ(editor.activeToolId(), ToolId::Construction);
    click({0.5, 0});
    EXPECT_TRUE(sketch_.entity(line.value()).value().construction);
    sketch_.undo();
    click({0, 0});  // Endpoint on the line.
    click({1, 0});  // Standalone point on the line.
    click({0, 1});  // Center of the circle.
    EXPECT_FALSE(sketch_.entity(line.value()).value().construction);
    EXPECT_FALSE(sketch_.entity(circle.value()).value().construction);
    EXPECT_FALSE(sketch_.entity(point.value()).value().construction);
    sketch_.redo();  // Ignored clicks must leave the history intact.
    EXPECT_TRUE(sketch_.entity(line.value()).value().construction);
    click({0.5, 1});
    EXPECT_TRUE(sketch_.entity(circle.value()).value().construction);
    view_.viewportController_.onKey({input::KeyCode::Escape, input::KeyAction::Press});
    EXPECT_EQ(editor.activeToolId(), ToolId::Cursor);
}

TEST_F(ConstructionEditing, RationalSplineCanBePickedAndDrawnAsConstruction) {
    view_.camera2D_.setViewport(800, 600);
    const NurbsCurve2 curve{2, {{{0, 0}}, {{2, 3}}, {{4, 0}}}, {1, 2, 1}, {0, 0, 0, 1, 1, 1}};
    const auto id = sketch_.addEntity(curve);
    ASSERT_TRUE(id);
    sketch_.pushState();

    view_.editorSession_.selectTool(ToolId::Construction);
    click({2, 3});  // Control point.
    EXPECT_FALSE(sketch_.entity(id.value()).value().construction);
    click({2, 2});  // Point on the rational curve at u = 0.5.
    EXPECT_TRUE(sketch_.entity(id.value()).value().construction);

    Cpu2dPicker picker(sketch_, view_.camera2D_);
    const auto crossing = picker.pickInRectAtWorld(1.9, 1.9, 2.1, 2.1, app::RectSelectionMode::Crossing);
    EXPECT_EQ(crossing, (std::vector<GeometryRef>{{id.value()}}));
    EXPECT_TRUE(picker.pickInRectAtWorld(1.9, 1.9, 2.1, 2.1, app::RectSelectionMode::Window).empty());

    render::RenderScene scene;
    AxisTexts axis(view_.camera2D_);
    RenderSceneBuilder builder(document_, view_.overlay_, axis, style_, scene, view_.camera2D_, view_.constraintLayout_);
    builder.rebuild();
    const auto layer = std::ranges::find(scene.layers, "geometry", &render::DrawLayer::name);
    ASSERT_NE(layer, scene.layers.end());
    ASSERT_EQ(layer->lineBatches.size(), 1U);
    EXPECT_FALSE(layer->lineBatches.front().lines.empty());
    const auto color = layer->lineBatches.front().style.color;
    EXPECT_FLOAT_EQ(color.r, 63.f / 255);
    EXPECT_FLOAT_EQ(color.g, 72.f / 255);
    EXPECT_FLOAT_EQ(color.b, 204.f / 255);
}
}  // namespace
