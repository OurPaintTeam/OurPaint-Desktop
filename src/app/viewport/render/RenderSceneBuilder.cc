#include "RenderSceneBuilder.h"

#include <../core/tinynurbs/include/tinynurbs/tinynurbs.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <glm/glm.hpp>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

#include "../../../core/Scene.h"
#include "../constraints/ConstraintLayout.h"
#include "AxisTexts.h"
#include "OverlayModel.h"
#include "RenderScene.h"
#include "SnapFeedbackBuilder.h"
#include "ViewportStyle.h"
#include "objects/Objects.h"

using namespace core;

namespace {

constexpr int kSelectedOrder         = 10;

constexpr int kGeometryOrder         = 20;

constexpr int kSelectedMarkers       = 21;
constexpr int kGeometryMarkerOrder   = 22;

constexpr int kSpecialOrder          = 30;

constexpr int kOverlayOrder          = 40;
constexpr int kOverlayMarkerOrder    = 50;

constexpr int kScreenOverlayOrder    = 100;
constexpr int kTextOrder             = 1000;

constexpr int kConstraints           = 2000;

render::Color makeColor(float r, float g, float b, float a = 1.0f) {
    return render::Color{r, g, b, a};
}

render::Color makeColor(const glm::vec3& c, float a = 1.0f) {
    return render::Color{c.r, c.g, c.b, a};
}

render::Marker makeMarker(double x, double y) {
    return render::Marker{
        static_cast<float>(x),
        static_cast<float>(y)
    };
}

render::Line makeLine(double x1, double y1, double x2, double y2) {
    return render::Line{
        static_cast<float>(x1),
        static_cast<float>(y1),
        static_cast<float>(x2),
        static_cast<float>(y2)
    };
}

render::Circle makeCircle(double x, double y, double r) {
    return render::Circle{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(r)
    };
}

render::Arc makeArc(double x,
                    double y,
                    double r,
                    double startAngle,
                    double endAngle) {
    return render::Arc{
        static_cast<float>(x),
        static_cast<float>(y),
        static_cast<float>(r),
        static_cast<float>(startAngle),
        static_cast<float>(endAngle)
    };
}

render::Rect makeRect(double xMin, double yMin, double xMax, double yMax) {
    return render::Rect{
        static_cast<float>(xMin),
        static_cast<float>(yMin),
        static_cast<float>(xMax),
        static_cast<float>(yMax)
    };
}

render::Arc makeArc(const sketch::Arc2& arc) {
    return makeArc(arc.center.x, arc.center.y, std::hypot(arc.start.x - arc.center.x, arc.start.y - arc.center.y),
                   std::atan2(arc.start.y - arc.center.y, arc.start.x - arc.center.x), std::atan2(arc.end.y - arc.center.y, arc.end.x - arc.center.x));
}

render::DrawLayer makeWorldLayer(std::string name, int order) {
    render::DrawLayer layer;
    layer.name = std::move(name);
    layer.order = order;
    layer.coordinateSpace = render::CoordinateSpace::World;
    return layer;
}

render::DrawLayer makeScreenLayer(std::string name, int order) {
    render::DrawLayer layer;
    layer.name = std::move(name);
    layer.order = order;
    layer.coordinateSpace = render::CoordinateSpace::Screen;
    return layer;
}

void pushLayerIfNotEmpty(render::RenderScene& scene, render::DrawLayer&& layer) {
    if (!layer.empty()) {
        scene.layers.push_back(std::move(layer));
    }
}

} // namespace

RenderSceneBuilder::RenderSceneBuilder(Document& document,
                                       const OverlayModel& overlay,
                                       AxisTexts& axis,
                                       const app::ViewportStyle& style,
                                       render::RenderScene& renderScene,
                                       const Camera2D& camera,
                                       const app::ConstraintLayout& constraintLayout)
    :
      document_(document),
      sketch_(document.sketch()),
      overlay_(overlay),
      axis_(axis),
      style_(style),
      renderScene_(renderScene),
      camera_(camera),
      constraintLayout_(constraintLayout) {}

void RenderSceneBuilder::rebuild() {
    renderScene_.clear();

    buildGrid();

    buildBaseObjects();
    buildBezierCurves();
    buildBaseMarkers();

    buildBezierHandles();

    buildOverlayObjects();
    buildOverlayMarkers();

    buildSelectedObjects();
    buildSelectedMarkers();

    buildSelectionRectangle();
    buildAxisText();

    buildConstraintsMarkers();
    buildSnapFeedback();

    testBuild();
}

void RenderSceneBuilder::buildGrid() {
    axis_.update();

    const auto& gridInfo = axis_.getGridInfo();

    render::Grid grid;
    grid.cellSize = static_cast<float>(gridInfo.cellSize);
    grid.subCellSize = static_cast<float>(gridInfo.subCellSize);

    // Current OpenGL grid shader uses one grid color and one axis color.
    // majorColor is kept equal to minorColor until the shader supports both.
    grid.minorColor = makeColor(gridInfo.gridColor);
    grid.majorColor = makeColor(gridInfo.gridColor);
    grid.axisColor = makeColor(gridInfo.axisColor);

    renderScene_.grid = grid;
}

void RenderSceneBuilder::buildBaseObjects() {
    render::DrawLayer layer = makeWorldLayer("geometry", kGeometryOrder);

    render::LineBatch lines;
    lines.style = style_.baseLine;

    render::CircleBatch circles;
    circles.stroke = style_.baseCircle;

    render::ArcBatch arcs;
    arcs.stroke = style_.baseCircle;
    auto result = sketch_.entities();
    if (!result) {
        return;
    }

    for (const auto& entity : result.value()) {
        if (const auto* line = std::get_if<sketch::Line2>(&entity.geometry)) {
            lines.lines.push_back(makeLine(line->start.x, line->start.y, line->end.x, line->end.y));
        } else if (const auto* circle = std::get_if<sketch::Circle2>(&entity.geometry)) {
            circles.circles.push_back(makeCircle(circle->center.x, circle->center.y, circle->radius));
        } else if (const auto* arc = std::get_if<sketch::Arc2>(&entity.geometry)) {
            arcs.arcs.push_back(makeArc(*arc));
        }
    }

    if (!lines.lines.empty()) {
        layer.lineBatches.push_back(std::move(lines));
    }

    if (!circles.circles.empty()) {
        layer.circleBatches.push_back(std::move(circles));
    }

    if (!arcs.arcs.empty()) {
        layer.arcBatches.push_back(std::move(arcs));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildBaseMarkers() {
    render::DrawLayer layer = makeWorldLayer("geometry_markers", kGeometryMarkerOrder);

    render::MarkerBatch markers;
    markers.style = style_.baseMarker;

    auto result = sketch_.pointElements();
    if (!result) {
        return;
    }
    for (const auto& point : result.value()) {
        markers.markers.push_back(makeMarker(point.position.x, point.position.y));
    }

    if (!markers.markers.empty()) {
        layer.markerBatches.push_back(std::move(markers));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildBezierCurves() {
    // render::DrawLayer layer = makeWorldLayer("bezier_curves", kGeometryOrder);
    //
    // render::LineBatch curves;
    // curves.style = style_.baseLine;
    //
    // for (const ObjectData& bezier : scene_.getBeziers()) {
    //     appendBezierPolyline(bezier, curves.lines);
    // }
    //
    // if (!curves.lines.empty()) {
    //     layer.lineBatches.push_back(std::move(curves));
    // }
    //
    // pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildBezierHandles() {
    // render::DrawLayer layer = makeWorldLayer("bezier_handles", kSpecialOrder);
    //
    // render::LineBatch handles;
    // handles.style = style_.specialLine;
    //
    // for (const ObjectData& od : scene_.getBeziers()) {
    //     const double p0x = od.params[0];
    //     const double p0y = od.params[1];
    //
    //     const double p3x = od.params[2];
    //     const double p3y = od.params[3];
    //
    //     const double p1x = od.params[4];
    //     const double p1y = od.params[5];
    //
    //     const double p2x = od.params[6];
    //     const double p2y = od.params[7];
    //
    //     handles.lines.push_back(makeLine(p0x, p0y, p1x, p1y));
    //     handles.lines.push_back(makeLine(p3x, p3y, p2x, p2y));
    // }
    //
    // if (!handles.lines.empty()) {
    //     layer.lineBatches.push_back(std::move(handles));
    // }
    //
    // pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildOverlayObjects() {
    render::DrawLayer layer = makeWorldLayer("overlay", kOverlayOrder);

    render::LineBatch lines;
    lines.style = style_.overlayLine;

    render::CircleBatch circles;
    circles.stroke = style_.overlayCircle;

    render::ArcBatch arcs;
    arcs.stroke = style_.overlayCircle;

    for (const auto& line : overlay_.lines_) {
        lines.lines.push_back(makeLine(
            line.x1,
            line.y1,
            line.x2,
            line.y2
        ));
    }

    for (const auto& circle : overlay_.circles_) {
        circles.circles.push_back(makeCircle(
            circle.cx,
            circle.cy,
            circle.r
        ));
    }

    for (const auto& arc : overlay_.arcs_) {
        arcs.arcs.push_back(makeArc(
            arc.cx,
            arc.cy,
            arc.r,
            arc.startAngle,
            arc.endAngle
        ));
    }

    if (!lines.lines.empty()) {
        layer.lineBatches.push_back(std::move(lines));
    }

    if (!circles.circles.empty()) {
        layer.circleBatches.push_back(std::move(circles));
    }

    if (!arcs.arcs.empty()) {
        layer.arcBatches.push_back(std::move(arcs));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildOverlayMarkers() {
    render::DrawLayer layer = makeWorldLayer("overlay_markers", kOverlayMarkerOrder);

    render::MarkerBatch markers;
    markers.style = style_.overlayMarker;

    for (const auto& point : overlay_.points_) {
        markers.markers.push_back(makeMarker(point.x, point.y));
    }

    if (!markers.markers.empty()) {
        layer.markerBatches.push_back(std::move(markers));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildSelectedObjects() {
    render::DrawLayer layer = makeWorldLayer("selected", kSelectedOrder);

    render::LineBatch lines;
    lines.style = style_.selectedLine;

    render::CircleBatch circles;
    circles.stroke = style_.selectedCircle;

    render::ArcBatch arcs;
    arcs.stroke = style_.selectedCircle;

    auto refs = overlay_.selection_.model.items();
    for (const auto& ref : overlay_.constraintRefs_) {
        if (!overlay_.selection_.model.contains(ref)) {
            refs.push_back(ref);
        }
    }
    for (const auto& ref : refs) {
        if (ref.sub != sketch::SubElement::Whole) {
            continue;
        }
        auto result = sketch_.entity(ref.entity);
        if (!result) {
            continue;
        }

        const auto& entity = result.value();

        if (const auto* line = std::get_if<sketch::Line2>(&entity.geometry)) {
            lines.lines.push_back(makeLine(line->start.x, line->start.y, line->end.x, line->end.y));
        } else if (const auto* circle = std::get_if<sketch::Circle2>(&entity.geometry)) {
            circles.circles.push_back(makeCircle(circle->center.x, circle->center.y, circle->radius));
        } else if (const auto* arc = std::get_if<sketch::Arc2>(&entity.geometry)) {
            arcs.arcs.push_back(makeArc(*arc));
        }
    }

    if (!lines.lines.empty()) {
        layer.lineBatches.push_back(std::move(lines));
    }

    if (!circles.circles.empty()) {
        layer.circleBatches.push_back(std::move(circles));
    }

    if (!arcs.arcs.empty()) {
        layer.arcBatches.push_back(std::move(arcs));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildSelectedMarkers() {
    render::DrawLayer layer = makeWorldLayer("selected_markers", kSelectedMarkers);

    render::MarkerBatch markers;
    markers.style = style_.selectedMarker;

    auto result = sketch_.pointElements();
    if (!result) {
        return;
    }
    for (const auto& point : result.value()) {
        if (overlay_.selection_.model.contains(point.ref) ||
            std::find(overlay_.constraintRefs_.begin(), overlay_.constraintRefs_.end(), point.ref) != overlay_.constraintRefs_.end()) {
            markers.markers.push_back(makeMarker(point.position.x, point.position.y));
        }
    }

    if (!markers.markers.empty()) {
        layer.markerBatches.push_back(std::move(markers));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildSelectionRectangle() {
    if (!overlay_.selectionRect_.has_value()) {
        return;
    }

    // Old renderer used camera MVP for selectionRect, so this keeps it as
    // world-space. If selectionRect_ is actually stored in screen pixels,
    // change this to makeScreenLayer(...).
    render::DrawLayer layer = makeWorldLayer("selection_rect", kScreenOverlayOrder);

    render::RectBatch rect;
    rect.stroke = style_.selectionRectStroke;
    rect.fill = style_.selectionRectFill;
    rect.rects.push_back(makeRect(
        overlay_.selectionRect_->xMin,
        overlay_.selectionRect_->yMin,
        overlay_.selectionRect_->xMax,
        overlay_.selectionRect_->yMax
    ));

    layer.rectBatches.push_back(std::move(rect));

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildAxisText() {
    render::DrawLayer layer = makeScreenLayer("axis_text", kTextOrder);

    render::TextBatch text;
    text.textObjects = axis_.getLabels();

    if (!text.textObjects.empty()) {
        layer.textBatches.push_back(std::move(text));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildConstraintsMarkers() {
    render::DrawLayer layer = makeScreenLayer("constraints", kConstraints);

    struct MarkerBatches {
        render::LineBatch lines;
        render::CircleBatch circles;
        render::ArcBatch arcs;
        render::TextBatch labels;
        std::vector<render::MarkerBatch> dots;
    };
    // Batch by interaction state; hover and drag use the same primitives as picking.
    std::array<MarkerBatches, 3> batches;
    const std::array colors{style_.constraintMarker.stroke.color, style_.constraintMarker.hoveredColor, style_.constraintMarker.draggedColor};
    const double dpr = camera_.viewport().devicePixelRatio;
    const double height = camera_.hFramebuffer();
    for (size_t i = 0; i < batches.size(); ++i) {
        auto& stroke = batches[i].lines.style;
        stroke = style_.constraintMarker.stroke;
        stroke.color = colors[i];
        stroke.widthPx *= static_cast<float>(dpr);
        if (stroke.edgeSoftnessPx) {
            *stroke.edgeSoftnessPx *= static_cast<float>(dpr);
        }
        batches[i].circles.stroke = stroke;
        batches[i].arcs.stroke = stroke;
    }

    // Layout uses logical pixels with a top-left origin; Screen layers use framebuffer pixels with a bottom-left origin.
    for (const auto& marker : constraintLayout_.markers()) {
        const auto represents = [&](const auto& id) {
            return id && std::find(marker.constraints.begin(), marker.constraints.end(), *id) != marker.constraints.end();
        };
        const size_t state = represents(overlay_.draggedDimension) ? 2 : represents(overlay_.hoveredDimension) ? 1 : 0;
        auto& group = batches[state];
        auto& linesBatch = group.lines;
        auto& circlesBatch = group.circles;
        auto& arcsBatch = group.arcs;
        auto& labelsBatch = group.labels;
        for (const auto& stroke : marker.strokes) {
            linesBatch.lines.push_back(makeLine(stroke.start.x * dpr, height - stroke.start.y * dpr, stroke.end.x * dpr, height - stroke.end.y * dpr));
        }
        for (const auto& dot : marker.dots) {
            const float radius = static_cast<float>(dot.radiusPx * dpr);
            auto batch =
                std::find_if(group.dots.begin(), group.dots.end(), [radius](const render::MarkerBatch& candidate) { return candidate.style.radius == radius; });
            if (batch == group.dots.end()) {
                render::MarkerBatch dotsBatch;
                dotsBatch.style = style_.constraintMarker.point;
                if (state != 0) {
                    dotsBatch.style.color = colors[state];
                }
                dotsBatch.style.radius = radius;
                if (dotsBatch.style.edgeSoftnessPx) {
                    *dotsBatch.style.edgeSoftnessPx *= static_cast<float>(dpr);
                }
                group.dots.push_back(std::move(dotsBatch));
                batch = std::prev(group.dots.end());
            }
            batch->markers.push_back(makeMarker(dot.center.x * dpr, height - dot.center.y * dpr));
        }
        for (const auto& circle : marker.circles) {
            circlesBatch.circles.push_back(makeCircle(circle.center.x * dpr, height - circle.center.y * dpr, circle.radiusPx * dpr));
        }
        for (const auto& arc : marker.arcs) {
            // Layout angles already use the renderer's right/up basis; only the center's Y is flipped.
            arcsBatch.arcs.push_back(makeArc(arc.center.x * dpr, height - arc.center.y * dpr, arc.radiusPx * dpr, arc.startAngle, arc.endAngle));
        }
        for (const auto& label : marker.labels) {
            render::text::TextObject text;
            text.utf8Text = label.text;
            text.style.height = label.heightPx * dpr;
            text.style.hAlign = render::text::TextHorizontalAlign::Center;
            text.style.vAlign = render::text::TextVerticalAlign::Middle;
            const auto& color = linesBatch.style.color;
            text.style.r = color.r;
            text.style.g = color.g;
            text.style.b = color.b;
            text.style.a = color.a;
            text.placement.space = render::text::TextSpace::Screen;
            text.placement.screen.anchorPx = {label.center.x * dpr, height - label.center.y * dpr};
            text.placement.screen.rotationRad = -label.rotationRad;
            labelsBatch.textObjects.push_back(std::move(text));
        }
    }

    for (auto& group : batches) {
        if (!group.lines.lines.empty()) {
            layer.lineBatches.push_back(std::move(group.lines));
        }
        if (!group.circles.circles.empty()) {
            layer.circleBatches.push_back(std::move(group.circles));
        }
        if (!group.arcs.arcs.empty()) {
            layer.arcBatches.push_back(std::move(group.arcs));
        }
        if (!group.labels.textObjects.empty()) {
            layer.textBatches.push_back(std::move(group.labels));
        }
        for (auto& dots : group.dots) {
            layer.markerBatches.push_back(std::move(dots));
        }
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildSnapFeedback() {
    if (const auto& feedback = overlay_.snapFeedback()) {
        app::appendSnapFeedback(*feedback, sketch_, camera_, style_.snapFeedback, renderScene_);
    }
}

void RenderSceneBuilder::testBuild() {
    auto entities = sketch_.entities(sketch::EntityKind::Nurbs);
    if (!entities) {
        std::cerr << "Cannot read NURBS curves: " << entities.error().message << '\n';
        return;
    }

    render::DrawLayer layer = makeWorldLayer("nurbs_curves", kGeometryOrder);
    render::LineBatch curves;
    render::MarkerBatch controlMarkers;
    curves.style = style_.baseLine;
    controlMarkers.style = style_.baseMarker;

    for (const auto& ent : entities.value()) {
        const auto& curve = std::get<sketch::NurbsCurve2>(ent.geometry);
        tinynurbs::RationalCurve<double> crv;
        crv.degree = static_cast<unsigned>(curve.degree);
        crv.knots = curve.knots;
        crv.weights = curve.weights;
        for (const auto& p : curve.controlPoints) {
            crv.control_points.push_back({p.position.x, p.position.y, 0.0});
            controlMarkers.markers.push_back(render::Marker(p.position.x, p.position.y));
        }
        if (!tinynurbs::curveIsValid(crv)) {
            std::cerr << "Invalid NURBS curve: " << ent.id.get() << '\n';
            continue;
        }

        constexpr size_t numSegments = 256;
        const double uMin = crv.knots[crv.degree];
        const double uMax = crv.knots[crv.control_points.size()];
        auto previous = tinynurbs::curvePoint(crv, uMin);
        for (size_t i = 1; i <= numSegments; ++i) {
            const double t = std::lerp(uMin, uMax, static_cast<double>(i) / static_cast<double>(numSegments));
            const auto current = tinynurbs::curvePoint(crv, t);
            curves.lines.push_back(makeLine(previous.x, previous.y, current.x, current.y));
            previous = current;
        }
    }

    if (!curves.lines.empty()) {
        layer.lineBatches.push_back(std::move(curves));
    }

    if (!controlMarkers.markers.empty()) {
        layer.markerBatches.push_back(std::move(controlMarkers));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}
