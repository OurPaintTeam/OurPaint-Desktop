#ifndef OURPAINT_RENDERING_GLFW_VIEWPORT_HOST_H_
#define OURPAINT_RENDERING_GLFW_VIEWPORT_HOST_H_

#include <GLFW/glfw3.h>

#include "../../app/IViewportHost.h"
#include "../../app/InputEvents.h"
#include "Logger.h"

/*

*/

class GlfwViewportHost : public IViewportHost {
public:
    explicit GlfwViewportHost(GLFWwindow* window);
    ~GlfwViewportHost() override;

    uint32_t         id()               const override;
    ViewportSize     getSize()             const override;
    float            getDevicePixelRatio() const override;
    RenderTargetDesc renderTargetDesc() const override;

    void requestRedraw()                        override;
    void setContinuousRedraw(bool enabled)      override;
    void setEventSink(IViewportController* sink) override;
    void presentFrame()                         override;

    bool         needsFrame()      const;
    bool         closeRequested()  const;
    void         dispatchFrame();
    GLFWwindow*  glfwWindow()      const { return window_; }

    //bool needsFrame() const;
    //void dispatchFrame();

private:
    static GlfwViewportHost* fromWindow(GLFWwindow* window);

    //static MouseEvent GlfwControllerInputMapper(int button, int action, int mods);

    static void framebufferSizeCallback (GLFWwindow* window, int width, int height);
    static void cursorPosCallback       (GLFWwindow* window, double x, double y);
    static void mouseButtonCallback     (GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback          (GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback             (GLFWwindow* window, int key, int scancode, int action, int mods);
    static void windowRefreshCallback   (GLFWwindow* window);

private:
    GLFWwindow* window_ = nullptr;
    uint32_t id_ = 0;
    IViewportController* sink_ = nullptr;


    bool redrawRequested_ = false;
    bool continuousRedraw_ = false;

    double lastCursorX_ = 0.0;
    double lastCursorY_ = 0.0;
    bool hasLastCursorPos_ = false;
};

#endif // ! OURPAINT_RENDERING_GLFW_VIEWPORT_HOST_H_
