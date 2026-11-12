#ifndef OURPAINT_APPLICATION_CONSTRAINT_LAYOUT_H_
#define OURPAINT_APPLICATION_CONSTRAINT_LAYOUT_H_

#include <glm/glm.hpp>
#include <string>
#include <vector>

#include "SketchTypes.h"

class Camera2D;

namespace render::text {
class TextMeasurer;
}

namespace core::sketch {
class Sketch;
}

namespace app {

struct ConstraintMarkerStyle;
struct ConstraintPresentationState;

// All positions and distances use logical screen pixels, with a top-left origin.
struct ConstraintMarkerSegment {
    glm::dvec2 start;
    glm::dvec2 end;
};

struct ConstraintDot {
    glm::dvec2 center;
    double radiusPx;
    double hitRadiusPx;
};

struct ConstraintCircle {
    glm::dvec2 center;
    double radiusPx;
};

struct ConstraintArc {
    glm::dvec2 center;
    double radiusPx;
    // Counterclockwise radians from screen-right, with angular +Y pointing up.
    // A point on the arc is center + radiusPx * (cos(angle), -sin(angle)).
    double startAngle;
    double endAngle;
};

struct ConstraintLabel {
    std::string text;
    glm::dvec2 center;
    // Measured width and height in the label's unrotated local frame.
    glm::dvec2 sizePx;
    // Nominal font height; visible glyph bounds are given by sizePx.
    double heightPx;
    // Rotation in logical screen coordinates (+Y down), kept readable by layout.
    double rotationRad;
};

struct ConstraintMarker {
    // Nonempty, sorted IDs. Grouped glyphs retain every represented constraint.
    std::vector<core::sketch::ConstraintId> constraints;
    // A deterministic local attachment, not a replacement for the constraint's references.
    core::sketch::GeometryRef attachment;
    core::sketch::ConstraintType type;
    std::vector<ConstraintMarkerSegment> strokes;
    std::vector<ConstraintDot> dots;
    // Picking tolerance around outlines and label rectangles. Dots have their own hit radii.
    double hitRadiusPx = 0.0;
    std::vector<ConstraintCircle> circles{};
    std::vector<ConstraintArc> arcs{};
    std::vector<ConstraintLabel> labels{};
};

// Shared presentation data for rendering and dimension picking. Rebuild
// invalidates borrowed markers and must follow geometry, camera, style,
// presentation-state or font-metric changes.
class ConstraintLayout {
public:
    void rebuild(const core::sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style,
                 const ConstraintPresentationState& presentation, const render::text::TextMeasurer& textMeasurer);
    const std::vector<ConstraintMarker>& markers() const noexcept { return markers_; }

private:
    std::vector<ConstraintMarker> markers_;
};

}  // namespace app

#endif
