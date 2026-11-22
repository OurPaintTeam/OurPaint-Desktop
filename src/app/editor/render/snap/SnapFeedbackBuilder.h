#ifndef OURPAINT_APP_SNAP_FEEDBACK_BUILDER_H_
#define OURPAINT_APP_SNAP_FEEDBACK_BUILDER_H_

namespace core::sketch {
class Sketch;
}

namespace render {
struct RenderScene;
class Camera2D;
}

namespace app {

struct SnapFeedback;
struct SnapFeedbackStyle;

void appendSnapFeedback(const SnapFeedback& feedback, const core::sketch::Sketch& sketch, const render::Camera2D& camera, const SnapFeedbackStyle& style,
                        render::RenderScene& scene);

}  // namespace app

#endif  // OURPAINT_APP_SNAP_FEEDBACK_BUILDER_H_
