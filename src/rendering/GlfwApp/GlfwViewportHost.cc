#include "GlfwViewportHost.h"
#include "Camera2D.h"

GlfwViewportHost::GlfwViewportHost(GLFWwindow* window) : window_(window) {
    mLog("[GlfwViewportHost] Init start;");
    if (!window_) {
        // "GlfwViewportHost: window must not be null";
    }

    glfwSetWindowUserPointer(window_, this);
    glfwMakeContextCurrent(window_);

    glfwSetFramebufferSizeCallback(window_, &GlfwViewportHost::framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, &GlfwViewportHost::cursorPosCallback);
    glfwSetMouseButtonCallback(window_, &GlfwViewportHost::mouseButtonCallback);
    glfwSetScrollCallback(window_, &GlfwViewportHost::scrollCallback);
    glfwSetKeyCallback(window_, &GlfwViewportHost::keyCallback);
    glfwSetWindowRefreshCallback(window_, &GlfwViewportHost::windowRefreshCallback);

    mLog("[GlfwViewportHost] Init end;");
}

GlfwViewportHost::~GlfwViewportHost() {
    glfwDestroyWindow(window_);
}

GlfwViewportHost* GlfwViewportHost::fromWindow(GLFWwindow* window) {
    return static_cast<GlfwViewportHost*>(glfwGetWindowUserPointer(window));
}

uint32_t GlfwViewportHost::id() const {
    return id_;
}

ViewportSize GlfwViewportHost::getSize() const {
    ViewportSize size;
    glfwGetFramebufferSize(window_, &size.width, &size.height);
    return size;
}

float GlfwViewportHost::getDevicePixelRatio() const {
    int windowW = 0;
    int windowH = 0;
    int fbW = 0;
    int fbH = 0;

    glfwGetWindowSize(window_, &windowW, &windowH);
    glfwGetFramebufferSize(window_, &fbW, &fbH);

    if (windowW <= 0 || windowH <= 0) {
        return 1.0f;
    }

    const float dprX = static_cast<float>(fbW) / static_cast<float>(windowW);
    const float dprY = static_cast<float>(fbH) / static_cast<float>(windowH);

    return (dprX * dprY) * 0.5f;
}

void GlfwViewportHost::requestRedraw() {
    redrawRequested_ = true;
}

void GlfwViewportHost::setContinuousRedraw(bool enabled) {
    continuousRedraw_ = enabled;
}

void GlfwViewportHost::setEventSink(IViewportController* sink) {
    sink_ = sink;
}

RenderTargetDesc GlfwViewportHost::renderTargetDesc() const {
    //auto s = size();
    //return { s.width, s.height, devicePixelRatio(), RenderBackend::OpenGL };
    return {};
}

void GlfwViewportHost::presentFrame() {
    glfwSwapBuffers(window_);
}

bool GlfwViewportHost::needsFrame() const {
    return continuousRedraw_ || redrawRequested_;
}

bool GlfwViewportHost::closeRequested() const {
    return glfwWindowShouldClose(window_);
}

void GlfwViewportHost::dispatchFrame() {
    redrawRequested_ = false;
    glfwMakeContextCurrent(window_);  // важно при нескольких окнах
    if (sink_) {
        //sink_->onFrame(this);
    }
}

// ------------------------------------  Callbacks

void GlfwViewportHost::framebufferSizeCallback(GLFWwindow* window, int, int) {
    //mLog("[GlfwViewportHost::framebufferSizeCallback] callback;");
    auto* self = fromWindow(window);
    if (!self) {
        return;
    }

    self->redrawRequested_ = true;

    //ViewportSize size = self->size();
    if (self->sink_) {
        //self->sink_->onResize(size.width, size.height, 0.0f);
    }
}

void GlfwViewportHost::cursorPosCallback(GLFWwindow* window, double x, double y) {
    //mLog("[GlfwViewportHost::cursorPosCallback] callback " + std::to_string(x) + " " + std::to_string(y));
    auto* self = fromWindow(window);
    if (!self) {
        return;
    }

    //mLog(std::to_string(x) + " " + std::to_string(y));

    self->lastCursorX_ = x;
    self->lastCursorY_ = y;
    //self->sink_->onMouseMove(x, y);
}

/*MouseEvent GlfwViewportHost::GlfwControllerInputMapper(int button, int action, int mods) {
    MouseEvent e;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        e.button = MouseButton::Left;
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        e.button = MouseButton::Right;
    }
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        e.button = MouseButton::Middle;
    }

    if (action == GLFW_PRESS) {
        e.action = ButtonAction::Press;
    }
    else if (action == GLFW_RELEASE) {
        e.action = ButtonAction::Release;
    }

    if (mods == GLFW_MOD_SHIFT) {
        e.shift = true;
    }
    if (mods == GLFW_MOD_CONTROL) {
        e.ctrl = true;
    }
    if (mods == GLFW_MOD_ALT) {
        e.alt = true;
    }

    return e;
}*/

void GlfwViewportHost::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    mLog("[GlfwViewportHost::mouseButtonCallback] callback " + std::to_string(button) + " " + std::to_string(action) + " " + std::to_string(mods));
    auto* self = fromWindow(window);
    if (!self) {
        return;
    }

    //MouseEvent e = GlfwControllerInputMapper(button, action, mods);

    //self->sink_->onMouseButtonPressed(e);
}

void GlfwViewportHost::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    mLog("[GlfwViewportHost::scrollCallback] callback");
}

void GlfwViewportHost::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    mLog("[GlfwViewportHost::keyCallback] callback");
    auto* self = fromWindow(window);
    if (!self || !self->sink_) {
        return;
    }


    self->redrawRequested_ = true;
}

void GlfwViewportHost::windowRefreshCallback(GLFWwindow* window) {
    mLog("[GlfwViewportHost::windowRefreshCallback] callback");
    auto* self = fromWindow(window);
    if (!self) {
        return;
    }

    self->redrawRequested_ = true;
}


