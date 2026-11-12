#ifndef OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_
#define OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_

#include "../../core/Document.h"
#include "../../render/Camera2D.h"
#include "../../render/RenderScene.h"
#include "IRenderer.h"
#include "IViewportController.h"
#include "InputEvents.h"
#include "OpenGL2dRenderer.h"
#include "SketchEditor.h"
#include "render/AxisTexts.h"
#include "render/RenderSceneBuilder.h"
#include "render/ViewportStyle.h"

namespace app {
struct ConstraintPresentationState;
}

class ViewportController : public IViewportController {
public:
    ViewportController(Camera2D& camera, OverlayModel& overlay, app::SketchEditor& session, Document& document, const app::ViewportStyle& style,
                       app::ConstraintLayout& constraintLayout, const app::ConstraintPresentationState& constraintPresentation);

    bool onResize     (const input::ResizeEvent& e)      override;
    bool onMouseMove  (const input::MouseMoveEvent& e)   override;
    bool onMouseButton(const input::MouseButtonEvent& e) override;
    bool onWheel      (const input::WheelEvent& e)       override;
    bool onKey        (const input::KeyEvent& e)         override;

    void render() override;

    // Release GPU resources while this view's OpenGL context is still current.
    void shutdown();

    void requestRedraw();
    void setContinuousRedraw(bool enable);

private:
    void updateConstraintLayout();

    Camera2D& camera2D_;
    OverlayModel& overlay_;
    app::SketchEditor& editorSession_;
    Document& document_;
    app::ConstraintLayout& constraintLayout_;
    const app::ConstraintPresentationState& constraintPresentation_;

    AxisTexts axisTexts_;
    const app::ViewportStyle& viewportStyle_;
    render::RenderScene renderScene_;
    RenderSceneBuilder builder_;
    render::OpenGL2dRenderer renderer_;

    double lastX_ = 0.0;
    double lastY_ = 0.0;

    bool _ini = false;
};

#endif // ! OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_
