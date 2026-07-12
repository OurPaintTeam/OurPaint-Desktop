#include "RenderSceneBuilder.h"

#include "ViewportStyle.h"

#include <utility>
#include <vector>

#include "AxisTexts.h"
#include "OverlayModel.h"
#include "RenderScene.h"
#include "Scene.h"
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

double cubicBezier(double p0, double p1, double p2, double p3, double t) {
    const double u = 1.0 - t;

    return u * u * u * p0
         + 3.0 * u * u * t * p1
         + 3.0 * u * t * t * p2
         + t * t * t * p3;
}

void appendBezierPolyline(const ObjectData& od, std::vector<render::Line>& outLines) {
    const double p0x = od.params[0];
    const double p0y = od.params[1];

    const double p3x = od.params[2];
    const double p3y = od.params[3];

    const double p1x = od.params[4];
    const double p1y = od.params[5];

    const double p2x = od.params[6];
    const double p2y = od.params[7];

    constexpr int kSegments = 100;

    double previousX = cubicBezier(p0x, p1x, p2x, p3x, 0.0);
    double previousY = cubicBezier(p0y, p1y, p2y, p3y, 0.0);

    for (int i = 1; i <= kSegments; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(kSegments);

        const double x = cubicBezier(p0x, p1x, p2x, p3x, t);
        const double y = cubicBezier(p0y, p1y, p2y, p3y, t);

        outLines.push_back(makeLine(previousX, previousY, x, y));

        previousX = x;
        previousY = y;
    }
}

} // namespace

RenderSceneBuilder::RenderSceneBuilder(const Scene& scene,
                                       const OverlayModel& overlay,
                                       AxisTexts& axis,
                                       const app::ViewportStyle& style,
                                       render::RenderScene& renderScene)
    : scene_(scene),
      overlay_(overlay),
      axis_(axis),
      style_(style),
      renderScene_(renderScene) {}

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

    for (const ObjectData& l : scene_.getLines()) {
        lines.lines.push_back(makeLine(
            l.params[0],
            l.params[1],
            l.params[2],
            l.params[3]
        ));
    }

    for (const ObjectData& c : scene_.getCircles()) {
        circles.circles.push_back(makeCircle(
            c.params[0],
            c.params[1],
            c.params[2]
        ));
    }

    if (!lines.lines.empty()) {
        layer.lineBatches.push_back(std::move(lines));
    }

    if (!circles.circles.empty()) {
        layer.circleBatches.push_back(std::move(circles));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildBaseMarkers() {
    render::DrawLayer layer = makeWorldLayer("geometry_markers", kGeometryMarkerOrder);

    render::MarkerBatch markers;
    markers.style = style_.baseMarker;

    for (const ObjectData& p : scene_.getPoints()) {
        markers.markers.push_back(makeMarker(p.params[0], p.params[1]));
    }

    for (const ObjectData& c : scene_.getCircles()) {
        // Keep old behavior: circle center is also rendered as a marker.
        markers.markers.push_back(makeMarker(c.params[0], c.params[1]));
    }

    if (!markers.markers.empty()) {
        layer.markerBatches.push_back(std::move(markers));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildBezierCurves() {
    render::DrawLayer layer = makeWorldLayer("bezier_curves", kGeometryOrder);

    render::LineBatch curves;
    curves.style = style_.baseLine;

    for (const ObjectData& bezier : scene_.getBeziers()) {
        appendBezierPolyline(bezier, curves.lines);
    }

    if (!curves.lines.empty()) {
        layer.lineBatches.push_back(std::move(curves));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildBezierHandles() {
    render::DrawLayer layer = makeWorldLayer("bezier_handles", kSpecialOrder);

    render::LineBatch handles;
    handles.style = style_.specialLine;

    for (const ObjectData& od : scene_.getBeziers()) {
        const double p0x = od.params[0];
        const double p0y = od.params[1];

        const double p3x = od.params[2];
        const double p3y = od.params[3];

        const double p1x = od.params[4];
        const double p1y = od.params[5];

        const double p2x = od.params[6];
        const double p2y = od.params[7];

        handles.lines.push_back(makeLine(p0x, p0y, p1x, p1y));
        handles.lines.push_back(makeLine(p3x, p3y, p2x, p2y));
    }

    if (!handles.lines.empty()) {
        layer.lineBatches.push_back(std::move(handles));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
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

    for (const auto& id : overlay_.selection_.model.items()) {
        const ObjectData od = scene_.getObjectData(id);

        if (od.et == ObjType::ET_LINE) {
            lines.lines.push_back(makeLine(
                od.params[0],
                od.params[1],
                od.params[2],
                od.params[3]
            ));
        } else if (od.et == ObjType::ET_CIRCLE) {
            circles.circles.push_back(makeCircle(
                od.params[0],
                od.params[1],
                od.params[2]
            ));
        }
    }

    if (!lines.lines.empty()) {
        layer.lineBatches.push_back(std::move(lines));
    }

    if (!circles.circles.empty()) {
        layer.circleBatches.push_back(std::move(circles));
    }

    pushLayerIfNotEmpty(renderScene_, std::move(layer));
}

void RenderSceneBuilder::buildSelectedMarkers() {
    render::DrawLayer layer = makeWorldLayer("selected_markers", kSelectedMarkers);

    render::MarkerBatch markers;
    markers.style = style_.selectedMarker;

    for (const auto& id : overlay_.selection_.model.items()) {
        const ObjectData od = scene_.getObjectData(id);

        if (od.et == ObjType::ET_POINT) {
            markers.markers.push_back(makeMarker(od.params[0], od.params[1]));
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



