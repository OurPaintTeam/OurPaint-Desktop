#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include "../viewport/constraints/ConstraintLayout.h"
#include "../viewport/constraints/ConstraintMarkerStyle.h"
#include "../viewport/constraints/ConstraintPresentationState.h"
#include "Camera2D.h"
#include "TestTextMeasurer.h"
#include "sketch/Sketch.h"

namespace {
using namespace core::sketch;

constexpr double kTolerance = 1e-8;

void expectPosition(glm::dvec2 actual, glm::dvec2 expected) {
    EXPECT_NEAR(actual.x, expected.x, kTolerance);
    EXPECT_NEAR(actual.y, expected.y, kTolerance);
}

class ConstraintLayoutBehavior : public testing::Test {
protected:
    void SetUp() override {
        sketch_ = std::make_unique<Sketch>();
        camera_.setViewport(800, 600);
        camera_.setCenter({0, 0});
        camera_.setZoom(100);
    }

    void rebuild() { layout_.rebuild(*sketch_, camera_, style_, presentation_, textMeasurer_); }

    std::vector<const app::ConstraintMarker*> attachedTo(EntityId entity) const {
        std::vector<const app::ConstraintMarker*> result;
        for (const auto& marker : layout_.markers()) {
            if (marker.attachment.entity == entity) {
                result.push_back(&marker);
            }
        }
        return result;
    }

    std::unique_ptr<Sketch> sketch_;
    Camera2D camera_;
    app::ConstraintMarkerStyle style_;
    app::ConstraintPresentationState presentation_;
    app::ConstraintLayout layout_;
    TestTextMeasurer textMeasurer_;
};

TEST_F(ConstraintLayoutBehavior, CoincidentGroupsOnDifferentNurbsControlsStaySeparate) {
    const auto curve = sketch_->addEntity(NurbsCurve2{3, {{{0, 0}}, {{2, 3}}, {{4, -2}}, {{6, 0}}}, {1, 1, 1, 1}, {0, 0, 0, 0, 1, 1, 1, 1}});
    const auto firstPoint = sketch_->addPoint({2, 3});
    const auto secondPoint = sketch_->addPoint({4, -2});
    ASSERT_TRUE(curve);
    ASSERT_TRUE(firstPoint);
    ASSERT_TRUE(secondPoint);
    const GeometryRef first{curve.value(), SubElement::Control, 1};
    const GeometryRef second{curve.value(), SubElement::Control, 2};
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {first, {firstPoint.value(), SubElement::Whole}}}));
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Coincident, {second, {secondPoint.value(), SubElement::Whole}}}));
    rebuild();
    const auto markers = attachedTo(curve.value());
    ASSERT_EQ(markers.size(), 2U);
    EXPECT_EQ(markers[0]->attachment, first);
    EXPECT_EQ(markers[1]->attachment, second);
    ASSERT_EQ(markers[0]->dots.size(), 1U);
    ASSERT_EQ(markers[1]->dots.size(), 1U);
    expectPosition(markers[0]->dots.front().center, camera_.worldToScreenLogical({2, 3}));
    expectPosition(markers[1]->dots.front().center, camera_.worldToScreenLogical({4, -2}));
}

TEST_F(ConstraintLayoutBehavior, PreservesLineGlyphsAndCanonicalPlacementOrder) {
    // Draw the first line backwards to exercise direction-independent placement.
    const auto line = sketch_->addLine({2, 0}, {-2, 0});
    const auto parallelLine = sketch_->addLine({-2, 2}, {2, 2});
    const auto verticalLine = sketch_->addLine({3, 2}, {3, -2});
    ASSERT_TRUE(line);
    ASSERT_TRUE(parallelLine);
    ASSERT_TRUE(verticalLine);
    const GeometryRef lineRef{line.value(), SubElement::Whole};
    const GeometryRef parallelRef{parallelLine.value(), SubElement::Whole};
    const GeometryRef verticalRef{verticalLine.value(), SubElement::Whole};
    const auto parallel = sketch_->addConstraint({ConstraintType::Parallel, {lineRef, parallelRef}});
    const auto perpendicular = sketch_->addConstraint({ConstraintType::Perpendicular, {lineRef, verticalRef}});
    const auto horizontal = sketch_->addConstraint({ConstraintType::Horizontal, {lineRef}});
    const auto vertical = sketch_->addConstraint({ConstraintType::Vertical, {verticalRef}});
    ASSERT_TRUE(parallel);
    ASSERT_TRUE(perpendicular);
    ASSERT_TRUE(horizontal);
    ASSERT_TRUE(vertical);

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 6u);
    const auto markers = attachedTo(line.value());
    ASSERT_EQ(markers.size(), 3u);
    EXPECT_EQ(markers[0]->type, ConstraintType::Horizontal);
    EXPECT_EQ(markers[1]->type, ConstraintType::Parallel);
    EXPECT_EQ(markers[2]->type, ConstraintType::Perpendicular);
    EXPECT_EQ(markers[0]->constraints, (std::vector<ConstraintId>{horizontal.value()}));
    EXPECT_EQ(markers[1]->constraints, (std::vector<ConstraintId>{parallel.value()}));
    EXPECT_EQ(markers[2]->constraints, (std::vector<ConstraintId>{perpendicular.value()}));
    ASSERT_EQ(markers[0]->strokes.size(), 4u);
    ASSERT_EQ(markers[1]->strokes.size(), 2u);
    ASSERT_EQ(markers[2]->strokes.size(), 2u);
    expectPosition(markers[0]->strokes.front().start, {290, 305});
    expectPosition(markers[0]->strokes.front().end, {310, 305});
    expectPosition(markers[1]->strokes[0].start, {390, 305});
    expectPosition(markers[1]->strokes[0].end, {410, 305});
    expectPosition(markers[1]->strokes[1].start, {390, 295});
    expectPosition(markers[1]->strokes[1].end, {410, 295});
    expectPosition(markers[2]->strokes[0].start, {490, 295});
    expectPosition(markers[2]->strokes[0].end, {510, 295});
    expectPosition(markers[2]->strokes[1].start, {500, 295});
    expectPosition(markers[2]->strokes[1].end, {500, 285});
    for (const auto& marker : layout_.markers()) {
        EXPECT_TRUE(marker.dots.empty());
        EXPECT_DOUBLE_EQ(marker.hitRadiusPx, style_.hitTolerancePx + style_.stroke.widthPx * 0.5);
    }
    const auto verticalMarkers = attachedTo(verticalLine.value());
    ASSERT_EQ(verticalMarkers.size(), 2u);
    EXPECT_EQ(verticalMarkers.front()->type, ConstraintType::Vertical);
    ASSERT_EQ(verticalMarkers.front()->strokes.size(), 4u);
    const auto& verticalStroke = verticalMarkers.front()->strokes.front();
    EXPECT_NEAR(verticalStroke.start.x, verticalStroke.end.x, kTolerance);
    EXPECT_NEAR(verticalStroke.end.y - verticalStroke.start.y, style_.lengthPx, kTolerance);
}

TEST_F(ConstraintLayoutBehavior, SharedParallelGlyphKeepsAllConstraintIds) {
    const auto first = sketch_->addLine({-2, 0}, {2, 0});
    const auto second = sketch_->addLine({-2, 1}, {2, 1});
    const auto third = sketch_->addLine({-2, 2}, {2, 2});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(third);
    const GeometryRef a{first.value(), SubElement::Whole};
    const auto ab = sketch_->addConstraint({ConstraintType::Parallel, {a, {second.value(), SubElement::Whole}}});
    const auto ac = sketch_->addConstraint({ConstraintType::Parallel, {{third.value(), SubElement::Whole}, a}});
    ASSERT_TRUE(ab);
    ASSERT_TRUE(ac);

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 3u);
    const auto markers = attachedTo(first.value());
    ASSERT_EQ(markers.size(), 1u);
    EXPECT_EQ(markers.front()->constraints, (std::vector<ConstraintId>{ab.value(), ac.value()}));
    EXPECT_EQ(markers.front()->strokes.size(), 2u);
}

TEST_F(ConstraintLayoutBehavior, TransitiveCoincidentGroupHasOneDotAndAllSortedIds) {
    const auto first = sketch_->addPoint({1, 2});
    const auto second = sketch_->addPoint({1, 2});
    const auto third = sketch_->addPoint({1, 2});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(third);
    const GeometryRef a{first.value(), SubElement::Whole};
    const GeometryRef b{second.value(), SubElement::Whole};
    const GeometryRef c{third.value(), SubElement::Whole};
    const auto bc = sketch_->addCoincident(b, c, CoincidentPlacement::Preserve);
    const auto ab = sketch_->addCoincident(a, b, CoincidentPlacement::Preserve);
    ASSERT_TRUE(bc);
    ASSERT_TRUE(ab);

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 1u);
    const auto& marker = layout_.markers().front();
    EXPECT_EQ(marker.type, ConstraintType::Coincident);
    EXPECT_EQ(marker.constraints, (std::vector<ConstraintId>{bc.value(), ab.value()}));
    EXPECT_TRUE(marker.strokes.empty());
    ASSERT_EQ(marker.dots.size(), 1u);
    expectPosition(marker.dots.front().center, camera_.worldToScreenLogical({1, 2}));
}

TEST_F(ConstraintLayoutBehavior, CoincidentGroupsAtTheSamePositionStayIndependent) {
    std::vector<GeometryRef> refs;
    for (int i = 0; i < 5; ++i) {
        const auto point = sketch_->addPoint({1, 2});
        ASSERT_TRUE(point);
        refs.push_back({point.value(), SubElement::Whole});
    }
    const auto first = sketch_->addCoincident(refs[0], refs[1], CoincidentPlacement::Preserve);
    const auto second = sketch_->addCoincident(refs[2], refs[3], CoincidentPlacement::Preserve);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 2u);
    std::vector<ConstraintId> ids;
    for (const auto& marker : layout_.markers()) {
        ASSERT_EQ(marker.constraints.size(), 1u);
        ids.push_back(marker.constraints.front());
        ASSERT_EQ(marker.dots.size(), 1u);
        expectPosition(marker.dots.front().center, camera_.worldToScreenLogical({1, 2}));
    }
    std::sort(ids.begin(), ids.end());
    EXPECT_EQ(ids, (std::vector<ConstraintId>{first.value(), second.value()}));
}

TEST_F(ConstraintLayoutBehavior, PointsOnTheSameCurveRemainSeparateCoincidentMarkers) {
    const auto line = sketch_->addLine({-2, 0}, {2, 0});
    const auto first = sketch_->addPoint({-1, 0});
    const auto second = sketch_->addPoint({1, 0});
    ASSERT_TRUE(line);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const GeometryRef lineRef{line.value(), SubElement::Whole};
    const GeometryRef firstRef{first.value(), SubElement::Whole};
    const GeometryRef secondRef{second.value(), SubElement::Whole};
    const ConstraintDefinition firstDefinition{ConstraintType::Coincident, {firstRef, lineRef}};
    const ConstraintDefinition secondDefinition{ConstraintType::Coincident, {lineRef, secondRef}};
    ASSERT_TRUE(sketch_->supportsConstraint(firstDefinition));
    ASSERT_TRUE(sketch_->supportsConstraint(secondDefinition));
    const auto firstConstraint = sketch_->addConstraint(firstDefinition);
    const auto secondConstraint = sketch_->addConstraint(secondDefinition);
    ASSERT_TRUE(firstConstraint);
    ASSERT_TRUE(secondConstraint);

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 2u);
    const auto firstMarkers = attachedTo(first.value());
    const auto secondMarkers = attachedTo(second.value());
    ASSERT_EQ(firstMarkers.size(), 1u);
    ASSERT_EQ(secondMarkers.size(), 1u);
    EXPECT_EQ(firstMarkers.front()->attachment, firstRef);
    EXPECT_EQ(secondMarkers.front()->attachment, secondRef);
    EXPECT_EQ(firstMarkers.front()->constraints, (std::vector<ConstraintId>{firstConstraint.value()}));
    EXPECT_EQ(secondMarkers.front()->constraints, (std::vector<ConstraintId>{secondConstraint.value()}));
    ASSERT_EQ(firstMarkers.front()->dots.size(), 1u);
    ASSERT_EQ(secondMarkers.front()->dots.size(), 1u);
    expectPosition(firstMarkers.front()->dots.front().center, camera_.worldToScreenLogical({-1, 0}));
    expectPosition(secondMarkers.front()->dots.front().center, camera_.worldToScreenLogical({1, 0}));
    EXPECT_TRUE(firstMarkers.front()->strokes.empty());
    EXPECT_TRUE(secondMarkers.front()->strokes.empty());
}

TEST_F(ConstraintLayoutBehavior, CoincidentResolvesCurveEndpointsAndCenters) {
    const auto point = sketch_->addPoint({1, 2});
    const auto lineStart = sketch_->addLine({1, 2}, {3, 4});
    const auto lineEnd = sketch_->addLine({-1, -1}, {1, 2});
    const auto circle = sketch_->addCircle({1, 2}, 2);
    const auto arcStart = sketch_->addArc({0, 2}, {1, 2}, {0, 3});
    const auto arcEnd = sketch_->addArc({1, 1}, {2, 1}, {1, 2});
    const auto arcCenter = sketch_->addArc({1, 2}, {2, 2}, {1, 3});
    ASSERT_TRUE(point);
    ASSERT_TRUE(lineStart);
    ASSERT_TRUE(lineEnd);
    ASSERT_TRUE(circle);
    ASSERT_TRUE(arcStart);
    ASSERT_TRUE(arcEnd);
    ASSERT_TRUE(arcCenter);
    const GeometryRef pointRef{point.value(), SubElement::Whole};
    const std::vector<GeometryRef> refs{{lineStart.value(), SubElement::Start}, {lineEnd.value(), SubElement::End}, {circle.value(), SubElement::Center},
                                        {arcStart.value(), SubElement::Start},  {arcEnd.value(), SubElement::End},  {arcCenter.value(), SubElement::Center}};
    std::vector<ConstraintId> ids;
    for (const auto ref : refs) {
        const auto coincident = sketch_->addCoincident(pointRef, ref, CoincidentPlacement::Preserve);
        ASSERT_TRUE(coincident);
        ids.push_back(coincident.value());
    }

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 1u);
    const auto& marker = layout_.markers().front();
    EXPECT_EQ(marker.constraints, ids);
    ASSERT_EQ(marker.dots.size(), 1u);
    expectPosition(marker.dots.front().center, camera_.worldToScreenLogical({1, 2}));
}

TEST_F(ConstraintLayoutBehavior, WholeLineFixRetainsLineSlotPlacement) {
    const auto line = sketch_->addLine({-2, 1}, {2, 1});
    ASSERT_TRUE(line);
    const GeometryRef lineRef{line.value(), SubElement::Whole};
    const ConstraintDefinition definition{ConstraintType::Fix, {lineRef}};
    ASSERT_TRUE(sketch_->supportsConstraint(definition));
    const auto fix = sketch_->addConstraint(definition);
    ASSERT_TRUE(fix);

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 1u);
    const auto& centeredMarker = layout_.markers().front();
    EXPECT_EQ(centeredMarker.type, ConstraintType::Fix);
    EXPECT_EQ(centeredMarker.attachment, lineRef);
    EXPECT_EQ(centeredMarker.constraints, (std::vector<ConstraintId>{fix.value()}));
    ASSERT_EQ(centeredMarker.strokes.size(), 1u);
    EXPECT_TRUE(centeredMarker.dots.empty());
    for (const auto& stroke : centeredMarker.strokes) {
        expectPosition((stroke.start + stroke.end) * 0.5, camera_.worldToScreenLogical({0, 1}) + glm::dvec2{0, -style_.offsetPx});
        EXPECT_NEAR(glm::length(stroke.end - stroke.start), style_.lengthPx, kTolerance);
        EXPECT_DOUBLE_EQ(stroke.start.y, stroke.end.y);
    }

    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Horizontal, {lineRef}}));

    rebuild();

    const auto markers = attachedTo(line.value());
    ASSERT_EQ(markers.size(), 2u);
    EXPECT_EQ(markers[0]->type, ConstraintType::Horizontal);
    EXPECT_EQ(markers[1]->type, ConstraintType::Fix);
    EXPECT_EQ(markers[1]->constraints, (std::vector<ConstraintId>{fix.value()}));
    ASSERT_EQ(markers[1]->strokes.size(), 1u);
    for (const auto& stroke : markers[1]->strokes) {
        expectPosition((stroke.start + stroke.end) * 0.5, camera_.worldToScreenLogical({2.0 / 3.0, 1}) + glm::dvec2{0, -style_.offsetPx});
    }
}

TEST_F(ConstraintLayoutBehavior, FixGlyphUsesTheReferencedPointRatherThanALineSlot) {
    const auto line = sketch_->addLine({-2, 1}, {2, 1});
    ASSERT_TRUE(line);
    const GeometryRef endpoint{line.value(), SubElement::End};
    const auto fix = sketch_->addConstraint({ConstraintType::Fix, {endpoint}, std::nullopt, Vec2{2, 1}});
    ASSERT_TRUE(fix);

    style_.offsetPx = 7.5;
    for (const double ratio : {1.0, 2.0}) {
        for (const double zoom : {25.0, 200.0}) {
            camera_.setViewport(800, 600, ratio);
            camera_.setZoom(zoom);
            rebuild();

            ASSERT_EQ(layout_.markers().size(), 1u);
            const auto& marker = layout_.markers().front();
            EXPECT_EQ(marker.type, ConstraintType::Fix);
            EXPECT_EQ(marker.attachment, endpoint);
            EXPECT_EQ(marker.constraints, (std::vector<ConstraintId>{fix.value()}));
            EXPECT_TRUE(marker.strokes.empty());
            EXPECT_TRUE(marker.dots.empty());
            EXPECT_TRUE(marker.arcs.empty());
            ASSERT_EQ(marker.circles.size(), 1u);
            expectPosition(marker.circles.front().center, camera_.worldToScreenLogical({2, 1}));
            EXPECT_DOUBLE_EQ(marker.circles.front().radiusPx, style_.offsetPx);
            EXPECT_DOUBLE_EQ(marker.hitRadiusPx, style_.hitTolerancePx + style_.stroke.widthPx * 0.5);
        }
    }
}

TEST_F(ConstraintLayoutBehavior, FixPointRingsSupportStandalonePointsAndCurveCenters) {
    const auto point = sketch_->addPoint({-1, 2});
    const auto circle = sketch_->addCircle({3, 1}, 2);
    ASSERT_TRUE(point);
    ASSERT_TRUE(circle);
    const std::vector<GeometryRef> refs{{point.value(), SubElement::Whole}, {circle.value(), SubElement::Center}};
    for (const auto ref : refs) {
        const auto position = sketch_->pointPosition(ref);
        ASSERT_TRUE(position);
        ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {ref}, std::nullopt, position.value()}));
    }
    rebuild();

    ASSERT_EQ(layout_.markers().size(), refs.size());
    for (const auto ref : refs) {
        const auto markers = attachedTo(ref.entity);
        ASSERT_EQ(markers.size(), 1u);
        EXPECT_EQ(markers.front()->attachment, ref);
        ASSERT_EQ(markers.front()->circles.size(), 1u);
        const auto position = sketch_->pointPosition(ref);
        ASSERT_TRUE(position);
        expectPosition(markers.front()->circles.front().center, camera_.worldToScreenLogical({position.value().x, position.value().y}));
        EXPECT_DOUBLE_EQ(markers.front()->circles.front().radiusPx, style_.offsetPx);
    }
}

TEST_F(ConstraintLayoutBehavior, FixLineOffsetIsIndependentOfDirectionZoomAndDpi) {
    const auto first = sketch_->addLine({-2, -1}, {2, 1});
    const auto reversed = sketch_->addLine({2, 1}, {-2, -1});
    ASSERT_TRUE(first);
    ASSERT_TRUE(reversed);
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{first.value(), SubElement::Whole}}}));
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{reversed.value(), SubElement::Whole}}}));
    for (const double ratio : {1.0, 2.0}) {
        for (const double zoom : {25.0, 200.0}) {
            camera_.setViewport(800, 600, ratio);
            camera_.setZoom(zoom);
            rebuild();

            ASSERT_EQ(layout_.markers().size(), 2u);
            const auto tangent = glm::normalize(camera_.worldToScreenLogical({2, 1}) - camera_.worldToScreenLogical({-2, -1}));
            const glm::dvec2 normal{tangent.y, -tangent.x};
            for (const auto& marker : layout_.markers()) {
                ASSERT_EQ(marker.strokes.size(), 1u);
                const auto& stroke = marker.strokes.front();
                expectPosition((stroke.start + stroke.end) * 0.5, camera_.worldToScreenLogical({0, 0}) + style_.offsetPx * normal);
                expectPosition(stroke.end - stroke.start, style_.lengthPx * tangent);
            }
        }
    }
}

TEST_F(ConstraintLayoutBehavior, FixedCircleHasAConstantPixelInset) {
    const auto circle = sketch_->addCircle({1, 2}, 2);
    ASSERT_TRUE(circle);
    const GeometryRef ref{circle.value(), SubElement::Whole};
    const auto fix = sketch_->addConstraint({ConstraintType::Fix, {ref}});
    ASSERT_TRUE(fix);
    for (const double ratio : {1.0, 2.0}) {
        for (const double zoom : {25.0, 200.0}) {
            camera_.setViewport(800, 600, ratio);
            camera_.setZoom(zoom);
            rebuild();

            ASSERT_EQ(layout_.markers().size(), 1u);
            const auto& marker = layout_.markers().front();
            EXPECT_EQ(marker.attachment, ref);
            EXPECT_EQ(marker.constraints, (std::vector<ConstraintId>{fix.value()}));
            EXPECT_TRUE(marker.strokes.empty());
            EXPECT_TRUE(marker.dots.empty());
            EXPECT_TRUE(marker.arcs.empty());
            ASSERT_EQ(marker.circles.size(), 1u);
            expectPosition(marker.circles.front().center, camera_.worldToScreenLogical({1, 2}));
            EXPECT_DOUBLE_EQ(2 * zoom - marker.circles.front().radiusPx, style_.offsetPx);
        }
    }
}

TEST_F(ConstraintLayoutBehavior, FixedArcsKeepTheirSweepAndConstantPixelInset) {
    const Vec2 center{1, 2};
    const std::vector<Arc2> arcs{{center, {3, 2}, {1, 4}}, {center, {1, 0}, {3, 2}}, {center, {3, 2}, {1, 0}}};
    std::vector<EntityId> ids;
    for (const auto& arc : arcs) {
        const auto id = sketch_->addArc(arc.center, arc.start, arc.end);
        ASSERT_TRUE(id);
        ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{id.value(), SubElement::Whole}}}));
        ids.push_back(id.value());
    }
    for (const double ratio : {1.0, 2.0}) {
        for (const double zoom : {25.0, 200.0}) {
            camera_.setViewport(800, 600, ratio);
            camera_.setZoom(zoom);
            rebuild();

            ASSERT_EQ(layout_.markers().size(), arcs.size());
            for (size_t i = 0; i < ids.size(); ++i) {
                const auto markers = attachedTo(ids[i]);
                ASSERT_EQ(markers.size(), 1u);
                EXPECT_TRUE(markers.front()->circles.empty());
                EXPECT_TRUE(markers.front()->strokes.empty());
                ASSERT_EQ(markers.front()->arcs.size(), 1u);
                const auto& inset = markers.front()->arcs.front();
                expectPosition(inset.center, camera_.worldToScreenLogical({center.x, center.y}));
                EXPECT_DOUBLE_EQ(2 * zoom - inset.radiusPx, style_.offsetPx);
                EXPECT_DOUBLE_EQ(inset.startAngle, std::atan2(arcs[i].start.y - center.y, arcs[i].start.x - center.x));
                EXPECT_DOUBLE_EQ(inset.endAngle, std::atan2(arcs[i].end.y - center.y, arcs[i].end.x - center.x));
            }
        }
    }
}

TEST_F(ConstraintLayoutBehavior, FixedCurvesSmallerThanTheInsetAreSkipped) {
    const auto circle = sketch_->addCircle({0, 0}, 1);
    const auto arc = sketch_->addArc({0, 0}, {1, 0}, {0, 1});
    ASSERT_TRUE(circle);
    ASSERT_TRUE(arc);
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{circle.value(), SubElement::Whole}}}));
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Fix, {{arc.value(), SubElement::Whole}}}));
    for (const double zoom : {style_.offsetPx * 0.5, style_.offsetPx}) {
        camera_.setZoom(zoom);
        rebuild();
        EXPECT_TRUE(layout_.markers().empty());
    }
    camera_.setZoom(style_.offsetPx * 2.0);
    rebuild();
    EXPECT_EQ(layout_.markers().size(), 2u);
}

TEST_F(ConstraintLayoutBehavior, CoincidentDotRadiusAndHitAreaUseLogicalPixels) {
    const auto first = sketch_->addPoint({1, 2});
    const auto second = sketch_->addPoint({1, 2});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(sketch_->addCoincident({first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}, CoincidentPlacement::Preserve));
    style_.point.radius = 3.5f;
    style_.stroke.widthPx = 8;
    style_.hitTolerancePx = 6;
    for (const double ratio : {1.0, 2.0}) {
        for (const double zoom : {25.0, 200.0}) {
            SCOPED_TRACE(testing::Message() << "DPR=" << ratio << ", zoom=" << zoom);
            camera_.setViewport(800, 600, ratio);
            camera_.setZoom(zoom);

            rebuild();

            ASSERT_EQ(layout_.markers().size(), 1u);
            ASSERT_EQ(layout_.markers().front().dots.size(), 1u);
            const auto& dot = layout_.markers().front().dots.front();
            expectPosition(dot.center, camera_.worldToScreenLogical({1, 2}));
            EXPECT_DOUBLE_EQ(dot.radiusPx, style_.point.radius);
            EXPECT_DOUBLE_EQ(dot.hitRadiusPx, style_.point.radius + style_.hitTolerancePx);
        }
    }
}

TEST_F(ConstraintLayoutBehavior, InvalidDimensionsOnlySuppressTheirPrimitiveFamily) {
    const auto line = sketch_->addLine({-2, 0}, {2, 0});
    const auto first = sketch_->addPoint({1, 2});
    const auto second = sketch_->addPoint({1, 2});
    ASSERT_TRUE(line);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Horizontal, {{line.value(), SubElement::Whole}}}));
    ASSERT_TRUE(sketch_->addCoincident({first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}, CoincidentPlacement::Preserve));
    style_.lengthPx = 0;

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 1u);
    EXPECT_EQ(layout_.markers().front().type, ConstraintType::Coincident);
    ASSERT_EQ(layout_.markers().front().dots.size(), 1u);
    EXPECT_TRUE(layout_.markers().front().strokes.empty());

    style_.lengthPx = 20;
    style_.point.radius = std::numeric_limits<float>::quiet_NaN();

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 1u);
    EXPECT_EQ(layout_.markers().front().type, ConstraintType::Horizontal);
    EXPECT_EQ(layout_.markers().front().strokes.size(), 4u);
    EXPECT_TRUE(layout_.markers().front().dots.empty());
}

TEST_F(ConstraintLayoutBehavior, RebuildKeepsUserLabelPlacementSeparateFromAutomaticPointPlacement) {
    const auto first = sketch_->addPoint({1, 2});
    const auto second = sketch_->addPoint({1, 2});
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    const auto coincident = sketch_->addCoincident({first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}, CoincidentPlacement::Preserve);
    ASSERT_TRUE(coincident);
    const Vec2 labelPosition{30, -40};
    presentation_.labelPlacements.emplace(coincident.value(), app::ConstraintLabelPlacement{labelPosition});
    rebuild();
    camera_.setCenter({2, -1});
    camera_.setZoom(150);

    rebuild();

    ASSERT_EQ(presentation_.labelPlacements.size(), 1u);
    EXPECT_EQ(presentation_.labelPlacements.at(coincident.value()).position, labelPosition);
    ASSERT_EQ(layout_.markers().size(), 1u);
    ASSERT_EQ(layout_.markers().front().dots.size(), 1u);
    expectPosition(layout_.markers().front().dots.front().center, camera_.worldToScreenLogical({1, 2}));
    ASSERT_TRUE(sketch_->removeConstraint(coincident.value()));
    rebuild();
    EXPECT_TRUE(layout_.markers().empty());
    EXPECT_EQ(presentation_.labelPlacements.at(coincident.value()).position, labelPosition);
}

TEST_F(ConstraintLayoutBehavior, LengthUsesDimensionPipelineAndExplicitPlacement) {
    const auto line = sketch_->addLine({-2, 0}, {2, 0});
    ASSERT_TRUE(line);
    const auto length = sketch_->addConstraint({ConstraintType::Length, {{line.value(), SubElement::Whole}}, 4.0});
    ASSERT_TRUE(length);
    presentation_.labelPlacements.emplace(length.value(), app::ConstraintLabelPlacement{{3, 1}});

    rebuild();

    ASSERT_EQ(layout_.markers().size(), 1u);
    const auto& marker = layout_.markers().front();
    EXPECT_EQ(marker.constraints, (std::vector<ConstraintId>{length.value()}));
    EXPECT_FALSE(marker.strokes.empty());
    ASSERT_EQ(marker.labels.size(), 1u);
    EXPECT_EQ(marker.labels.front().text, "4");
    expectPosition(marker.labels.front().center, camera_.worldToScreenLogical({3, 1}));
}
}  // namespace
