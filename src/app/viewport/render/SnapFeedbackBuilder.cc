#include "SnapFeedbackBuilder.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <utility>
#include <vector>

#include "../SnapFeedback.h"
#include "../SnapFeedbackStyle.h"
#include "Camera2D.h"
#include "RenderScene.h"
#include "sketch/Sketch.h"

namespace app {
namespace {

using core::sketch::GeometryRef;
using core::sketch::Line2;
using core::sketch::SubElement;
using core::sketch::Vec2;

bool finite(Vec2 point) { return std::isfinite(point.x) && std::isfinite(point.y); }

bool renderable(double value) { return std::isfinite(value) && std::abs(value) <= std::numeric_limits<float>::max(); }

bool renderable(Vec2 point) { return renderable(point.x) && renderable(point.y); }

bool validColor(const render::Color& color) { return std::isfinite(color.r) && std::isfinite(color.g) && std::isfinite(color.b) && std::isfinite(color.a); }

bool validStroke(const render::StrokeStyle& style) {
    return validColor(style.color) && std::isfinite(style.widthPx) && style.widthPx >= 0.0f &&
           (!style.edgeSoftnessPx || (std::isfinite(*style.edgeSoftnessPx) && *style.edgeSoftnessPx >= 0.0f));
}

bool validMarker(const render::MarkerStyle& style) {
    return validColor(style.color) && std::isfinite(style.radius) && style.radius > 0.0f &&
           (!style.edgeSoftnessPx || (std::isfinite(*style.edgeSoftnessPx) && *style.edgeSoftnessPx >= 0.0f));
}

render::DrawLayer makeLayer(const char* name, int order, render::CoordinateSpace coordinateSpace) {
    render::DrawLayer layer;
    layer.name = name;
    layer.order = order;
    layer.coordinateSpace = coordinateSpace;
    return layer;
}

void appendLayer(render::DrawLayer layer, render::RenderScene& scene) {
    if (!layer.empty()) {
        scene.layers.push_back(std::move(layer));
    }
}

bool appendLine(Vec2 start, Vec2 end, render::LineBatch& batch) {
    if (!renderable(start) || !renderable(end)) {
        return false;
    }
    const render::Line line{static_cast<float>(start.x), static_cast<float>(start.y), static_cast<float>(end.x), static_cast<float>(end.y)};
    const double length = std::hypot(static_cast<double>(line.x2) - line.x1, static_cast<double>(line.y2) - line.y1);
    // The line shader calculates its length in float, including squared components.
    if (length == 0.0 || length > std::sqrt(std::numeric_limits<float>::max())) {
        return false;
    }
    batch.lines.push_back(line);
    return true;
}

bool renderableCircle(Vec2 center, double radius) {
    return renderable(center) && std::isfinite(radius) && radius > 0.0 && radius <= std::numeric_limits<float>::max() && static_cast<float>(radius) > 0.0f &&
           std::abs(center.x) <= std::numeric_limits<float>::max() - radius && std::abs(center.y) <= std::numeric_limits<float>::max() - radius;
}

void appendSources(const SnapFeedback& feedback, const core::sketch::Sketch& sketch, const SnapFeedbackStyle& style, render::RenderScene& scene) {
    auto layer = makeLayer("snap_sources", 2100, render::CoordinateSpace::World);
    render::MarkerBatch points;
    points.style = style.referencePoint;
    render::LineBatch lines;
    lines.style = style.referenceCurve;
    render::CircleBatch circles;
    circles.stroke = style.referenceCurve;
    render::ArcBatch arcs;
    arcs.stroke = style.referenceCurve;
    std::vector<GeometryRef> visited;
    for (const auto ref : feedback.sources) {
        if (std::find(visited.begin(), visited.end(), ref) != visited.end()) {
            continue;
        }
        visited.push_back(ref);
        const auto entity = sketch.entity(ref.entity);
        if (!entity) {
            continue;
        }
        const auto& geometry = entity.value().geometry;
        if (ref.sub != SubElement::Whole || std::holds_alternative<core::sketch::Point2>(geometry)) {
            const auto position = sketch.pointPosition(ref);
            if (position && renderable(position.value()) && validMarker(points.style)) {
                points.markers.push_back({static_cast<float>(position.value().x), static_cast<float>(position.value().y)});
            }
            continue;
        }
        if (!validStroke(style.referenceCurve)) {
            continue;
        }
        if (const auto* line = std::get_if<Line2>(&geometry)) {
            appendLine(line->start, line->end, lines);
        } else if (const auto* circle = std::get_if<core::sketch::Circle2>(&geometry)) {
            if (renderableCircle(circle->center, circle->radius)) {
                circles.circles.push_back({static_cast<float>(circle->center.x), static_cast<float>(circle->center.y), static_cast<float>(circle->radius)});
            }
        } else if (const auto* arc = std::get_if<core::sketch::Arc2>(&geometry)) {
            const double radius = std::hypot(arc->start.x - arc->center.x, arc->start.y - arc->center.y);
            if (finite(arc->start) && finite(arc->end) && arc->start != arc->end && arc->end != arc->center && renderableCircle(arc->center, radius)) {
                // Match base geometry rendering even when an unsolved arc has unequal radii.
                arcs.arcs.push_back({static_cast<float>(arc->center.x), static_cast<float>(arc->center.y), static_cast<float>(radius),
                                     static_cast<float>(std::atan2(arc->start.y - arc->center.y, arc->start.x - arc->center.x)),
                                     static_cast<float>(std::atan2(arc->end.y - arc->center.y, arc->end.x - arc->center.x))});
            }
        }
    }
    if (!points.markers.empty()) {
        layer.markerBatches.push_back(std::move(points));
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
    appendLayer(std::move(layer), scene);
}

bool clipAxis(double origin, double direction, double minimum, double maximum, double& first, double& last) {
    if (direction == 0.0) {
        return origin >= minimum && origin <= maximum;
    }
    double enter = (minimum - origin) / direction;
    double leave = (maximum - origin) / direction;
    if (enter > leave) {
        std::swap(enter, leave);
    }
    first = std::max(first, enter);
    last = std::min(last, leave);
    return first <= last;
}

bool clipGuide(const SnapGuide& guide, const RectD& bounds, Line2& clipped) {
    if (!finite(guide.line.start) || !finite(guide.line.end)) {
        return false;
    }
    Vec2 direction{guide.line.end.x - guide.line.start.x, guide.line.end.y - guide.line.start.y};
    const double scale = std::max(std::abs(direction.x), std::abs(direction.y));
    if (!finite(direction) || scale == 0.0) {
        return false;
    }
    direction.x /= scale;
    direction.y /= scale;
    double first;
    double last;
    switch (guide.extent) {
        case SnapGuideExtent::Segment:
            first = 0.0;
            last = scale;
            break;
        case SnapGuideExtent::InfiniteLine:
            first = -std::numeric_limits<double>::infinity();
            last = std::numeric_limits<double>::infinity();
            break;
        default:
            return false;
    }
    if (!clipAxis(guide.line.start.x, direction.x, bounds.min.x, bounds.max.x, first, last) ||
        !clipAxis(guide.line.start.y, direction.y, bounds.min.y, bounds.max.y, first, last) || !std::isfinite(first) || !std::isfinite(last)) {
        return false;
    }
    clipped = {{std::fma(direction.x, first, guide.line.start.x), std::fma(direction.y, first, guide.line.start.y)},
               {std::fma(direction.x, last, guide.line.start.x), std::fma(direction.y, last, guide.line.start.y)}};
    return finite(clipped.start) && finite(clipped.end);
}

void appendGuides(const SnapFeedback& feedback, const RectD& bounds, const SnapFeedbackStyle& style, render::RenderScene& scene) {
    if (!validStroke(style.guide)) {
        return;
    }
    auto layer = makeLayer("snap_guides", 2110, render::CoordinateSpace::World);
    render::LineBatch lines;
    lines.style = style.guide;
    for (const auto& guide : feedback.guides) {
        Line2 clipped;
        if (clipGuide(guide, bounds, clipped)) {
            appendLine(clipped.start, clipped.end, lines);
        }
    }
    if (!lines.lines.empty()) {
        layer.lineBatches.push_back(std::move(lines));
    }
    appendLayer(std::move(layer), scene);
}

void appendGlyph(const SnapFeedback& feedback, const Camera2D& camera, const SnapFeedbackStyle& style, render::RenderScene& scene) {
    const double dpr = camera.viewport().devicePixelRatio;
    const auto logical = camera.worldToScreenLogical({feedback.position.x, feedback.position.y});
    const Vec2 center{logical.x * dpr, camera.hFramebuffer() - logical.y * dpr};
    const double halfSize = style.glyphSizePx * dpr * 0.5;
    if (!renderable(center) || !std::isfinite(halfSize) || halfSize <= 0.0 || !validStroke(style.marker)) {
        return;
    }
    render::LineBatch lines;
    lines.style = style.marker;
    const double width = lines.style.widthPx * dpr;
    if (!renderable(width) || (lines.style.edgeSoftnessPx && !renderable(*lines.style.edgeSoftnessPx * dpr))) {
        return;
    }
    lines.style.widthPx = static_cast<float>(width);
    if (lines.style.edgeSoftnessPx) {
        *lines.style.edgeSoftnessPx = static_cast<float>(*lines.style.edgeSoftnessPx * dpr);
    }
    const auto stroke = [&](double x1, double y1, double x2, double y2) {
        appendLine({center.x + x1 * halfSize, center.y + y1 * halfSize}, {center.x + x2 * halfSize, center.y + y2 * halfSize}, lines);
    };
    switch (feedback.glyph) {
        case SnapGlyph::Point:
            stroke(-1, 0, 1, 0);
            stroke(0, -1, 0, 1);
            break;
        case SnapGlyph::Endpoint:
            stroke(-1, -1, 1, -1);
            stroke(1, -1, 1, 1);
            stroke(1, 1, -1, 1);
            stroke(-1, 1, -1, -1);
            break;
        case SnapGlyph::Midpoint:
            stroke(-1, -1, 1, -1);
            stroke(1, -1, 0, 1);
            stroke(0, 1, -1, -1);
            break;
        case SnapGlyph::Center:
            for (int i = 0; i < 16; ++i) {
                const double start = 2.0 * std::numbers::pi * i / 16.0;
                const double end = 2.0 * std::numbers::pi * (i + 1) / 16.0;
                stroke(std::cos(start), std::sin(start), std::cos(end), std::sin(end));
            }
            break;
        case SnapGlyph::Intersection:
            stroke(-1, -1, 1, 1);
            stroke(-1, 1, 1, -1);
            break;
        case SnapGlyph::Direction:
            stroke(-1, 0, 0, 1);
            stroke(0, 1, 1, 0);
            stroke(1, 0, 0, -1);
            stroke(0, -1, -1, 0);
            break;
        default:
            return;
    }
    if (!lines.lines.empty()) {
        auto layer = makeLayer("snap_glyph", 2120, render::CoordinateSpace::Screen);
        layer.lineBatches.push_back(std::move(lines));
        appendLayer(std::move(layer), scene);
    }
}

}  // namespace

void appendSnapFeedback(const SnapFeedback& feedback, const core::sketch::Sketch& sketch, const Camera2D& camera, const SnapFeedbackStyle& style,
                        render::RenderScene& scene) {
    const auto& viewport = camera.viewport();
    const double framebufferWidth = viewport.logicalWidth * viewport.devicePixelRatio;
    const double framebufferHeight = viewport.logicalHeight * viewport.devicePixelRatio;
    if (!finite(feedback.position) || !std::isfinite(camera.zoom()) || camera.zoom() <= 0.0 || viewport.logicalWidth <= 0 || viewport.logicalHeight <= 0 ||
        !std::isfinite(viewport.devicePixelRatio) || viewport.devicePixelRatio <= 0.0 || !std::isfinite(framebufferWidth) ||
        !std::isfinite(framebufferHeight) || framebufferWidth < 0.5 || framebufferHeight < 0.5 || framebufferWidth > std::numeric_limits<int>::max() - 0.5 ||
        framebufferHeight > std::numeric_limits<int>::max() - 0.5 || !renderable(camera.centerWorld().x) || !renderable(camera.centerWorld().y) ||
        !renderable(camera.centerWorld().x * camera.zoom()) || !renderable(camera.centerWorld().y * camera.zoom())) {
        return;
    }
    const auto bounds = camera.visibleWorldRect();
    if (!finite({bounds.min.x, bounds.min.y}) || !finite({bounds.max.x, bounds.max.y}) || bounds.min.x >= bounds.max.x || bounds.min.y >= bounds.max.y) {
        return;
    }
    appendSources(feedback, sketch, style, scene);
    appendGuides(feedback, bounds, style, scene);
    appendGlyph(feedback, camera, style, scene);
}

}  // namespace app
