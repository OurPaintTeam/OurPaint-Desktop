#ifndef OURPAINT_APPLICATION_DIMENSION_LAYOUT_H_
#define OURPAINT_APPLICATION_DIMENSION_LAYOUT_H_

#include <vector>

class Camera2D;
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
struct ConstraintPresentationState;

// Builds presentation only; it neither validates solver support nor mutates Sketch.
void appendDimension(const core::sketch::SketchConstraint& constraint,
                     const core::sketch::Sketch& sketch,
                     const Camera2D& camera,
                     const ConstraintMarkerStyle& style,
                     const ConstraintPresentationState& presentation,
                     const render::text::TextMeasurer& textMeasurer,
                     std::vector<ConstraintMarker>& output);
}  // namespace app

#endif  // OURPAINT_APPLICATION_DIMENSION_LAYOUT_H_
