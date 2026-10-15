#ifndef OURPAINT_APPLICATION_SNAP_FEEDBACK_STYLE_H_
#define OURPAINT_APPLICATION_SNAP_FEEDBACK_STYLE_H_

#include "RenderScene.h"

namespace app {

// Sizes, stroke widths and softness are logical screen pixels. The scene
// builder handles framebuffer conversion for the screen-space glyph.
struct SnapFeedbackStyle {
    render::StrokeStyle marker;
    render::MarkerStyle referencePoint;
    render::StrokeStyle referenceCurve;
    render::StrokeStyle guide;
    double glyphSizePx = 10.0;
};

}  // namespace app

#endif  // OURPAINT_APPLICATION_SNAP_FEEDBACK_STYLE_H_
