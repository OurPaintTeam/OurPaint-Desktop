#include "ConstraintLayout.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

#include "../../../core/sketch/Sketch.h"
#include "Camera2D.h"
#include "ConstraintMarkerStyle.h"

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

    if (type == sketch::ConstraintType::Fix) {
        constexpr double k = 0.7071067811865476;   // sqrt(2)/2
        const glm::dvec2 d1{ k,  k};
        const glm::dvec2 d2{ k, -k};
        strokes.push_back({-halfLength * d1, halfLength * d1});
        strokes.push_back({-halfLength * d2, halfLength * d2});
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
    return type == sketch::ConstraintType::Parallel ||
           type == sketch::ConstraintType::Perpendicular;
}

}  // namespace

void ConstraintLayout::rebuild(const sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style) {
    markers_.clear();
    if (!validStyle(style)) {
        return;
    }

    const auto constraints = sketch.constraints();
    const auto entities = sketch.lines();
    if (!constraints || !entities) {
        return;
    }

    std::map<sketch::EntityId, LineMarkers> lines;
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

    for (const auto& constraint : constraints.value()) {
        const auto& definition = constraint.definition;
        const size_t refCount = expectedRefCount(definition.type);
        if (refCount == 0 || definition.refs.size() != refCount ||
            std::any_of(definition.refs.begin(), definition.refs.end(), [](const auto& ref) { return ref.sub != sketch::SubElement::Whole; })) {
            continue;
        }
        if (refCount == 2 && definition.refs[0].entity == definition.refs[1].entity) {
            continue;
        }
        for (const auto& ref : definition.refs) {
            const auto found = lines.find(ref.entity);
            if (found == lines.end()) {
                continue;
            }
            auto& line = found->second;
            if (glyphIsEntityAgnostic(definition.type)) {
                const auto existing = std::find_if(
                    line.markers.begin(), line.markers.end(),
                    [type = definition.type](const auto& marker) { return marker.type == type; });
                if (existing != line.markers.end()) {
                    existing->constraints.push_back(constraint.id);
                    continue;
                }
            }
            line.markers.push_back({{constraint.id},
                                    ref,
                                    definition.type,
                                    makeGlyph(definition.type, line.tangent, line.normal, style),
                                    style.hitTolerancePx + static_cast<double>(style.stroke.widthPx) * 0.5});
        }
    }

    for (auto& entry : lines) {
        auto& line = entry.second;
        if (!line.markers.empty()) {
            placeMarkers(line, style, markers_);
        }
    }
}

}  // namespace app
