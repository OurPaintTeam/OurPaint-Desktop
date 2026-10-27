#include "ConstraintLayout.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <optional>
#include <utility>

#include "../../../core/sketch/Sketch.h"
#include "Camera2D.h"
#include "ConstraintMarkerStyle.h"
#include "ConstraintPresentationState.h"
#include "DimensionLayout.h"

namespace app {
namespace {

namespace sketch = core::sketch;

constexpr double kMinLineLength = 1e-12;

struct LineMarkers {
    glm::dvec2 start;
    glm::dvec2 end;
    glm::dvec2 tangent;
    glm::dvec2 normal;
    double lengthPx;
    std::vector<ConstraintMarker> markers;
};

bool finite(glm::dvec2 point) { return std::isfinite(point.x) && std::isfinite(point.y); }

bool validStyle(const ConstraintMarkerStyle& style) {
    for (double dimension : {style.lengthPx, style.offsetPx, style.hatchLengthPx, style.hatchSpacingPx,
                             style.minGapPx, style.hitTolerancePx, static_cast<double>(style.stroke.widthPx)}) {
        if (!std::isfinite(dimension) || dimension < 0.0) {
            return false;
        }
    }
    return style.lengthPx > 0.0;
}

size_t expectedRefCount(sketch::ConstraintType type) {
    switch (type) {
        case sketch::ConstraintType::Horizontal:
        case sketch::ConstraintType::Vertical:
        case sketch::ConstraintType::Fix:
            return 1;
        case sketch::ConstraintType::Parallel:
        case sketch::ConstraintType::Perpendicular:
            return 2;
        default:
            return 0;
    }
}

// Glyphs contain offsets around their attachment anchor. Placement does not
// depend on how each constraint symbol is constructed.
std::vector<ConstraintMarkerSegment> makeGlyph(sketch::ConstraintType type, glm::dvec2 tangent, glm::dvec2 normal, const ConstraintMarkerStyle& style) {
    std::vector<ConstraintMarkerSegment> strokes;
    const double halfLength = style.lengthPx * 0.5;

    if (type == sketch::ConstraintType::Fix) {
        const auto center = style.offsetPx * normal;
        return {{center - halfLength * tangent, center + halfLength * tangent}};
    }

    if (type == sketch::ConstraintType::Parallel) {
        strokes.reserve(2);
        for (double side : {-1.0, 1.0}) {
            const auto center = side * style.offsetPx * normal;
            strokes.push_back({center - halfLength * tangent, center + halfLength * tangent});
        }
        return strokes;
    }

    if (type == sketch::ConstraintType::Perpendicular) {
        const auto center = style.offsetPx * normal;
        strokes.reserve(2);
        strokes.push_back({center - halfLength * tangent, center + halfLength * tangent});
        strokes.push_back({center, center + halfLength * normal});
        return strokes;
    }

    // H/V glyphs retain their semantic orientation, even during an unsolved edit.
    const bool isHorizontal = (type == sketch::ConstraintType::Horizontal);
    const glm::dvec2 axis = isHorizontal ? glm::dvec2{1.0, 0.0} : glm::dvec2{0.0, 1.0};
    const auto center = -style.offsetPx * normal;
    strokes.reserve(4);
    strokes.push_back({center - halfLength * axis, center + halfLength * axis});

    // hatchSide is axis rotated 90° counter-clockwise.
    const glm::dvec2 hatchSide{-axis.y, axis.x};
    const auto hatch = (-axis + hatchSide) * (style.hatchLengthPx / std::sqrt(2.0));

    // Shift the row of three hatches along the axis.
    const auto hatchShift = (style.hatchSpacingPx / 3.0) * axis;

    for (int i = -1; i <= 1; ++i) {
        const auto start = center + static_cast<double>(i) * style.hatchSpacingPx * axis + hatchShift;
        strokes.push_back({start, start + hatch});
    }

    return strokes;
}

void placeMarkers(LineMarkers& line, const ConstraintMarkerStyle& style, std::vector<ConstraintMarker>& output) {
    for (auto& marker : line.markers) {
        std::sort(marker.constraints.begin(), marker.constraints.end());
    }
    std::sort(line.markers.begin(), line.markers.end(), [](const auto& a, const auto& b) {
        if (a.type != b.type) {
            return a.type < b.type;
        }
        return a.constraints.front() < b.constraints.front();
    });

    double alongRadius = 0.0;
    double acrossMin = 0.0;
    double acrossMax = 0.0;
    for (const auto& marker : line.markers) {
        for (const auto& stroke : marker.strokes) {
            for (auto point : {stroke.start, stroke.end}) {
                alongRadius = std::max(alongRadius, std::abs(glm::dot(point, line.tangent)));
                const double across = glm::dot(point, line.normal);
                acrossMin = std::min(acrossMin, across);
                acrossMax = std::max(acrossMax, across);
            }
        }
    }

    const double spacingPx = 2.0 * alongRadius + style.stroke.widthPx + style.minGapPx;
    const double rowSpacingPx = acrossMax - acrossMin + style.stroke.widthPx + style.minGapPx;
    const double slots = line.lengthPx / spacingPx;
    size_t rowCapacity = 1;
    if (slots >= static_cast<double>(line.markers.size()) + 1.0) {
        rowCapacity = line.markers.size();
    } else if (slots >= 3.0) {
        rowCapacity = static_cast<size_t>(std::floor(slots)) - 1;
    }

    // A short line still gets a centered symbol. Additional rows avoid overlap
    // among this line's markers without introducing a global collision solver.
    for (size_t first = 0, row = 0; first < line.markers.size(); first += rowCapacity, ++row) {
        const size_t count = std::min(rowCapacity, line.markers.size() - first);
        for (size_t i = 0; i < count; ++i) {
            const double t = static_cast<double>(i + 1) / static_cast<double>(count + 1);
            const auto anchor = line.start + t * (line.end - line.start) + static_cast<double>(row) * rowSpacingPx * line.normal;
            auto& marker = line.markers[first + i];
            for (auto& stroke : marker.strokes) {
                stroke.start += anchor;
                stroke.end += anchor;
            }
            output.push_back(std::move(marker));
        }
    }
}

bool glyphIsEntityAgnostic(sketch::ConstraintType type) {
    return type == sketch::ConstraintType::Parallel || type == sketch::ConstraintType::Perpendicular ||
           type == sketch::ConstraintType::Vertical || type == sketch::ConstraintType::Horizontal ||
           type == sketch::ConstraintType::Fix;
}

std::map<sketch::EntityId, LineMarkers> collectLines(const sketch::Sketch& sketch, const Camera2D& camera) {
    std::map<sketch::EntityId, LineMarkers> lines;
    const auto entities = sketch.lines();
    if (!entities) {
        return lines;
    }
    for (const auto& entity : entities.value()) {
        const auto* geometry = std::get_if<sketch::Line2>(&entity.geometry);
        if (!geometry) {
            continue;
        }
        const glm::dvec2 worldStart{geometry->start.x, geometry->start.y};
        const glm::dvec2 worldEnd{geometry->end.x, geometry->end.y};
        if (!finite(worldStart) || !finite(worldEnd) || std::hypot(worldEnd.x - worldStart.x, worldEnd.y - worldStart.y) <= kMinLineLength) {
            continue;
        }

        auto start = camera.worldToScreenLogical(worldStart);
        auto end = camera.worldToScreenLogical(worldEnd);
        const double length = std::hypot(end.x - start.x, end.y - start.y);
        if (!finite(start) || !finite(end) || !std::isfinite(length) || length <= kMinLineLength) {
            continue;
        }
        auto tangent = (end - start) / length;
        // Canonical endpoints keep ordering and the marker side independent of
        // the direction in which a line was originally drawn.
        const bool reverse = std::abs(tangent.x) >= std::abs(tangent.y) ? tangent.x < 0.0 : tangent.y < 0.0;
        if (reverse) {
            std::swap(start, end);
            tangent = -tangent;
        }
        lines.emplace(entity.id, LineMarkers{start, end, tangent, {tangent.y, -tangent.x}, length, {}});
    }
    return lines;
}

// Emits markers for line-based constraints.
void appendLineConstraint(const sketch::SketchConstraint& constraint, const ConstraintMarkerStyle& style, std::map<sketch::EntityId, LineMarkers>& lines) {
    const auto& definition = constraint.definition;

    // Reject malformed constraints early.
    const size_t refCount = expectedRefCount(definition.type);
    if (refCount == 0 || definition.refs.size() != refCount ||
        std::any_of(definition.refs.begin(), definition.refs.end(), [](const auto& ref) {
            return ref.sub != sketch::SubElement::Whole;
        })) {
        return;
    }
    // A two-reference constraint (Parallel/Perpendicular) pointing at the same
    // entity twice is degenerate; skip so a glyph is not drawn against itself.
    if (refCount == 2 && definition.refs[0].entity == definition.refs[1].entity) {
        return;
    }

    // Emit one marker per referenced line. A Parallel/Perpendicular constraint
    // therefore produces two markers (one per line) while still representing a
    // single constraint; both markers carry the same constraint ID.
    for (const auto& ref : definition.refs) {
        const auto found = lines.find(ref.entity);
        if (found == lines.end()) {
            // The entity is not a renderable line.
            continue;
        }
        auto& line = found->second;

        // Coalesce same-type glyphs on one line: two horizontal constraints on
        // the same line share a single glyph, and the marker keeps both IDs so
        // picking either constraint selects this glyph.
        if (glyphIsEntityAgnostic(definition.type)) {
            const auto existing = std::find_if(line.markers.begin(),
                                                                line.markers.end(),
                                                                [type = definition.type](const auto& marker) {
                return marker.type == type;
            });
            if (existing != line.markers.end()) {
                existing->constraints.push_back(constraint.id);
                continue; // Reuse the existing glyph; do not add a second one.
            }
        }

        // Build the glyph in the line's local frame. Offsets inside the glyph
        // are relative to the attachment anchor; placeMarkers() translates them
        // to their final screen positions. The hit radius is a capsule around
        // the stroke: half the pen width plus a picking tolerance.
        line.markers.push_back({{constraint.id},
                                ref,
                                definition.type,
                                makeGlyph(definition.type, line.tangent, line.normal, style),
                                {},
                                style.hitTolerancePx + static_cast<double>(style.stroke.widthPx) * 0.5});
    }
}

std::optional<glm::dvec2> screenPoint(const sketch::Sketch& sketch, sketch::GeometryRef ref, const Camera2D& camera) {
    const auto position = sketch.pointPosition(ref);
    if (!position || !finite({position.value().x, position.value().y})) {
        return std::nullopt;
    }
    const auto screen = camera.worldToScreenLogical({position.value().x, position.value().y});
    return finite(screen) ? std::optional<glm::dvec2>(screen) : std::nullopt;
}

void appendFixedPoint(const sketch::SketchConstraint& constraint, const sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style,
                      std::vector<ConstraintMarker>& output) {
    if (constraint.definition.refs.size() != 1) {
        return;
    }
    const auto ref = constraint.definition.refs.front();
    const auto center = screenPoint(sketch, ref, camera);
    if (!center) {
        return;
    }
    if (style.offsetPx <= 0.0) {
        return;
    }
    ConstraintMarker marker{{constraint.id}, ref, constraint.definition.type, {}, {}, style.hitTolerancePx + static_cast<double>(style.stroke.widthPx) * 0.5};
    marker.circles.push_back({*center, style.offsetPx});
    output.push_back(std::move(marker));
}

void appendFixedCurve(const sketch::SketchConstraint& constraint, const sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style,
                      std::vector<ConstraintMarker>& output) {
    const auto& refs = constraint.definition.refs;
    if (refs.size() != 1 || refs.front().sub != sketch::SubElement::Whole) {
        return;
    }
    const auto entity = sketch.entity(refs.front().entity);
    if (!entity) {
        return;
    }
    ConstraintMarker marker{
        {constraint.id}, refs.front(), constraint.definition.type, {}, {}, style.hitTolerancePx + static_cast<double>(style.stroke.widthPx) * 0.5};
    if (const auto* circle = std::get_if<sketch::Circle2>(&entity.value().geometry)) {
        const auto center = camera.worldToScreenLogical({circle->center.x, circle->center.y});
        const double radius = camera.worldToScreenLogical(circle->radius) - style.offsetPx;
        // Keep the requested gap; skip circles too small to contain the inset.
        if (!finite(center) || !std::isfinite(radius) || radius <= 0.0) {
            return;
        }
        marker.circles.push_back({center, radius});
    } else if (const auto* arc = std::get_if<sketch::Arc2>(&entity.value().geometry)) {
        const auto center = camera.worldToScreenLogical({arc->center.x, arc->center.y});
        const glm::dvec2 start{arc->start.x - arc->center.x, arc->start.y - arc->center.y};
        const glm::dvec2 end{arc->end.x - arc->center.x, arc->end.y - arc->center.y};
        // Match base arc rendering: its radius comes from the start point, even during an unsolved edit.
        const double radius = camera.worldToScreenLogical(std::hypot(start.x, start.y)) - style.offsetPx;
        if (!finite(center) || !finite(start) || !finite(end) || end == glm::dvec2{0.0, 0.0} || arc->start == arc->end || !std::isfinite(radius) ||
            radius <= 0.0) {
            return;
        }
        marker.arcs.push_back({center, radius, std::atan2(start.y, start.x), std::atan2(end.y, end.x)});
    } else {
        return;
    }
    output.push_back(std::move(marker));
}

struct GeometryRefLess {
    bool operator()(sketch::GeometryRef a, sketch::GeometryRef b) const { return a.entity != b.entity ? a.entity < b.entity : a.sub < b.sub; }
};

struct CoincidentNeighbor {
    sketch::GeometryRef ref;
    sketch::ConstraintId constraint;
};

struct CoincidentNode {
    std::vector<CoincidentNeighbor> neighbors;
    bool visited = false;
};

using CoincidentGraph = std::map<sketch::GeometryRef, CoincidentNode, GeometryRefLess>;

void collectCoincident(const sketch::SketchConstraint& constraint, const sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style,
                       CoincidentGraph& graph, std::vector<ConstraintMarker>& output) {
    const auto& refs = constraint.definition.refs;
    if (refs.size() != 2 || refs[0] == refs[1]) {
        return;
    }
    const auto first = sketch.pointPosition(refs[0]);
    const auto second = sketch.pointPosition(refs[1]);
    if (!first || !second) {
        // Point-on-curve coincidence does not join all points attached to that curve.
        if (first || second) {
            const auto attachment = first ? refs[0] : refs[1];
            if (const auto position = screenPoint(sketch, attachment, camera)) {
                output.push_back({{constraint.id},
                                  attachment,
                                  sketch::ConstraintType::Coincident,
                                  {},
                                  {{*position, style.point.radius, style.point.radius + style.hitTolerancePx}}});
            }
        }
        return;
    }
    graph[refs[0]].neighbors.push_back({refs[1], constraint.id});
    graph[refs[1]].neighbors.push_back({refs[0], constraint.id});
}

void appendCoincidentMarkers(CoincidentGraph& graph, const sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style,
                             std::vector<ConstraintMarker>& output) {
    for (auto& [attachment, node] : graph) {
        if (node.visited) {
            continue;
        }
        ConstraintMarker marker{{}, attachment, sketch::ConstraintType::Coincident, {}, {}};
        std::vector<sketch::GeometryRef> pending{attachment};
        node.visited = true;
        // Position deduplication happens only inside a topological coincidence group.
        std::map<std::pair<double, double>, glm::dvec2> positions;
        while (!pending.empty()) {
            const auto ref = pending.back();
            pending.pop_back();
            if (const auto position = screenPoint(sketch, ref, camera)) {
                positions.emplace(std::make_pair(position->x, position->y), *position);
            }
            for (const auto& neighbor : graph.at(ref).neighbors) {
                marker.constraints.push_back(neighbor.constraint);
                auto& next = graph.at(neighbor.ref);
                if (!next.visited) {
                    next.visited = true;
                    pending.push_back(neighbor.ref);
                }
            }
        }
        std::sort(marker.constraints.begin(), marker.constraints.end());
        marker.constraints.erase(std::unique(marker.constraints.begin(), marker.constraints.end()), marker.constraints.end());
        // An unsatisfied group keeps its actual positions instead of inventing a midpoint.
        for (const auto& position : positions) {
            marker.dots.push_back({position.second, style.point.radius, style.point.radius + style.hitTolerancePx});
        }
        if (!marker.dots.empty()) {
            output.push_back(std::move(marker));
        }
    }
}

void appendGeometricRelation(const sketch::SketchConstraint&, const sketch::Sketch&, const Camera2D&, const ConstraintMarkerStyle&,
                             std::vector<ConstraintMarker>&) {
    // TODO: Build relation-specific anchors and glyphs for Tangent, Equal,
    // Concentric, Symmetric and Midpoint. These do not share line-slot placement.
}

}  // namespace

void ConstraintLayout::rebuild(const sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style,
                               const ConstraintPresentationState& presentation, const render::text::TextMeasurer& textMeasurer) {
    // Single entry point: rebuild all markers from scratch.
    markers_.clear();

    // A non-finite or negative hit tolerance makes picking unusable; bail out.
    if (!std::isfinite(style.hitTolerancePx) || style.hitTolerancePx < 0.0) {
        return;
    }
    // No constraint list (or a failed lookup) means there is nothing to draw.
    const auto constraints = sketch.constraints();
    if (!constraints) {
        return;
    }

    // Separate validity flags: stroke glyphs require the whole style, dots only
    // need a finite radius. This lets e.g. coincident dots survive a bad style.
    const bool strokeGlyphsValid = validStyle(style);
    const bool dotsValid = std::isfinite(style.point.radius);
    // Precompute screen-space line frames once; skipped entirely if no strokes
    // can be drawn, so line-based constraints become no-ops.
    auto lines = strokeGlyphsValid ? collectLines(sketch, camera) : std::map<sketch::EntityId, LineMarkers>{};
    // Coincident constraints are collected into a graph and flushed after the
    // loop, because chains (A-B, B-C) must be merged into a single marker.
    CoincidentGraph coincidences;

    // Dispatch per constraint type. Three strategies: emit line-glyph specs into
    // `lines` (laid out later), build the coincident graph (flushed later), or
    // push a point/whole-entity marker directly into markers_.
    for (const auto& constraint : constraints.value()) {
        switch (constraint.definition.type) {
            case sketch::ConstraintType::Horizontal:
            case sketch::ConstraintType::Vertical:
            case sketch::ConstraintType::Parallel:
            case sketch::ConstraintType::Perpendicular:
                appendLineConstraint(constraint, style, lines);
                break;
            case sketch::ConstraintType::Coincident:
                if (dotsValid) {
                    collectCoincident(constraint, sketch, camera, style, coincidences, markers_);
                }
                break;
            case sketch::ConstraintType::Fix:
                if (strokeGlyphsValid) {
                    const auto& refs = constraint.definition.refs;
                    // Lines use shared slots; point rings and inset curves stay attached to their geometry.
                    if (refs.size() == 1 && refs.front().sub == sketch::SubElement::Whole && lines.contains(refs.front().entity)) {
                        appendLineConstraint(constraint, style, lines);
                    } else {
                        appendFixedPoint(constraint, sketch, camera, style, markers_);
                        appendFixedCurve(constraint, sketch, camera, style, markers_);
                    }
                }
                break;
            case sketch::ConstraintType::Tangent:
            case sketch::ConstraintType::Equal:
            case sketch::ConstraintType::Concentric:
            case sketch::ConstraintType::Symmetric:
            case sketch::ConstraintType::Midpoint:
                appendGeometricRelation(constraint, sketch, camera, style, markers_);
                break;
            case sketch::ConstraintType::Distance:
            case sketch::ConstraintType::HorizontalDistance:
            case sketch::ConstraintType::VerticalDistance:
            case sketch::ConstraintType::Length:
            case sketch::ConstraintType::Radius:
            case sketch::ConstraintType::Diameter:
            case sketch::ConstraintType::Angle:
                appendDimension(constraint, sketch, camera, style, presentation, textMeasurer, markers_);
                break;
        }
    }

    // Flush the coincident graph: each connected component becomes one marker
    // carrying every represented constraint ID and one dot per unique position.
    appendCoincidentMarkers(coincidences, sketch, camera, style, markers_);

    // Layout phase: position each line's glyphs along the line (translation from
    // local frame to screen-space) and move them into the final output.
    for (auto& entry : lines) {
        auto& line = entry.second;
        if (!line.markers.empty()) {
            placeMarkers(line, style, markers_);
        }
    }
}

}  // namespace app
