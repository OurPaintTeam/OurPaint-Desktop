#ifndef OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_
#define OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_

#include <functional>
#include <utility>

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

// Frontend entry point for input, resize and drawing for one document view. The host
// translates native events and provides the current graphics context and render target.
// The view owns this controller and the editor state it borrows. Hosts detach before
// the view is destroyed and call shutdown() while its graphics context is current.
// Route project and toolbar commands through UIController; keep widget types out of both APIs.
class ViewportController : public IViewportController {
public:
    ViewportController(render::Camera2D& camera,
                       OverlayModel& overlay,
                       app::SketchEditor& session,
                       Document& document,
                       const app::ViewportStyle& style,
                       app::ConstraintLayout& constraintLayout,
                       const app::ConstraintPresentationState& constraintPresentation);

    bool onResize     (const input::ResizeEvent& e)      override;
    bool onMouseMove  (const input::MouseMoveEvent& e)   override;
    bool onMouseButton(const input::MouseButtonEvent& e) override;
    bool onWheel      (const input::WheelEvent& e)       override;
    bool onKey        (const input::KeyEvent& e)         override;

    void render() override;

    // Release GPU resources while this view's OpenGL context is still current.
    void shutdown() override;

    // Shared geometry may change during a gesture. The owning workspace schedules
    // redraws for sibling views without copying this view's camera or selection.
    void setSiblingRedrawCallback(std::function<void()> callback) { redrawSiblings_ = std::move(callback); }
    void requestRedraw();
    void setContinuousRedraw(bool enable);

private:
    void updateConstraintLayout();

    render::Camera2D& camera2D_;
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

    std::function<void()> redrawSiblings_;
    double lastX_ = 0.0;
    double lastY_ = 0.0;

    bool _ini = false;
};

#endif // ! OURPAINT_RENDERER_VIEWPORT_CONTROLLER_H_
