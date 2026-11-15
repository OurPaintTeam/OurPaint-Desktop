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

// TEST_F(ConstraintLayoutBehavior, CoincidentDotRadiusAndHitAreaUseLogicalPixels) {
//     const auto first = sketch_->addPoint({1, 2});
//     const auto second = sketch_->addPoint({1, 2});
//     ASSERT_TRUE(first);
//     ASSERT_TRUE(second);
//     ASSERT_TRUE(sketch_->addCoincident({first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}, CoincidentPlacement::Preserve));
//     style_.point.radius = 3.5f;
//     style_.stroke.widthPx = 8;
//     style_.hitTolerancePx = 6;
//     for (const double ratio : {1.0, 2.0}) {
//         for (const double zoom : {25.0, 200.0}) {
//             SCOPED_TRACE(testing::Message() << "DPR=" << ratio << ", zoom=" << zoom);
//             camera_.setViewport(800, 600, ratio);
//             camera_.setZoom(zoom);
//
//             rebuild();
//
//             ASSERT_EQ(layout_.markers().size(), 1u);
//             ASSERT_EQ(layout_.markers().front().dots.size(), 1u);
//             const auto& dot = layout_.markers().front().dots.front();
//             expectPosition(dot.center, camera_.worldToScreenLogical({1, 2}));
//             EXPECT_DOUBLE_EQ(dot.radiusPx, style_.point.radius);
//             EXPECT_DOUBLE_EQ(dot.hitRadiusPx, style_.point.radius + style_.hitTolerancePx);
//         }
//     }
// }

// TEST_F(ConstraintLayoutBehavior, InvalidDimensionsOnlySuppressTheirPrimitiveFamily) {
//     const auto line = sketch_->addLine({-2, 0}, {2, 0});
//     const auto first = sketch_->addPoint({1, 2});
//     const auto second = sketch_->addPoint({1, 2});
//     ASSERT_TRUE(line);
//     ASSERT_TRUE(first);
//     ASSERT_TRUE(second);
//     ASSERT_TRUE(sketch_->addConstraint({ConstraintType::Horizontal, {{line.value(), SubElement::Whole}}}));
//     ASSERT_TRUE(sketch_->addCoincident({first.value(), SubElement::Whole}, {second.value(), SubElement::Whole}, CoincidentPlacement::Preserve));
//     style_.lengthPx = 0;
//
//     rebuild();
//
//     ASSERT_EQ(layout_.markers().size(), 1u);
//     EXPECT_EQ(layout_.markers().front().type, ConstraintType::Coincident);
//     ASSERT_EQ(layout_.markers().front().dots.size(), 1u);
//     EXPECT_TRUE(layout_.markers().front().strokes.empty());
//
//     style_.lengthPx = 20;
//     style_.point.radius = std::numeric_limits<float>::quiet_NaN();
//
//     rebuild();
//
//     ASSERT_EQ(layout_.markers().size(), 1u);
//     EXPECT_EQ(layout_.markers().front().type, ConstraintType::Horizontal);
//     EXPECT_EQ(layout_.markers().front().strokes.size(), 4u);
//     EXPECT_TRUE(layout_.markers().front().dots.empty());
// }

}  // namespace
