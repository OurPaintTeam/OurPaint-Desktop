#ifndef OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_
#define OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_

#include "../../core/Document.h"
#include "../editor/SketchEditor.h"
#include "../platform/IViewportHost.h"
#include "../platform/InputEvents.h"
#include "AxisTexts.h"
#include "Camera2D.h"
#include "IRenderer.h"
#include "IViewportController.h"
#include "OpenGL2dRenderer.h"
#include "RenderScene.h"
#include "ViewportStyle.h"
#include "render/RenderSceneBuilder.h"

class ViewportController : public IViewportController {
public:
    ViewportController(Camera2D& camera,
                       OverlayModel& overlay,
                       SketchEditor& session,
                       Document& document,
                       const app::ViewportStyle& style,
                       app::ConstraintLayout& constraintLayout);

    bool onResize     (const input::ResizeEvent& e)      override;
    bool onMouseMove  (const input::MouseMoveEvent& e)   override;
    bool onMouseButton(const input::MouseButtonEvent& e) override;
    bool onWheel      (const input::WheelEvent& e)       override;
    bool onKey        (const input::KeyEvent& e)         override;

    void render() override;

    void requestRedraw();
    void setContinuousRedraw(bool enable);

private:
    void updateConstraintLayout();

    Camera2D& camera2D_;
    OverlayModel& overlay_;
    SketchEditor& editorSession_;
    Document& document_;
    app::ConstraintLayout& constraintLayout_;

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
