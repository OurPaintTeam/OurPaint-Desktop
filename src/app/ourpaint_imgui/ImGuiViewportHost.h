#ifndef OURPAINT_APPLICATION_IMGUI_VIEWPORT_HOST_H_
#define OURPAINT_APPLICATION_IMGUI_VIEWPORT_HOST_H_
#include <vector>

#include "../editor/InputEvents.h"
#include "../platform/IViewportHost.h"
#include "Camera2D.h"
#include "OpenGLRenderTarget.h"

// Adapts an ImGui image region to the existing controller API. Owns its FBO but
// only borrows controllers. No GLFW window or OpenGL context is created here.
class ImGuiViewportHost final : public IViewportHost {
public:
    explicit ImGuiViewportHost(uint32_t id = 1);
    uint32_t id() const override;
    Viewport2D getSize() const override;
    float getDevicePixelRatio() const override;
    void requestRedraw() override;
    void setContinuousRedraw(bool enabled) override;
    int setEventSink(IViewportController* sink) override;
    void presentFrame() override;
    int addController(IViewportController* controller) override;
    void setActiveController(int index) override;
    bool removeController(IViewportController* controller) override;

    // Call inside RenderArea with the OpenGL context current. Reserves an interactive
    // image, routes local input, renders CAD into the FBO and queues its texture.
    void draw();

private:
    void dispatchInput(float originX, float originY, float imageWidth, float imageHeight, bool hovered);

    uint32_t id_;
    Viewport2D size_;
    render::OpenGLRenderTarget target_;
    std::vector<IViewportController*> controllers_;
    IViewportController* controller_ = nullptr;
    input::MouseButton capturedButtons_ = input::MouseButton::None;
    double lastMouseX_ = 0.0;
    double lastMouseY_ = 0.0;
    input::Modifiers lastModifiers_ = input::Modifiers::None;
    bool wasHovered_ = false;
    bool needsResize_ = true;
};

#endif  // ! OURPAINT_APPLICATION_IMGUI_VIEWPORT_HOST_H_
