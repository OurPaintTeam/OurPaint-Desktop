#ifndef OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_
#define OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_

#include "../editor/EditorSession.h"
#include "../platform/IViewportHost.h"
#include "../platform/InputEvents.h"
#include "Camera2D.h"
#include "Document.h"
#include "IRenderer.h"
#include "IViewportController.h"
#include "RenderData.h"
#include "rendering/RenderDataBuilder.h"
#include "OpenGLRenderer.h"
#include "AxisTexts.h"

class ViewportController : public IViewportController {
public:
    ViewportController(Camera2D& camera,
                       OverlayModel& overlay,
                       EditorSession& session,
                       Document& document);

    bool onResize     (const input::ResizeEvent& e)      override;
    bool onMouseMove  (const input::MouseMoveEvent& e)   override;
    bool onMouseButton(const input::MouseButtonEvent& e) override;
    bool onWheel      (const input::WheelEvent& e)       override;
    bool onKey        (const input::KeyEvent& e)         override;

    void render() override;

    void requestRedraw();
    void setContinuousRedraw(bool enable);

private:
    Camera2D& camera2D_;
    OverlayModel& overlay_;
    EditorSession& editorSession_;
    Document& document_;

    AxisTexts axisTexts_;
    RenderDataBuilder builder_;
    renderer::RenderData renderScene_;
    renderer::OpenGLRenderer renderer_;

    double lastX_ = 0.0;
    double lastY_ = 0.0;

    bool _ini = false;
};

#endif // ! OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_