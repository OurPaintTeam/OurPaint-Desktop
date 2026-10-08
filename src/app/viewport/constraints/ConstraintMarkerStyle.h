#ifndef OURPAINT_APPLICATION_CONSTRAINT_MARKER_STYLE_H_
#define OURPAINT_APPLICATION_CONSTRAINT_MARKER_STYLE_H_

#include "RenderScene.h"

namespace app {

// All marker sizes and stroke widths are in logical screen pixels.
struct ConstraintMarkerStyle {
    render::StrokeStyle stroke;
    double lengthPx = 25.0;
    double offsetPx = 5.0;
    double hatchLengthPx = 6.0;
    double hatchSpacingPx = 6.0;
    double minGapPx = 6.0;
    double hitTolerancePx = 4.0;
};

}  // namespace app

#endif  // OURPAINT_APPLICATION_CONSTRAINT_MARKER_STYLE_H_
