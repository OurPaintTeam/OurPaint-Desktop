#ifndef OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_
#define OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_

#include "DocumentManager.h"
#include "IRenderer.h"
#include "IViewportController.h"
#include "IViewportHost.h"
#include "InputEvents.h"
#include "InteractionTools/EditorSession.h"
#include "RenderData.h"
#include "Camera2D.h"
#include "RenderDataBuilder.h"

class ViewportController : public IViewportController {
public:
    ViewportController(Camera2D& view,
                       EditorSession& session,
                       IRenderer& renderer,
                       renderer::RenderData& renderScene,
                       RenderDataBuilder& builder);

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
    EditorSession& editorSession_;
    IRenderer& renderer_;
    renderer::RenderData& renderScene_;
    RenderDataBuilder& builder_;

    bool panning_ = false;
    double lastX_ = 0.0;
    double lastY_ = 0.0;

    bool _ini = false;
};

#endif // ! OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_