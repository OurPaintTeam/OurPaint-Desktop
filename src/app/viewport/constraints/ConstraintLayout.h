#ifndef OURPAINT_APPLICATION_CONSTRAINT_LAYOUT_H_
#define OURPAINT_APPLICATION_CONSTRAINT_LAYOUT_H_

#include <glm/glm.hpp>
#include <vector>

#include "../../../core/sketch/SketchTypes.h"

class Camera2D;

namespace core::sketch {
class Sketch;
}

namespace app {

struct ConstraintMarkerStyle;

// All positions and distances use logical screen pixels, with a top-left origin.
struct ConstraintMarkerSegment {
    glm::dvec2 start;
    glm::dvec2 end;
};

struct ConstraintMarker {
    // Nonempty, sorted IDs. A single Parallel glyph can represent several constraints.
    std::vector<core::sketch::ConstraintId> constraints;
    core::sketch::GeometryRef attachment;
    core::sketch::ConstraintType type;
    std::vector<ConstraintMarkerSegment> strokes;
    // Future picking can test a capsule of this radius around each drawn stroke.
    double hitRadiusPx;
};

// Shared presentation data for rendering and future constraint picking. Rebuild
// invalidates borrowed markers and must follow geometry, camera, or style changes.
class ConstraintLayout {
public:
    void rebuild(const core::sketch::Sketch& sketch, const Camera2D& camera, const ConstraintMarkerStyle& style);
    const std::vector<ConstraintMarker>& markers() const noexcept { return markers_; }

private:
    std::vector<ConstraintMarker> markers_;
};

}  // namespace app

#endif
