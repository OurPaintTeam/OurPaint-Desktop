#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "../viewport/OverlayModel.h"
#include "../viewport/SnapFeedbackStyle.h"
#include "../viewport/render/SnapFeedbackBuilder.h"
#include "Camera2D.h"
#include "RenderScene.h"
#include "sketch/Sketch.h"

namespace {
using namespace core::sketch;

constexpr int kSourceLayer = 2100;
constexpr int kGuideLayer = 2110;
constexpr int kGlyphLayer = 2120;
constexpr double kCoordinateTolerance = 1e-4;

const render::DrawLayer* findLayer(const render::RenderScene& scene, int order) {
    const auto found = std::find_if(scene.layers.begin(), scene.layers.end(), [order](const auto& layer) { return layer.order == order; });
    return found == scene.layers.end() ? nullptr : &*found;
}

std::vector<render::Line> linesIn(const render::RenderScene& scene, int order) {
    std::vector<render::Line> result;
    if (const auto* layer = findLayer(scene, order)) {
        for (const auto& batch : layer->lineBatches) {
            result.insert(result.end(), batch.lines.begin(), batch.lines.end());
        }
    }
    return result;
}

std::vector<render::Marker> markersIn(const render::RenderScene& scene, int order) {
    std::vector<render::Marker> result;
    if (const auto* layer = findLayer(scene, order)) {
        for (const auto& batch : layer->markerBatches) {
            result.insert(result.end(), batch.markers.begin(), batch.markers.end());
        }
    }
    return result;
}

bool hasLine(const std::vector<render::Line>& lines, Vec2 start, Vec2 end) {
    const auto near = [](double actual, double expected) { return std::abs(actual - expected) <= kCoordinateTolerance; };
    return std::any_of(lines.begin(), lines.end(), [&](const auto& line) {
        return (near(line.x1, start.x) && near(line.y1, start.y) && near(line.x2, end.x) && near(line.y2, end.y)) ||
               (near(line.x2, start.x) && near(line.y2, start.y) && near(line.x1, end.x) && near(line.y1, end.y));
    });
}

app::SnapFeedback feedbackAt(Vec2 position = {0, 0}) {
    app::SnapFeedback feedback;
    feedback.position = position;
    return feedback;
}

TEST(SnapFeedbackOwnership, StoresAndCopiesDetachedValues) {
    OverlayModel overlay{};
    auto feedback = feedbackAt({1, 2});
    feedback.sources = {{EntityId(7), SubElement::Start}};
    feedback.guides = {{{{0, 0}, {1, 2}}, app::SnapGuideExtent::Segment}};
    overlay.setSnapFeedback(feedback);

    feedback.position = {20, 30};
    feedback.sources.clear();
    feedback.guides.front().line.end = {40, 50};
    ASSERT_TRUE(overlay.snapFeedback());
    EXPECT_EQ(overlay.snapFeedback()->position, (Vec2{1, 2}));
    EXPECT_EQ(overlay.snapFeedback()->sources, (std::vector<GeometryRef>{{EntityId(7), SubElement::Start}}));
    ASSERT_EQ(overlay.snapFeedback()->guides.size(), 1u);
    EXPECT_EQ(overlay.snapFeedback()->guides.front().line.end, (Vec2{1, 2}));

    const OverlayModel copied = overlay;
    overlay.clearSnap();
    EXPECT_FALSE(overlay.snapFeedback());
    ASSERT_TRUE(copied.snapFeedback());
    EXPECT_EQ(copied.snapFeedback()->position, (Vec2{1, 2}));
    EXPECT_EQ(copied.snapFeedback()->guides.front().line.end, (Vec2{1, 2}));
}

TEST(SnapFeedbackOwnership, ClearingSnapPreservesPreviewOperandsAndSelection) {
    OverlayModel overlay{};
    const GeometryRef selected{EntityId(3), SubElement::End};
    const GeometryRef operand{EntityId(4), SubElement::Whole};
    overlay.selection_.model.replace(selected);
    overlay.selectionRect_ = OverlayModel::Rect{};
    overlay.constraintRefs_.push_back(operand);
    overlay.points_.push_back(OverlayModel::Point{});
    overlay.lines_.push_back(OverlayModel::Line{});
    overlay.circles_.push_back(OverlayModel::Circle{});
    overlay.arcs_.push_back(OverlayModel::Arc{});
    overlay.setSnapFeedback(feedbackAt());

    overlay.clearSnap();

    EXPECT_FALSE(overlay.snapFeedback());
    EXPECT_EQ(overlay.selection_.model.items(), (std::vector<GeometryRef>{selected}));
    EXPECT_TRUE(overlay.selectionRect_);
    EXPECT_EQ(overlay.constraintRefs_, (std::vector<GeometryRef>{operand}));
    EXPECT_EQ(overlay.points_.size(), 1u);
    EXPECT_EQ(overlay.lines_.size(), 1u);
    EXPECT_EQ(overlay.circles_.size(), 1u);
    EXPECT_EQ(overlay.arcs_.size(), 1u);
}

TEST(SnapFeedbackOwnership, PreviewAndSelectionCleanupPreserveSnapUntilFullCleanup) {
    OverlayModel overlay{};
    const GeometryRef selected{EntityId(3), SubElement::Whole};
    overlay.selection_.model.replace(selected);
    overlay.selectionRect_ = OverlayModel::Rect{};
    overlay.points_.push_back(OverlayModel::Point{});
    overlay.constraintRefs_.push_back(selected);
    overlay.setSnapFeedback(feedbackAt({2, 3}));

    overlay.clearPreview();
    EXPECT_TRUE(overlay.points_.empty());
    EXPECT_TRUE(overlay.constraintRefs_.empty());
    EXPECT_EQ(overlay.selection_.model.items(), (std::vector<GeometryRef>{selected}));
    ASSERT_TRUE(overlay.snapFeedback());
    EXPECT_EQ(overlay.snapFeedback()->position, (Vec2{2, 3}));

    overlay.clearSelection();
    EXPECT_TRUE(overlay.selection_.model.empty());
    EXPECT_FALSE(overlay.selectionRect_);
    ASSERT_TRUE(overlay.snapFeedback());
    EXPECT_EQ(overlay.snapFeedback()->position, (Vec2{2, 3}));

    overlay.clear();
    EXPECT_FALSE(overlay.snapFeedback());
}

class SnapFeedbackRendering : public testing::Test {
protected:
    void SetUp() override {
        auto created = Sketch::create();
        ASSERT_TRUE(created);
        sketch_ = std::move(created.value());
        camera_.setViewport(800, 600);
        camera_.setZoom(100);
    }

    render::RenderScene renderFeedback(const app::SnapFeedback& feedback) const {
        render::RenderScene scene;
        app::appendSnapFeedback(feedback, *sketch_, camera_, style_, scene);
        return scene;
    }

    std::unique_ptr<Sketch> sketch_;
    Camera2D camera_;
    app::SnapFeedbackStyle style_;
};

TEST_F(SnapFeedbackRendering, HighlightsBothIntersectionSourcesWithoutEditingOrSelectingThem) {
    const auto horizontal = sketch_->addLine({-2, 0}, {2, 0});
    const auto vertical = sketch_->addLine({0, -2}, {0, 2});
    const auto selected = sketch_->addPoint({3, 1});
    ASSERT_TRUE(horizontal);
    ASSERT_TRUE(vertical);
    ASSERT_TRUE(selected);
    OverlayModel overlay{};
    const GeometryRef selectedRef{selected.value(), SubElement::Whole};
    overlay.selection_.model.replace(selectedRef);
    auto feedback = feedbackAt();
    feedback.glyph = app::SnapGlyph::Intersection;
    feedback.sources = {{horizontal.value(), SubElement::Whole}, {vertical.value(), SubElement::Whole}};
    overlay.setSnapFeedback(feedback);

    render::RenderScene scene;
    render::DrawLayer existing;
    existing.name = "Existing selection";
    existing.order = 1000;
    scene.layers.push_back(existing);
    app::appendSnapFeedback(*overlay.snapFeedback(), *sketch_, camera_, style_, scene);

    const auto* sources = findLayer(scene, kSourceLayer);
    ASSERT_NE(sources, nullptr);
    EXPECT_EQ(sources->coordinateSpace, render::CoordinateSpace::World);
    const auto lines = linesIn(scene, kSourceLayer);
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(hasLine(lines, {-2, 0}, {2, 0}));
    EXPECT_TRUE(hasLine(lines, {0, -2}, {0, 2}));
    EXPECT_TRUE(markersIn(scene, kSourceLayer).empty());
    EXPECT_EQ(scene.layers.front().name, "Existing selection");
    EXPECT_EQ(overlay.selection_.model.items(), (std::vector<GeometryRef>{selectedRef}));
    EXPECT_EQ(sketch_->entityCount(), 3u);
    EXPECT_EQ(sketch_->constraintCount(), 0u);
    const auto horizontalStart = sketch_->pointPosition({horizontal.value(), SubElement::Start});
    const auto horizontalEnd = sketch_->pointPosition({horizontal.value(), SubElement::End});
    const auto verticalStart = sketch_->pointPosition({vertical.value(), SubElement::Start});
    const auto verticalEnd = sketch_->pointPosition({vertical.value(), SubElement::End});
    ASSERT_TRUE(horizontalStart);
    ASSERT_TRUE(horizontalEnd);
    ASSERT_TRUE(verticalStart);
    ASSERT_TRUE(verticalEnd);
    EXPECT_EQ(horizontalStart.value(), (Vec2{-2, 0}));
    EXPECT_EQ(horizontalEnd.value(), (Vec2{2, 0}));
    EXPECT_EQ(verticalStart.value(), (Vec2{0, -2}));
    EXPECT_EQ(verticalEnd.value(), (Vec2{0, 2}));
}

TEST_F(SnapFeedbackRendering, HighlightsOnlyTheReferencedEndpointWithoutHighlightingItsWholeCurve) {
    const auto line = sketch_->addLine({-2, 1}, {2, 1});
    ASSERT_TRUE(line);
    auto feedback = feedbackAt({-2, 1});
    feedback.glyph = app::SnapGlyph::Endpoint;
    feedback.sources = {{line.value(), SubElement::Start}};

    const auto scene = renderFeedback(feedback);

    const auto markers = markersIn(scene, kSourceLayer);
    ASSERT_EQ(markers.size(), 1u);
    EXPECT_FLOAT_EQ(markers.front().x, -2);
    EXPECT_FLOAT_EQ(markers.front().y, 1);
    EXPECT_TRUE(linesIn(scene, kSourceLayer).empty());
}

TEST_F(SnapFeedbackRendering, CircleCenterHighlightsItsSiteWithoutHighlightingTheCircumference) {
    const auto circle = sketch_->addCircle({1, 2}, 0.5);
    ASSERT_TRUE(circle);
    auto feedback = feedbackAt({1, 2});
    feedback.glyph = app::SnapGlyph::Center;
    feedback.sources = {{circle.value(), SubElement::Center}};

    const auto scene = renderFeedback(feedback);

    const auto markers = markersIn(scene, kSourceLayer);
    ASSERT_EQ(markers.size(), 1u);
    EXPECT_FLOAT_EQ(markers.front().x, 1);
    EXPECT_FLOAT_EQ(markers.front().y, 2);
    const auto* sources = findLayer(scene, kSourceLayer);
    ASSERT_NE(sources, nullptr);
    EXPECT_TRUE(sources->circleBatches.empty());
    EXPECT_TRUE(sources->arcBatches.empty());
}

TEST_F(SnapFeedbackRendering, IgnoresMissingAndInvalidReferencesWhileRetainingValidSources) {
    const auto line = sketch_->addLine({-1, 0}, {1, 0});
    const auto removed = sketch_->addPoint({0, 1});
    ASSERT_TRUE(line);
    ASSERT_TRUE(removed);
    ASSERT_TRUE(sketch_->removeEntity(removed.value()));
    auto feedback = feedbackAt();
    feedback.sources = {
        {EntityId(9999), SubElement::Whole}, {removed.value(), SubElement::Whole}, {line.value(), SubElement::Center}, {line.value(), SubElement::Whole}};

    const auto scene = renderFeedback(feedback);

    const auto lines = linesIn(scene, kSourceLayer);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_TRUE(hasLine(lines, {-1, 0}, {1, 0}));
    EXPECT_TRUE(markersIn(scene, kSourceLayer).empty());
    EXPECT_FALSE(linesIn(scene, kGlyphLayer).empty());
    EXPECT_EQ(sketch_->entityCount(), 1u);
    EXPECT_EQ(sketch_->constraintCount(), 0u);
}

TEST_F(SnapFeedbackRendering, GlyphSizeIsIndependentOfZoomAndUsesFramebufferCoordinates) {
    camera_.setCenter({1, -1});
    style_.glyphSizePx = 10;
    const auto feedback = feedbackAt({2, -0.5});
    for (const double ratio : {1.0, 2.0}) {
        for (const double zoom : {25.0, 200.0}) {
            SCOPED_TRACE(testing::Message() << "DPR=" << ratio << ", zoom=" << zoom);
            camera_.setViewport(800, 600, ratio);
            camera_.setZoom(zoom);
            const auto scene = renderFeedback(feedback);
            const auto* glyph = findLayer(scene, kGlyphLayer);
            ASSERT_NE(glyph, nullptr);
            EXPECT_EQ(glyph->coordinateSpace, render::CoordinateSpace::Screen);
            const auto lines = linesIn(scene, kGlyphLayer);
            ASSERT_FALSE(lines.empty());
            double minX = std::numeric_limits<double>::infinity();
            double maxX = -std::numeric_limits<double>::infinity();
            double minY = std::numeric_limits<double>::infinity();
            double maxY = -std::numeric_limits<double>::infinity();
            for (const auto& line : lines) {
                minX = std::min({minX, static_cast<double>(line.x1), static_cast<double>(line.x2)});
                maxX = std::max({maxX, static_cast<double>(line.x1), static_cast<double>(line.x2)});
                minY = std::min({minY, static_cast<double>(line.y1), static_cast<double>(line.y2)});
                maxY = std::max({maxY, static_cast<double>(line.y1), static_cast<double>(line.y2)});
            }
            EXPECT_NEAR((maxX - minX) / ratio, style_.glyphSizePx, kCoordinateTolerance);
            EXPECT_NEAR((maxY - minY) / ratio, style_.glyphSizePx, kCoordinateTolerance);
            const auto logical = camera_.worldToScreenLogical({feedback.position.x, feedback.position.y});
            EXPECT_NEAR((minX + maxX) * 0.5, logical.x * ratio, kCoordinateTolerance);
            EXPECT_NEAR((minY + maxY) * 0.5, (camera_.hLogical() - logical.y) * ratio, kCoordinateTolerance);
        }
    }
}

TEST_F(SnapFeedbackRendering, EveryGlyphCanRenderWithoutSourceGeometryOrGuides) {
    for (const auto glyph : {app::SnapGlyph::Point, app::SnapGlyph::Endpoint, app::SnapGlyph::Midpoint, app::SnapGlyph::Center, app::SnapGlyph::Intersection,
                             app::SnapGlyph::Direction}) {
        SCOPED_TRACE(static_cast<int>(glyph));
        auto feedback = feedbackAt({1, 1});
        feedback.glyph = glyph;

        const auto scene = renderFeedback(feedback);

        const auto lines = linesIn(scene, kGlyphLayer);
        ASSERT_FALSE(lines.empty());
        for (const auto& line : lines) {
            EXPECT_TRUE(std::isfinite(line.x1));
            EXPECT_TRUE(std::isfinite(line.y1));
            EXPECT_TRUE(std::isfinite(line.x2));
            EXPECT_TRUE(std::isfinite(line.y2));
        }
        EXPECT_TRUE(linesIn(scene, kSourceLayer).empty());
        EXPECT_TRUE(linesIn(scene, kGuideLayer).empty());
    }
}

TEST_F(SnapFeedbackRendering, ClipsFiniteGuidesWithoutExtendingTheirRemainingEnds) {
    auto feedback = feedbackAt();
    feedback.guides = {{{{-10, 1}, {1, 1}}, app::SnapGuideExtent::Segment},
                       {{{-1, 0}, {1, 0}}, app::SnapGuideExtent::Segment},
                       {{{-10, 5}, {10, 5}}, app::SnapGuideExtent::Segment}};

    const auto scene = renderFeedback(feedback);

    const auto* guides = findLayer(scene, kGuideLayer);
    ASSERT_NE(guides, nullptr);
    EXPECT_EQ(guides->coordinateSpace, render::CoordinateSpace::World);
    const auto lines = linesIn(scene, kGuideLayer);
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(hasLine(lines, {-4, 1}, {1, 1}));
    EXPECT_TRUE(hasLine(lines, {-1, 0}, {1, 0}));
}

TEST_F(SnapFeedbackRendering, ExtendsInfiniteGuidesToTheVisibleWorldBounds) {
    auto feedback = feedbackAt();
    feedback.guides = {{{{-1, -1}, {1, -1}}, app::SnapGuideExtent::InfiniteLine}, {{{2, -1}, {2, 1}}, app::SnapGuideExtent::InfiniteLine}};

    const auto scene = renderFeedback(feedback);

    const auto lines = linesIn(scene, kGuideLayer);
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_TRUE(hasLine(lines, {-4, -1}, {4, -1}));
    EXPECT_TRUE(hasLine(lines, {2, -3}, {2, 3}));
}

TEST_F(SnapFeedbackRendering, SkipsDegenerateAndNonfiniteGuides) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    auto feedback = feedbackAt();
    feedback.guides = {{{{1, 1}, {1, 1}}, app::SnapGuideExtent::Segment},
                       {{{2, 2}, {2, 2}}, app::SnapGuideExtent::InfiniteLine},
                       {{{nan, 0}, {1, 0}}, app::SnapGuideExtent::Segment},
                       {{{0, 0}, {infinity, 1}}, app::SnapGuideExtent::InfiniteLine},
                       {{{-1, 0}, {1, 0}}, app::SnapGuideExtent::Segment}};

    const auto scene = renderFeedback(feedback);

    const auto lines = linesIn(scene, kGuideLayer);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_TRUE(hasLine(lines, {-1, 0}, {1, 0}));
    EXPECT_FALSE(linesIn(scene, kGlyphLayer).empty());
}

TEST_F(SnapFeedbackRendering, NonfiniteFeedbackPositionProducesNoRenderPrimitives) {
    const auto line = sketch_->addLine({-1, 0}, {1, 0});
    ASSERT_TRUE(line);
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    for (const Vec2 position : std::array<Vec2, 3>{{{nan, 0}, {0, infinity}, {-infinity, 0}}}) {
        auto feedback = feedbackAt(position);
        feedback.sources = {{line.value(), SubElement::Whole}};
        feedback.guides = {{{{-1, 0}, {1, 0}}, app::SnapGuideExtent::Segment}};

        const auto scene = renderFeedback(feedback);

        EXPECT_TRUE(scene.layers.empty());
    }
}
}  // namespace
