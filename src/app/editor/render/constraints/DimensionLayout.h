#ifndef OURPAINT_APPLICATION_DIMENSION_LAYOUT_H_
#define OURPAINT_APPLICATION_DIMENSION_LAYOUT_H_

#include <glm/vec2.hpp>
#include <optional>
#include <variant>
#include <vector>

#include "ConstraintPresentationState.h"

namespace render {
class Camera2D;
}
namespace core::sketch {
class Sketch;
struct SketchConstraint;
}  // namespace core::sketch
namespace render::text {
class TextMeasurer;
}
namespace app {
struct ConstraintMarker;
struct ConstraintMarkerStyle;

// Derived from the current geometry and camera. Both conversions use logical
// screen pixels (+Y down) and store lengths in sketch units (+Y up).
struct DimensionPlacementFrame {
    struct Linear {
        glm::dvec2 tangent;
        double alongScalePx;
    };
    struct Polar {
        double referenceAngle;
        double referenceRadiusPx;
    };
    glm::dvec2 originPx;
    double pixelsPerUnit;
    std::variant<Linear, Polar> axes;

    std::optional<ConstraintLabelPlacement> toLocal(glm::dvec2 centerPx) const;
    std::optional<glm::dvec2> toScreen(const ConstraintLabelPlacement& placement) const;
};

// Builds presentation only; it neither validates solver support nor mutates Sketch.
void appendDimension(const core::sketch::SketchConstraint& constraint,
                     const core::sketch::Sketch& sketch,
                     const render::Camera2D& camera,
                     const ConstraintMarkerStyle& style,
                     const ConstraintPresentationState& presentation,
                     const render::text::TextMeasurer& textMeasurer,
                     std::vector<ConstraintMarker>& output);
}  // namespace app

#endif  // OURPAINT_APPLICATION_DIMENSION_LAYOUT_H_
