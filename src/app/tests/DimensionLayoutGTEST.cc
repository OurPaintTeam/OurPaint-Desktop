#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>

#include "../viewport/constraints/ConstraintLayout.h"
#include "../viewport/constraints/ConstraintMarkerStyle.h"
#include "../viewport/constraints/ConstraintPresentationState.h"
#include "../viewport/constraints/DimensionLayout.h"
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

class DimensionLayoutBehavior : public testing::Test {
protected:
    void SetUp() override {
        camera_.setViewport(800, 600);
        camera_.setCenter({0, 0});
        camera_.setZoom(100);
    }

    GeometryRef point(Vec2 position) {
        const auto result = sketch_.addPoint(position);
        EXPECT_TRUE(result);
        return {result.value(), SubElement::Whole};
    }

    GeometryRef line(Vec2 start, Vec2 end) {
        const auto result = sketch_.addLine(start, end);
        EXPECT_TRUE(result);
        return {result.value(), SubElement::Whole};
    }

    void layout(ConstraintType type, std::vector<GeometryRef> refs, double value) {
        markers_.clear();
        // Presentation also supports definitions whose solver adapters are not implemented yet.
        app::appendDimension({id_, {type, std::move(refs), value}}, sketch_, camera_, style_, presentation_, textMeasurer_, markers_);
    }

    void place(Vec2 position) { presentation_.labelPlacements[id_] = {position}; }

    const app::ConstraintLabel& label() const { return markers_.front().labels.front(); }

    Sketch sketch_;
    Camera2D camera_;
    app::ConstraintMarkerStyle style_;
    app::ConstraintPresentationState presentation_;
    TestTextMeasurer textMeasurer_;
    std::vector<app::ConstraintMarker> markers_;
    ConstraintId id_{42};
};

TEST_F(DimensionLayoutBehavior, DistanceDisplaysTargetAndLeavesMeasuredTextGap) {
    const auto a = point({-1, 0});
    const auto b = point({1, 0});
    // The target value may differ from current geometry during an unsolved edit.
    layout(ConstraintType::Distance, {a, b}, 12.3456);

    ASSERT_EQ(markers_.size(), 1u);
    ASSERT_EQ(markers_.front().labels.size(), 1u);
    EXPECT_EQ(label().text, "12.3456");
    expectPosition(label().center, {400, 270});
    EXPECT_DOUBLE_EQ(label().rotationRad, 0.0);
    const double halfGap = textMeasurer_.measureText(label().text, style_.dimension.textHeightPx)->width * 0.5 + style_.dimension.textGapPx;
    int baselineParts = 0;
    int witnesses = 0;
    for (const auto& stroke : markers_.front().strokes) {
        if (stroke.start.y == 270 && stroke.end.y == 270) {
            ++baselineParts;
            EXPECT_TRUE(std::max(stroke.start.x, stroke.end.x) <= 400 - halfGap + kTolerance ||
                        std::min(stroke.start.x, stroke.end.x) >= 400 + halfGap - kTolerance);
        }
        if (stroke.start.x == stroke.end.x) {
            ++witnesses;
            EXPECT_DOUBLE_EQ(stroke.start.y, 300 - style_.dimension.extensionGapPx);
            EXPECT_DOUBLE_EQ(stroke.end.y, 270 - style_.dimension.extensionOverrunPx);
        }
    }
    EXPECT_EQ(baselineParts, 2);
    EXPECT_EQ(witnesses, 2);
    EXPECT_TRUE(presentation_.labelPlacements.empty());
}

TEST_F(DimensionLayoutBehavior, ManualPlacementExtendsBothEndsAndWitnesses) {
    const auto a = point({-1, 0});
    const auto b = point({1, 0});
    for (double x : {-3.0, 3.0}) {
        place({x, 2});
        layout(ConstraintType::Distance, {a, b}, 2);
        ASSERT_EQ(markers_.size(), 1u);
        expectPosition(label().center, camera_.worldToScreenLogical({x, 2}));
        double low = std::numeric_limits<double>::infinity();
        double high = -low;
        int witnesses = 0;
        for (const auto& stroke : markers_.front().strokes) {
            if (stroke.start.y == 100 && stroke.end.y == 100) {
                low = std::min({low, stroke.start.x, stroke.end.x});
                high = std::max({high, stroke.start.x, stroke.end.x});
            }
            if (stroke.start.x == stroke.end.x) {
                ++witnesses;
                EXPECT_DOUBLE_EQ(stroke.end.y, 100 - style_.dimension.extensionOverrunPx);
            }
        }
        const double halfGap = label().sizePx.x * 0.5 + style_.dimension.textGapPx;
        EXPECT_LT(low, label().center.x - halfGap);
        EXPECT_GT(high, label().center.x + halfGap);
        EXPECT_EQ(witnesses, 2);
    }
}

TEST_F(DimensionLayoutBehavior, PlacementSurvivesCameraChangesWithConstantLogicalTextSize) {
    const auto a = point({-1, 0});
    const auto b = point({1, 0});
    place({3, 2});
    layout(ConstraintType::Distance, {a, b}, 2);
    ASSERT_EQ(markers_.size(), 1u);
    const auto size = label().sizePx;
    camera_.setZoom(20);
    camera_.setCenter({1, 1});
    camera_.setViewport(800, 600, 2.0);
    layout(ConstraintType::Distance, {a, b}, 2);

    ASSERT_EQ(markers_.size(), 1u);
    expectPosition(label().center, camera_.worldToScreenLogical({3, 2}));
    expectPosition(label().sizePx, size);
    EXPECT_DOUBLE_EQ(label().heightPx, style_.dimension.textHeightPx);
    EXPECT_EQ(presentation_.labelPlacements.at(id_).position, (Vec2{3, 2}));
}

TEST_F(DimensionLayoutBehavior, ReversingPointReferencesKeepsAutomaticSideAndReadableText) {
    const auto a = point({-1, -1});
    const auto b = point({1, 1});
    layout(ConstraintType::Distance, {a, b}, 3);
    ASSERT_EQ(markers_.size(), 1u);
    const auto forward = label();
    layout(ConstraintType::Distance, {b, a}, 3);

    ASSERT_EQ(markers_.size(), 1u);
    expectPosition(label().center, forward.center);
    EXPECT_DOUBLE_EQ(label().rotationRad, forward.rotationRad);
    EXPECT_LE(std::abs(label().rotationRad), std::numbers::pi / 2.0);
}

TEST_F(DimensionLayoutBehavior, HorizontalAndVerticalDistancesUseAxisProjections) {
    const auto a = point({-1, -1});
    const auto b = point({1, 1});
    place({3, 2});
    layout(ConstraintType::HorizontalDistance, {a, b}, 2);
    ASSERT_EQ(markers_.size(), 1u);
    EXPECT_DOUBLE_EQ(label().rotationRad, 0.0);
    const auto hasStroke = [&](glm::dvec2 start, glm::dvec2 end) {
        return std::ranges::any_of(markers_.front().strokes, [&](const auto& stroke) {
            return glm::length(stroke.start - start) < kTolerance && glm::length(stroke.end - end) < kTolerance;
        });
    };
    EXPECT_TRUE(hasStroke({300, 397}, {300, 96}));
    EXPECT_TRUE(hasStroke({500, 197}, {500, 96}));

    layout(ConstraintType::VerticalDistance, {a, b}, 2);
    ASSERT_EQ(markers_.size(), 1u);
    EXPECT_DOUBLE_EQ(label().rotationRad, -std::numbers::pi / 2.0);
    EXPECT_TRUE(hasStroke({303, 400}, {704, 400}));
    EXPECT_TRUE(hasStroke({503, 200}, {704, 200}));
}

TEST_F(DimensionLayoutBehavior, ZeroDistanceAndInvalidPlacementRemainFinite) {
    const auto a = point({0, 0});
    const auto b = point({0, 0});
    place({std::numeric_limits<double>::quiet_NaN(), 1});
    layout(ConstraintType::Distance, {a, b}, 0);
    ASSERT_EQ(markers_.size(), 1u);
    EXPECT_DOUBLE_EQ(label().center.y, 270);
    EXPECT_GT(label().center.x - label().sizePx.x * 0.5 - style_.dimension.textGapPx, 400 + style_.dimension.arrowLengthPx);
    EXPECT_EQ(label().text, "0");
    for (const auto& stroke : markers_.front().strokes) {
        EXPECT_TRUE(std::isfinite(stroke.start.x) && std::isfinite(stroke.start.y));
        EXPECT_TRUE(std::isfinite(stroke.end.x) && std::isfinite(stroke.end.y));
    }
    EXPECT_TRUE(std::isnan(presentation_.labelPlacements.at(id_).position.x));
}

TEST_F(DimensionLayoutBehavior, PointLineAndParallelLinesMeasurePerpendicularSeparation) {
    const auto a = point({0, 1});
    const auto b = line({-1, 0}, {1, 0});
    const auto c = line({-1, 1}, {1, 1});
    const auto d = line({0, 0}, {0, 1});
    layout(ConstraintType::Distance, {a, b}, 1);
    ASSERT_EQ(markers_.size(), 1u);
    expectPosition(label().center, {370, 250});
    EXPECT_DOUBLE_EQ(label().rotationRad, -std::numbers::pi / 2.0);
    layout(ConstraintType::Distance, {b, c}, 1);
    ASSERT_EQ(markers_.size(), 1u);
    expectPosition(label().center, {370, 250});
    layout(ConstraintType::Distance, {b, d}, 1);
    EXPECT_TRUE(markers_.empty());
}

TEST_F(DimensionLayoutBehavior, RadiusAndDiameterShareTheLinearPipelineAndRespectManualDirection) {
    const auto circle = sketch_.addCircle({0, 0}, 1);
    ASSERT_TRUE(circle);
    const GeometryRef ref{circle.value(), SubElement::Whole};
    place({3, 0});
    for (const auto type : {ConstraintType::Radius, ConstraintType::Diameter}) {
        layout(type, {ref}, type == ConstraintType::Radius ? 1 : 2);
        ASSERT_EQ(markers_.size(), 1u);
        EXPECT_EQ(label().text, type == ConstraintType::Radius ? "R 1" : "D 2");
        expectPosition(label().center, {700, 300});
        EXPECT_DOUBLE_EQ(label().rotationRad, 0.0);
        const auto& strokes = markers_.front().strokes;
        EXPECT_TRUE(std::ranges::any_of(strokes, [](const auto& stroke) { return stroke.start == glm::dvec2(500, 300); }));
        if (type == ConstraintType::Diameter) {
            EXPECT_TRUE(std::ranges::any_of(strokes, [](const auto& stroke) { return stroke.start == glm::dvec2(300, 300); }));
        }
    }
}

TEST_F(DimensionLayoutBehavior, ArcRadiusDefaultsToItsCounterclockwiseMidpoint) {
    const auto arc = sketch_.addArc({0, 0}, {1, 0}, {0, 1});
    ASSERT_TRUE(arc);
    layout(ConstraintType::Radius, {{arc.value(), SubElement::Whole}}, 1);
    ASSERT_EQ(markers_.size(), 1u);
    const auto delta = label().center - camera_.worldToScreenLogical({0, 0});
    EXPECT_GT(delta.x, 0);
    EXPECT_NEAR(delta.x, -delta.y, kTolerance);
    EXPECT_DOUBLE_EQ(label().rotationRad, -std::numbers::pi / 4.0);
}

TEST_F(DimensionLayoutBehavior, AngleUsesArcGapAndManualRadius) {
    const auto a = line({0, 0}, {1, 0});
    const auto b = line({0, 0}, {0, 1});
    place({2, 2});
    layout(ConstraintType::Angle, {a, b}, std::numbers::pi / 2.0);
    ASSERT_EQ(markers_.size(), 1u);
    EXPECT_EQ(label().text, "90 deg");
    expectPosition(label().center, {600, 100});
    ASSERT_EQ(markers_.front().arcs.size(), 2u);
    const auto& arcs = markers_.front().arcs;
    for (const auto& arc : arcs) {
        expectPosition(arc.center, {400, 300});
        EXPECT_NEAR(arc.radiusPx, std::hypot(200.0, 200.0), kTolerance);
        EXPECT_GT(arc.endAngle, arc.startAngle);
    }
    EXPECT_NEAR(arcs.front().startAngle, 0.0, kTolerance);
    EXPECT_NEAR(arcs.back().endAngle, std::numbers::pi / 2.0, kTolerance);
    EXPECT_LT(arcs.front().endAngle, std::numbers::pi / 4.0);
    EXPECT_GT(arcs.back().startAngle, std::numbers::pi / 4.0);
}

TEST_F(DimensionLayoutBehavior, AngleAcrossZeroAndOutsideSectorDoesNotBecomeAFullCircle) {
    const auto a = line({0, 0}, {1, -1});
    const auto b = line({0, 0}, {1, 1});
    place({-2, 0});
    layout(ConstraintType::Angle, {a, b}, std::numbers::pi / 2.0);
    ASSERT_EQ(markers_.size(), 1u);
    expectPosition(label().center, {200, 300});
    ASSERT_EQ(markers_.front().arcs.size(), 1u);
    const auto& arc = markers_.front().arcs.front();
    EXPECT_NEAR(arc.startAngle, -std::numbers::pi / 4.0, kTolerance);
    EXPECT_NEAR(arc.endAngle, std::numbers::pi / 4.0, kTolerance);
    EXPECT_FALSE(markers_.front().strokes.empty());
}

TEST_F(DimensionLayoutBehavior, ParallelAngleUsesLeadersWithoutAFictitiousVertex) {
    const auto a = line({-1, 0}, {1, 0});
    const auto b = line({1, 1}, {-1, 1});
    layout(ConstraintType::Angle, {a, b}, std::numbers::pi);
    ASSERT_EQ(markers_.size(), 1u);
    EXPECT_EQ(label().text, "180 deg");
    EXPECT_TRUE(markers_.front().arcs.empty());
    EXPECT_FALSE(markers_.front().strokes.empty());
}

TEST_F(DimensionLayoutBehavior, MissingFontMetricsSuppressOnlyDimensions) {
    const auto a = line({-1, 0}, {1, 0});
    ASSERT_TRUE(sketch_.addConstraint({ConstraintType::Length, {a}, 2}));
    ASSERT_TRUE(sketch_.addConstraint({ConstraintType::Horizontal, {a}}));
    textMeasurer_.available = false;
    app::ConstraintLayout layout;
    layout.rebuild(sketch_, camera_, style_, presentation_, textMeasurer_);
    ASSERT_EQ(layout.markers().size(), 1u);
    EXPECT_EQ(layout.markers().front().type, ConstraintType::Horizontal);
    EXPECT_TRUE(layout.markers().front().labels.empty());
}
}  // namespace
