#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <numbers>

#include "../editor/ConstraintActions.h"
#include "../editor/tools/CursorTool.h"
#include "../viewport/constraints/ConstraintLayout.h"
#include "../viewport/constraints/ConstraintMarkerStyle.h"
#include "TestTextMeasurer.h"

namespace {
using namespace core::sketch;
constexpr double kTolerance = 1e-8;

void expectPosition(glm::dvec2 actual, glm::dvec2 expected) {
    EXPECT_NEAR(actual.x, expected.x, kTolerance);
    EXPECT_NEAR(actual.y, expected.y, kTolerance);
}

class DimensionInteraction : public testing::Test {
protected:
    void SetUp() override {
        camera_.setViewport(800, 600);
        camera_.setCenter({0, 0});
        camera_.setZoom(100);
        const auto line = sketch().addLine({-2, 0}, {2, 0});
        ASSERT_TRUE(line);
        line_ = line.value();
        const auto constraint = sketch().addConstraint({ConstraintType::Length, {{line_, SubElement::Whole}}, 4});
        ASSERT_TRUE(constraint);
        dimension_ = constraint.value();
        rebuild();
    }

    Sketch& sketch() { return document_.sketch(); }
    void rebuild() { layout_.rebuild(sketch(), camera_, style_, presentation_, textMeasurer_); }

    const app::ConstraintMarker& marker(ConstraintId id) const {
        const auto found = std::find_if(layout_.markers().begin(), layout_.markers().end(),
                                        [id](const auto& item) { return item.constraints == std::vector<ConstraintId>{id}; });
        EXPECT_NE(found, layout_.markers().end());
        return *found;
    }

    std::optional<DimensionPickResult> pick(glm::dvec2 position) const { return picker_.pickDimensionAtScreenLogical(position.x, position.y, layout_); }

    void button(glm::dvec2 position, input::MouseButtonAction action, input::Modifiers modifiers = input::Modifiers::None) {
        rebuild();
        const auto buttons = action == input::MouseButtonAction::Press ? input::MouseButton::Left : input::MouseButton::None;
        EXPECT_FALSE(tool_.onMouseButton({position.x, position.y, input::MouseButton::Left, action, buttons, modifiers}));
    }

    void move(glm::dvec2 position, bool dragging = false, input::Modifiers modifiers = input::Modifiers::None) {
        rebuild();
        EXPECT_FALSE(tool_.onMouseMove({position.x, position.y, dragging ? input::MouseButton::Left : input::MouseButton::None, modifiers}));
    }

    glm::dvec2 placementScreen() const {
        const auto p = presentation_.labelPlacements.at(dimension_).position;
        return camera_.worldToScreenLogical({p.x, p.y});
    }

    void makeAngle() {
        ASSERT_TRUE(sketch().removeConstraint(dimension_));
        const auto first = sketch().addLine({0, 0}, {1, -1});
        const auto second = sketch().addLine({0, 0}, {1, 1});
        ASSERT_TRUE(first);
        ASSERT_TRUE(second);
        const auto angle =
            sketch().addConstraint({ConstraintType::Angle, {{first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}}, std::numbers::pi / 2.0});
        ASSERT_TRUE(angle);
        dimension_ = angle.value();
        presentation_.labelPlacements[dimension_] = {{2, 0}};
        rebuild();
    }

    Document document_;
    Camera2D camera_;
    OverlayModel overlay_;
    snap::SnapSystem snap_{document_.sketch(), camera_};
    Cpu2dPicker picker_{document_.sketch(), camera_};
    ConstraintActions actions_{document_.sketch()};
    app::ConstraintLayout layout_;
    app::ConstraintPresentationState presentation_;
    app::ConstraintMarkerStyle style_;
    TestTextMeasurer textMeasurer_;
    CursorTool tool_{document_, camera_, snap_, picker_, overlay_, actions_, layout_, presentation_};
    EntityId line_;
    ConstraintId dimension_;
};

TEST_F(DimensionInteraction, RotatedLabelUsesItsLocalRectangle) {
    ASSERT_TRUE(sketch().updateLine(line_, {{-2, -2}, {2, 2}}));
    auto definition = sketch().constraint(dimension_).value().definition;
    definition.value = 123456;
    ASSERT_TRUE(sketch().updateConstraint(dimension_, definition));
    presentation_.labelPlacements[dimension_] = {{2, 1}};
    rebuild();
    const auto label = marker(dimension_).labels.front();
    const glm::dvec2 u{std::cos(label.rotationRad), std::sin(label.rotationRad)};
    const glm::dvec2 n{-u.y, u.x};
    const auto inside = pick(label.center + u * (label.sizePx.x * 0.5 - 1) + n * (label.sizePx.y * 0.5 - 1));
    ASSERT_TRUE(inside);
    EXPECT_EQ(inside->constraint, dimension_);
    EXPECT_EQ(inside->part, DimensionPickResult::Part::Label);
    // This point is inside the axis-aligned bounding box, but outside the rotated label.
    EXPECT_FALSE(pick(label.center + glm::dvec2(18, 18)));
}

TEST_F(DimensionInteraction, SegmentPickingClampsToEndpointsAndUsesLogicalTolerance) {
    for (const double zoom : {25.0, 200.0}) {
        for (const double dpr : {1.0, 2.0}) {
            camera_.setZoom(zoom);
            camera_.setViewport(800, 600, dpr);
            rebuild();
            const auto& item = marker(dimension_);
            const auto segment = item.strokes.front();
            const auto middle = (segment.start + segment.end) * 0.5;
            const auto hit = pick(middle + glm::dvec2(0, item.hitRadiusPx - 0.1));
            ASSERT_TRUE(hit);
            EXPECT_EQ(hit->part, DimensionPickResult::Part::Body);
            EXPECT_FALSE(pick(middle + glm::dvec2(0, item.hitRadiusPx + 1.0)));
            EXPECT_FALSE(pick(segment.start - glm::dvec2(50, 0)));
        }
    }
}

TEST_F(DimensionInteraction, ArcPickingRespectsWrappedSweepAndEndpointTolerance) {
    ASSERT_NO_FATAL_FAILURE(makeAngle());
    const auto arc = marker(dimension_).arcs.front();
    const auto at = [&](double angle) { return arc.center + arc.radiusPx * glm::dvec2(std::cos(angle), -std::sin(angle)); };
    const auto middle = pick(at((arc.startAngle + arc.endAngle) * 0.5));
    ASSERT_TRUE(middle);
    EXPECT_EQ(middle->part, DimensionPickResult::Part::Body);
    const double tolerance = marker(dimension_).hitRadiusPx;
    EXPECT_TRUE(pick(at(arc.startAngle - tolerance * 0.5 / arc.radiusPx)));
    EXPECT_FALSE(pick(at(std::numbers::pi)));
    EXPECT_FALSE(pick(at(arc.startAngle - 0.2)));
}

TEST_F(DimensionInteraction, OverlapsPreferLabelsThenDistanceThenStableId) {
    const auto otherLine = sketch().addLine({-2, 1}, {2, 1});
    ASSERT_TRUE(otherLine);
    const auto other = sketch().addConstraint({ConstraintType::Length, {{otherLine.value(), SubElement::Whole}}, 4});
    ASSERT_TRUE(other);
    presentation_.labelPlacements[other.value()] = {{0.3, 0.3}};
    rebuild();
    const auto labelHit = pick({430, 270});
    ASSERT_TRUE(labelHit);
    EXPECT_EQ(labelHit->constraint, other.value());
    EXPECT_EQ(labelHit->part, DimensionPickResult::Part::Label);

    presentation_.labelPlacements[other.value()] = {{0, 0.25}};
    rebuild();
    const auto closer = pick({300, 274});
    ASSERT_TRUE(closer);
    EXPECT_EQ(closer->constraint, other.value());
    const auto tied = pick({300, 272.5});
    ASSERT_TRUE(tied);
    EXPECT_EQ(tied->constraint, dimension_);
}

TEST_F(DimensionInteraction, GeometricGlyphsAreNotMovableDimensions) {
    ASSERT_TRUE(sketch().removeConstraint(dimension_));
    const auto fixed = sketch().addConstraint({ConstraintType::Fix, {{line_, SubElement::Whole}}});
    ASSERT_TRUE(fixed);
    rebuild();
    const auto stroke = marker(fixed.value()).strokes.front();
    EXPECT_FALSE(pick((stroke.start + stroke.end) * 0.5));
}

TEST_F(DimensionInteraction, HoverMatchesClickPriorityAndClearsOnCancel) {
    move({400, 270});
    EXPECT_EQ(overlay_.hoveredDimension, dimension_);
    // A line endpoint takes priority over the nearby witness, while a label takes priority over a point.
    move({200, 300});
    EXPECT_FALSE(overlay_.hoveredDimension);
    presentation_.labelPlacements[dimension_] = {{-2, 0}};
    move({200, 300});
    EXPECT_EQ(overlay_.hoveredDimension, dimension_);
    EXPECT_FALSE(tool_.cancel().handled);
    EXPECT_FALSE(overlay_.hoveredDimension);
}

TEST_F(DimensionInteraction, ClickAndSmallMovementDoNotFreezeAutomaticPlacement) {
    button({403, 272}, input::MouseButtonAction::Press);
    EXPECT_EQ(overlay_.draggedDimension, dimension_);
    move({404, 273}, true);
    EXPECT_TRUE(presentation_.labelPlacements.empty());
    button({404, 273}, input::MouseButtonAction::Release);
    EXPECT_TRUE(presentation_.labelPlacements.empty());
    EXPECT_FALSE(overlay_.draggedDimension);
}

TEST_F(DimensionInteraction, DragKeepsGrabOffsetAndChangesOnlyPresentation) {
    overlay_.selection_.model.replace({line_, SubElement::Whole});
    button({403, 272}, input::MouseButtonAction::Press);
    EXPECT_TRUE(overlay_.selection_.model.empty());
    move({463, 232}, true);
    expectPosition(placementScreen(), {460, 230});
    // Release may carry a final position that has not arrived in a mouse-move event.
    button({483, 212}, input::MouseButtonAction::Release);
    expectPosition(placementScreen(), {480, 210});
    rebuild();
    expectPosition(marker(dimension_).labels.front().center, {480, 210});
    const auto geometry = std::get<Line2>(sketch().entity(line_).value().geometry);
    EXPECT_EQ(geometry.start, (Vec2{-2, 0}));
    EXPECT_EQ(geometry.end, (Vec2{2, 0}));
    const auto definition = sketch().constraint(dimension_).value().definition;
    EXPECT_EQ(definition.value, 4.0);
    EXPECT_EQ(definition.refs, (std::vector<GeometryRef>{{line_, SubElement::Whole}}));
    EXPECT_FALSE(overlay_.draggedDimension);
}

TEST_F(DimensionInteraction, BodyDragMovesTheLabelWithoutSnappingItsCenterToTheMouse) {
    const auto hit = pick({300, 270});
    ASSERT_TRUE(hit);
    ASSERT_EQ(hit->part, DimensionPickResult::Part::Body);
    button({300, 270}, input::MouseButtonAction::Press);
    move({320, 250}, true);
    button({320, 250}, input::MouseButtonAction::Release);
    expectPosition(placementScreen(), {420, 250});
}

TEST_F(DimensionInteraction, CancelRestoresAutomaticOrExplicitPlacement) {
    for (const bool explicitPlacement : {false, true}) {
        presentation_.labelPlacements.clear();
        if (explicitPlacement) {
            presentation_.labelPlacements[dimension_] = {{0, 0.3}};
        }
        button({400, 270}, input::MouseButtonAction::Press);
        move({500, 170}, true);
        ASSERT_TRUE(presentation_.labelPlacements.contains(dimension_));
        // SketchEditor routes Escape, tool changes and tab changes through cancel().
        const auto cancelled = tool_.cancel();
        EXPECT_TRUE(cancelled.handled);
        EXPECT_FALSE(cancelled.report);
        if (explicitPlacement) {
            EXPECT_EQ(presentation_.labelPlacements.at(dimension_).position, (Vec2{0, 0.3}));
        } else {
            EXPECT_TRUE(presentation_.labelPlacements.empty());
        }
        EXPECT_FALSE(overlay_.draggedDimension);
        EXPECT_FALSE(tool_.cancel().handled);
        // A later release must not commit a cancelled gesture.
        button({500, 170}, input::MouseButtonAction::Release);
        EXPECT_EQ(presentation_.labelPlacements.size(), explicitPlacement ? 1u : 0u);
    }
}

TEST_F(DimensionInteraction, MissingReleaseCancelsPresentationPreview) {
    button({400, 270}, input::MouseButtonAction::Press);
    move({500, 170}, true);
    ASSERT_TRUE(presentation_.labelPlacements.contains(dimension_));
    move({500, 170});
    EXPECT_TRUE(presentation_.labelPlacements.empty());
    EXPECT_FALSE(overlay_.draggedDimension);
}

TEST_F(DimensionInteraction, DeletedConstraintIsNotRestoredByCancellation) {
    presentation_.labelPlacements[dimension_] = {{0, 0.3}};
    button({400, 270}, input::MouseButtonAction::Press);
    move({500, 170}, true);
    ASSERT_TRUE(sketch().removeConstraint(dimension_));
    move({550, 150}, true);
    EXPECT_TRUE(presentation_.labelPlacements.empty());
    EXPECT_FALSE(overlay_.draggedDimension);
    EXPECT_FALSE(tool_.cancel().handled);
}

TEST_F(DimensionInteraction, CameraChangesUseCurrentTransformAndLogicalGrabOffset) {
    button({403, 272}, input::MouseButtonAction::Press);
    move({453, 252}, true);
    camera_.setZoom(200);
    camera_.setCenter({1, -1});
    camera_.setViewport(800, 600, 2.0);
    move({503, 202}, true);
    button({503, 202}, input::MouseButtonAction::Release);
    expectPosition(placementScreen(), {500, 200});
}

TEST_F(DimensionInteraction, PointPriorityAndShiftKeepGeometrySelectionReachable) {
    button({200, 300}, input::MouseButtonAction::Press);
    EXPECT_FALSE(overlay_.draggedDimension);
    EXPECT_TRUE(overlay_.selection_.model.contains({line_, SubElement::Start}));
    button({200, 300}, input::MouseButtonAction::Release);
    presentation_.labelPlacements[dimension_] = {{-2, 0}};
    button({200, 300}, input::MouseButtonAction::Press, input::Modifiers::Shift);
    EXPECT_FALSE(overlay_.draggedDimension);
    EXPECT_FALSE(overlay_.selection_.model.contains({line_, SubElement::Start}));
    button({200, 300}, input::MouseButtonAction::Release, input::Modifiers::Shift);
    button({200, 300}, input::MouseButtonAction::Press);
    EXPECT_EQ(overlay_.draggedDimension, dimension_);
    EXPECT_TRUE(overlay_.selection_.model.empty());
    tool_.cancel();
}

TEST_F(DimensionInteraction, ArcDragRebuildsRadiusThroughPresentationState) {
    ASSERT_NO_FATAL_FAILURE(makeAngle());
    const auto arc = marker(dimension_).arcs.front();
    const double angle = (arc.startAngle + arc.endAngle) * 0.5;
    const auto grab = arc.center + arc.radiusPx * glm::dvec2(std::cos(angle), -std::sin(angle));
    const auto hit = pick(grab);
    ASSERT_TRUE(hit);
    ASSERT_EQ(hit->part, DimensionPickResult::Part::Body);
    button(grab, input::MouseButtonAction::Press);
    move(grab + glm::dvec2(20, -20), true);
    button(grab + glm::dvec2(20, -20), input::MouseButtonAction::Release);
    rebuild();
    const auto& result = marker(dimension_);
    expectPosition(result.labels.front().center, {620, 280});
    ASSERT_FALSE(result.arcs.empty());
    EXPECT_NEAR(result.arcs.front().radiusPx, std::hypot(220.0, 20.0), kTolerance);
}
}  // namespace
