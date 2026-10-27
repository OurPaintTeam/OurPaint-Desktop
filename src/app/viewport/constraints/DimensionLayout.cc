#include "DimensionLayout.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <numbers>
#include <optional>
#include <utility>

#include "Camera2D.h"
#include "ConstraintLayout.h"
#include "ConstraintMarkerStyle.h"
#include "ConstraintPresentationState.h"
#include "sketch/Sketch.h"
#include "text/TextMeasurer.h"

namespace app {
namespace {
namespace sketch = core::sketch;
constexpr double kEpsilon = 1e-9;
constexpr double kTau = 2.0 * std::numbers::pi;

struct Line {
    glm::dvec2 start;
    glm::dvec2 end;
};
struct CircularCurve {
    glm::dvec2 center;
    double radius;
    double defaultAngle;
};

// Small 2D geometry helpers for glm::dvec2.
bool finite(glm::dvec2 p) {
    return std::isfinite(p.x) && std::isfinite(p.y);
}
double cross(glm::dvec2 a, glm::dvec2 b) {
    return a.x * b.y - a.y * b.x;
}
double length(glm::dvec2 v) {
    return std::hypot(v.x, v.y);
}
glm::dvec2 normal(glm::dvec2 u) {
    return {u.y, -u.x};
}
glm::dvec2 radial(double angle) {
    return {std::cos(angle), -std::sin(angle)};
}
double positiveAngle(double angle) {
    return angle - std::floor(angle / kTau) * kTau;
}

glm::dvec2 unit(glm::dvec2 v, glm::dvec2 fallback = {1.0, 0.0}) {
    const double magnitude = length(v);
    return std::isfinite(magnitude) && magnitude > kEpsilon ? v / magnitude : fallback;
}

glm::dvec2 readable(glm::dvec2 u) {
    return u.x < -kEpsilon || (std::abs(u.x) <= kEpsilon && u.y > 0.0) ? -u : u;
}

// Check ConstraintMarkerStyle parameters (is finite, is positive value)
bool validStyle(const ConstraintMarkerStyle& style) {
    const auto& d = style.dimension;
    for (const double value : {d.textHeightPx,
                               d.offsetPx,
                               d.textGapPx,
                               d.arrowLengthPx,
                               d.extensionGapPx,
                               d.extensionOverrunPx,
                               style.hitTolerancePx,
                               static_cast<double>(style.stroke.widthPx)}) {
        if (!std::isfinite(value) || value < 0.0) {
            return false;
        }
    }
    return d.textHeightPx > 0.0 && d.arrowLengthPx > 0.0 && d.precision >= 1 && d.precision <= 15;
}

// sketch::GeometryRef -> Point (in logical pixels)
std::optional<glm::dvec2> screenPoint(const sketch::Sketch& sketch, sketch::GeometryRef ref, const Camera2D& camera) {
    const auto point = sketch.pointPosition(ref);
    if (!point) {
        return std::nullopt;
    }
    const auto screen = camera.worldToScreenLogical({point.value().x, point.value().y});
    return finite(screen) ? std::optional(screen) : std::nullopt;
}

// sketch::GeometryRef -> Line (in logical pixels)
std::optional<Line> screenLine(const sketch::Sketch& sketch, sketch::GeometryRef ref, const Camera2D& camera) {
    if (ref.sub != sketch::SubElement::Whole) {
        return std::nullopt;
    }
    const auto entity = sketch.entity(ref.entity);
    if (!entity) {
        return std::nullopt;
    }
    const auto* line = std::get_if<sketch::Line2>(&entity.value().geometry);
    if (!line) {
        return std::nullopt;
    }
    Line result{camera.worldToScreenLogical({line->start.x, line->start.y}),
           camera.worldToScreenLogical({line->end.x, line->end.y})};
    return finite(result.start) &&
           finite(result.end) &&
           length(result.end - result.start) > kEpsilon ? std::optional(result) : std::nullopt;
}

// sketch::GeometryRef -> CircularCurve (in logical pixels)
std::optional<CircularCurve> screenCurve(const sketch::Sketch& sketch, sketch::GeometryRef ref, const Camera2D& camera) {
    if (ref.sub != sketch::SubElement::Whole) {
        return std::nullopt;
    }
    const auto entity = sketch.entity(ref.entity);
    if (!entity) {
        return std::nullopt;
    }
    CircularCurve result;
    if (const auto* circle = std::get_if<sketch::Circle2>(&entity.value().geometry)) {
        result = {camera.worldToScreenLogical({circle->center.x, circle->center.y}),
               camera.worldToScreenLogical(circle->radius), std::numbers::pi / 4.0};
    } else if (const auto* arc = std::get_if<sketch::Arc2>(&entity.value().geometry)) {
        const glm::dvec2 start{arc->start.x - arc->center.x, arc->start.y - arc->center.y};
        const glm::dvec2 end{arc->end.x - arc->center.x, arc->end.y - arc->center.y};
        if (!finite(start) || !finite(end) || length(end) <= kEpsilon || arc->start == arc->end) {
            return std::nullopt;
        }
        const double angle = std::atan2(start.y, start.x);
        result = {camera.worldToScreenLogical({arc->center.x, arc->center.y}),
               camera.worldToScreenLogical(length(start)),
                     angle + positiveAngle(std::atan2(end.y, end.x) - angle) * 0.5};
    } else {
        return std::nullopt;
    }
    return finite(result.center) && std::isfinite(result.radius) && result.radius > kEpsilon ? std::optional(result) : std::nullopt;
}

std::optional<glm::dvec2> userLabel(sketch::ConstraintId id, const ConstraintPresentationState& presentation, const Camera2D& camera) {
    const auto found = presentation.labelPlacements.find(id);
    if (found == presentation.labelPlacements.end()) {
        return std::nullopt;
    }
    const auto& position = found->second.position;
    const auto center = camera.worldToScreenLogical({position.x, position.y});
    return finite(center) ? std::optional(center) : std::nullopt;
}

// Make ConstraintLabel without defining center and rotationRad
std::optional<ConstraintLabel> makeLabel(const sketch::ConstraintDefinition& definition,
                                         const DimensionMarkerStyle& style,
                                         const render::text::TextMeasurer& measurer) {
    if (!definition.value || !std::isfinite(*definition.value) || *definition.value < 0.0) {
        return std::nullopt;
    }
    double value = *definition.value;
    if (definition.type == sketch::ConstraintType::Angle) {
        value *= 180.0 / std::numbers::pi; // rad to degree
    }
    if (!std::isfinite(value)) {
        return std::nullopt;
    }
    char buffer[64];
    const auto formatted = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::general, style.precision);
    if (formatted.ec != std::errc{}) {
        return std::nullopt;
    }
    std::string text(buffer, formatted.ptr);
    // ASCII notation matches the current renderer's atlas; units remain those of the sketch.
    if (definition.type == sketch::ConstraintType::Radius) {
        text = "R " + text;
    } else if (definition.type == sketch::ConstraintType::Diameter) {
        text = "D " + text;
    } else if (definition.type == sketch::ConstraintType::Angle) {
        text += " deg";
    }
    const auto metrics = measurer.measureText(text, style.textHeightPx);
    if (!metrics ||
        !std::isfinite(metrics->width) ||
        !std::isfinite(metrics->ascent) ||
        !std::isfinite(metrics->descent) ||
        metrics->width <= 0.0 ||
        metrics->ascent < 0.0 ||
        metrics->descent < 0.0 ||
        metrics->ascent + metrics->descent <= 0.0) {
        return std::nullopt;
    }
    return ConstraintLabel{std::move(text), {}, {metrics->width, metrics->ascent + metrics->descent}, style.textHeightPx, 0.0};
}

void placeLabel(ConstraintLabel& label, glm::dvec2 center, glm::dvec2 tangent) {
    label.center = center;
    tangent = readable(tangent);
    label.rotationRad = std::atan2(tangent.y, tangent.x);
}

void segment(ConstraintMarker& marker, glm::dvec2 start, glm::dvec2 end) {
    if (finite(start) && finite(end) && length(end - start) > kEpsilon) {
        marker.strokes.push_back({start, end});
    }
}

void arrow(ConstraintMarker& marker, glm::dvec2 tip, glm::dvec2 interior, double size) {
    const auto back = tip + interior * size;
    const auto wing = normal(interior) * (size * 0.4);
    segment(marker, tip, back + wing);
    segment(marker, tip, back - wing);
}

void witness(ConstraintMarker& marker, glm::dvec2 source, glm::dvec2 end, const DimensionMarkerStyle& style) {
    if (length(end - source) <= style.extensionGapPx) {
        return;
    }
    const auto direction = unit(end - source);
    segment(marker, source + direction * style.extensionGapPx, end + direction * style.extensionOverrunPx);
}

// Draw dimension line (populate a ConstraintMarker) between first and second.
// With label at center along direction.
// Extend the dimension line to the label even when the label lies beyond either witness.
// All linear families share the same measured text gap, arrows and extension policy.
void linearDimension(ConstraintMarker& marker,
                     glm::dvec2 first,
                     glm::dvec2 second,
                     glm::dvec2 direction,
                     ConstraintLabel label,
                     glm::dvec2 labelCenter,
                     const DimensionMarkerStyle& style,
                     bool drawWitnesses = true,
                     bool firstArrow = true) {
    // Canonical unit direction of the dimension line (stable under u/-u flips).
    const auto u = readable(direction);
    // Normal to the dimension line: the axis along which the label is offset from the measured points.
    const auto n = normal(u);
    // Position the label along u; also fills in label.sizePx, used below.
    placeLabel(label, labelCenter, u);
    // Project first/second onto the line through labelCenter with normal n.
    // a and b are the endpoints of the dimension line itself (arrow tips sit here),
    // not the raw measured points.
    const auto a = first + n * glm::dot(labelCenter - first, n);
    const auto b = second + n * glm::dot(labelCenter - second, n);
    // Signed length of the dimension line along u (0 at a, end at b).
    const double end = glm::dot(b - a, u);
    // Parametric position of the label center along u, measured from a.
    const double text = glm::dot(labelCenter - a, u);
    // Half-width of the gap to cut out of the dimension line for the label (half the text width plus padding on each side).
    const double halfGap = label.sizePx.x * 0.5 + style.textGapPx;
    // "Short dimension" case: text + gaps + arrows do not fit between a and b,
    // so arrows must be placed outside and the line extended past the tips.
    const bool outsideArrows = std::abs(end) < label.sizePx.x + 2.0 * (style.textGapPx + style.arrowLengthPx);
    // Extra line length added past an arrow (visual tail).
    const double tail = style.arrowLengthPx * 1.5;
    // Initial parametric span of the dimension line: from a (0) to b (end).
    double low = std::min(0.0, end);
    double high = std::max(0.0, end);
    // Short dimension: extend the line beyond both arrow positions.
    if (outsideArrows) {
        low -= tail;
        high += tail;
    }
    // Also extend if the label overhangs the current span, so the text gap
    // is fully inside [low, high] with a tail of margin.
    if (text - halfGap < low) {
        low = text - halfGap - tail;
    }
    if (text + halfGap > high) {
        high = text + halfGap + tail;
    }
    // Draw a segment between two parametric positions along u, skipping
    // degenerate intervals.
    const auto interval = [&](double from, double to) {
        if (to - from > kEpsilon) {
            segment(marker, a + from * u, a + to * u);
        }
    };
    // Split the dimension line around the text gap: [low .. text-halfGap]
    // and [text+halfGap .. high].
    interval(low, std::min(high, text - halfGap));
    interval(std::max(low, text + halfGap), high);
    // Extension (witness) lines from the measured points to the dimension line.
    if (drawWitnesses) {
        witness(marker, first, a, style);
        witness(marker, second, b, style);
    }
    // Arrow direction at a:
    //   normal case  -> points from a toward b (inward),
    //   short case   -> flipped to point outward, since arrows are placed outside.
    const auto interior = (end < 0.0 ? -u : u) * (outsideArrows ? -1.0 : 1.0);
    // First arrow is optional (e.g. radius/single-sided dimensions draw only one);
    // second arrow is always drawn.
    if (firstArrow) {
        arrow(marker, a, interior, style.arrowLengthPx);
    }
    arrow(marker, b, -interior, style.arrowLengthPx);
    marker.labels.push_back(std::move(label));
}

// project point onto line
glm::dvec2 project(glm::dvec2 point, const Line& line, bool clampToSegment = false) {
    const auto u = unit(line.end - line.start);
    // Scalar projection of (point - start) onto the line direction.
    double along = glm::dot(point - line.start, u);
    if (clampToSegment) {
        // Clamp to the segment extent when projecting onto a segment.
        along = std::clamp(along, 0.0, length(line.end - line.start));
    }
    return line.start + along * u;
}

// anchor line (in logical pixels)
// Get only: Distance, HorizontalDistance, VerticalDistance, Length
std::optional<Line> linearAnchors(const sketch::ConstraintDefinition& definition, const sketch::Sketch& sketch, const Camera2D& camera) {
    const auto& refs = definition.refs;

    // Length (line) constraint
    if (definition.type == sketch::ConstraintType::Length) {
        return refs.size() == 1 ? screenLine(sketch, refs.front(), camera) : std::nullopt;
    }

    // Distance / HorizontalDistance / VerticalDistance constraint
    if (refs.size() != 2) {
        return std::nullopt;
    }

    // Two objects is points
    const auto first = screenPoint(sketch, refs[0], camera);
    const auto second = screenPoint(sketch, refs[1], camera);
    if (first && second) {
        return Line{*first, *second}; // line between two points
    }

    // Only Distance constraint
    if (definition.type != sketch::ConstraintType::Distance) {
        return std::nullopt;
    }
    const auto firstLine = screenLine(sketch, refs[0], camera);
    const auto secondLine = screenLine(sketch, refs[1], camera);
    if (first && secondLine) { // point + line
        return Line{*first, project(*first, *secondLine)};
    }
    if (second && firstLine) { // line + point
        return Line{project(*second, *firstLine), *second};
    }
    if (firstLine && secondLine) { // line + line (only for parallel lines)
        // check lines is parallel
        if (std::abs(cross(unit(firstLine->end - firstLine->start),
                            unit(secondLine->end - secondLine->start))) > 1e-6) {
            return std::nullopt;  // Intersecting supporting lines do not have a nonzero separation dimension.
        }
        // if parallel
        const auto midpoint = firstLine->start + (firstLine->end - firstLine->start) * 0.5;
        return Line{midpoint, project(midpoint, *secondLine)};
    }

    // point + curve
    if (const auto point = first ? first : second) {
        if (const auto curve = screenCurve(sketch, refs[first ? 1 : 0], camera)) {
            return Line{*point, curve->center + unit(*point - curve->center) * curve->radius};
        }
    }
    return std::nullopt;
}

void leaderToLabel(ConstraintMarker& marker, glm::dvec2 start, const ConstraintLabel& label, double gap) {
    const glm::dvec2 u{std::cos(label.rotationRad), std::sin(label.rotationRad)};
    const auto delta = start - label.center;
    const double scale =
        std::max(std::abs(glm::dot(delta, u)) / (label.sizePx.x * 0.5 + gap), std::abs(glm::dot(delta, normal(u))) / (label.sizePx.y * 0.5 + gap));
    if (scale > 1.0) {
        segment(marker, start, label.center + delta / scale);
    }
}

void angularDimension(ConstraintMarker& marker, const Line& first, const Line& second, ConstraintLabel label, std::optional<glm::dvec2> placement,
                      const DimensionMarkerStyle& style) {
    const auto u = unit(first.end - first.start);
    const auto v = unit(second.end - second.start);
    const double determinant = cross(u, v);
    if (std::abs(determinant) <= 1e-6) {
        // Parallel lines have no unique vertex; use two leaders instead of inventing an arc center.
        const auto a = first.start + (first.end - first.start) * 0.5;
        const auto b = second.start + (second.end - second.start) * 0.5;
        placeLabel(label, placement.value_or(a + (b - a) * 0.5 + normal(readable(u)) * style.offsetPx), u);
        leaderToLabel(marker, a, label, style.textGapPx);
        leaderToLabel(marker, b, label, style.textGapPx);
        marker.labels.push_back(std::move(label));
        return;
    }
    const auto center = first.start + u * (cross(second.start - first.start, v) / determinant);
    if (!finite(center)) {
        return;
    }
    const double firstAngle = std::atan2(-u.y, u.x);
    const double secondAngle = std::atan2(-v.y, v.x);
    const double delta = std::remainder(secondAngle - firstAngle, kTau);
    const double start = delta < 0.0 ? secondAngle : firstAngle;
    const double sweep = std::abs(delta);
    const double middle = start + sweep * 0.5;
    double radius = std::max(style.offsetPx, (label.sizePx.x + 2.0 * (style.textGapPx + style.arrowLengthPx)) / sweep);
    if (placement && length(*placement - center) > kEpsilon) {
        radius = length(*placement - center);
    }
    if (!std::isfinite(radius) || radius <= kEpsilon) {
        return;
    }
    const auto labelCenter = placement.value_or(center + radial(middle) * radius);
    const double labelAngle = length(labelCenter - center) > kEpsilon ? std::atan2(center.y - labelCenter.y, labelCenter.x - center.x) : middle;
    placeLabel(label, labelCenter, {-std::sin(labelAngle), -std::cos(labelAngle)});
    const double relative = positiveAngle(labelAngle - start);
    const auto arc = [&](double from, double to) {
        // The shader treats sweeps below 1e-5 as full circles; allow for float rounding.
        if (to - from > 2e-5) {
            marker.arcs.push_back({center, radius, from, to});
        }
    };
    if (relative <= sweep && length(labelCenter - center) > kEpsilon) {
        // A conservative gap enclosing the rotated label's measured rectangle.
        const double boxRadius = length(label.sizePx * 0.5 + glm::dvec2(style.textGapPx));
        const double halfGap = 2.0 * std::asin(std::min(1.0, boxRadius / (2.0 * radius)));
        arc(start, std::max(start, start + relative - halfGap));
        arc(std::min(start + sweep, start + relative + halfGap), start + sweep);
    } else {
        arc(start, start + sweep);
        const double nearest =
            std::abs(std::remainder(labelAngle - start, kTau)) < std::abs(std::remainder(labelAngle - start - sweep, kTau)) ? start : start + sweep;
        leaderToLabel(marker, center + radial(nearest) * radius, label, style.textGapPx);
    }
    const auto firstTip = center + u * radius;
    const auto secondTip = center + v * radius;
    witness(marker, project(firstTip, first, true), firstTip, style);
    witness(marker, project(secondTip, second, true), secondTip, style);
    const auto startDirection = radial(start);
    const auto endDirection = radial(start + sweep);
    arrow(marker, center + startDirection * radius, normal(startDirection), style.arrowLengthPx);
    arrow(marker, center + endDirection * radius, -normal(endDirection), style.arrowLengthPx);
    marker.labels.push_back(std::move(label));
}
}  // namespace

void appendDimension(const sketch::SketchConstraint& constraint,
                     const sketch::Sketch& sketch,
                     const Camera2D& camera,
                     const ConstraintMarkerStyle& style,
                     const ConstraintPresentationState& presentation,
                     const render::text::TextMeasurer& textMeasurer,
                     std::vector<ConstraintMarker>& output) {
    const auto& definition = constraint.definition;
    if (!validStyle(style) || definition.refs.empty()) {
        return;
    }

    // make text label
    auto label = makeLabel(definition, style.dimension, textMeasurer);
    if (!label) {
        return;  // Font metrics must be available before laying out a dimension.
    }

    // user placement
    const auto placement = userLabel(constraint.id, presentation, camera);
    ConstraintMarker marker{
        {constraint.id},
        definition.refs.front(),
        definition.type,
        {},
        {},
        style.hitTolerancePx + static_cast<double>(style.stroke.widthPx) * 0.5};

    switch (definition.type) {
        case sketch::ConstraintType::Distance:
        case sketch::ConstraintType::HorizontalDistance:
        case sketch::ConstraintType::VerticalDistance:
        case sketch::ConstraintType::Length: {
            // Anchor line
            const auto anchors = linearAnchors(definition, sketch, camera);
            if (!anchors) {
                return;
            }
            auto direction = unit(anchors->end - anchors->start);
            if (definition.type == sketch::ConstraintType::HorizontalDistance) {
                direction = {1.0, 0.0};
            } else if (definition.type == sketch::ConstraintType::VerticalDistance) {
                direction = {0.0, -1.0};
            }
            const auto u = readable(direction);
            const auto& d = style.dimension;
            const auto midpoint = anchors->start + (anchors->end - anchors->start) * 0.5;
            auto automaticCenter = midpoint + normal(u) * d.offsetPx;
            const double span = std::abs(glm::dot(anchors->end - anchors->start, u));
            if (span < label->sizePx.x + 2.0 * (d.textGapPx + d.arrowLengthPx)) {
                // Put automatic labels beyond short spans so even a zero distance stays legible.
                automaticCenter += u * (span * 0.5 + d.arrowLengthPx * 1.5 + d.textGapPx + label->sizePx.x * 0.5);
            }
            linearDimension(marker, anchors->start, anchors->end, direction, *label, placement.value_or(automaticCenter), d);
            break;
        }
        case sketch::ConstraintType::Radius:
        case sketch::ConstraintType::Diameter: {
            if (definition.refs.size() != 1) {
                return;
            }
            const auto curve = screenCurve(sketch, definition.refs.front(), camera);
            if (!curve) {
                return;
            }
            const auto direction = placement ? unit(*placement - curve->center, radial(curve->defaultAngle)) : radial(curve->defaultAngle);
            const bool diameter = definition.type == sketch::ConstraintType::Diameter;
            const auto first = diameter ? curve->center - direction * curve->radius : curve->center;
            const auto second = curve->center + direction * curve->radius;
            linearDimension(marker, first, second, direction, *label,
                            placement.value_or(second + direction * (style.dimension.offsetPx + label->sizePx.x * 0.5)), style.dimension, false, diameter);
            break;
        }
        case sketch::ConstraintType::Angle: {
            if (definition.refs.size() != 2) {
                return;
            }
            const auto first = screenLine(sketch, definition.refs[0], camera);
            const auto second = screenLine(sketch, definition.refs[1], camera);
            if (!first || !second) {
                return;
            }
            angularDimension(marker, *first, *second, *label, placement, style.dimension);
            break;
        }
        default:
            return;
    }
    if (!marker.labels.empty()) {
        output.push_back(std::move(marker));
    }
}
}  // namespace app
