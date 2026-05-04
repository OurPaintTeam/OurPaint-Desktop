#include "GlfwRuntimePlatform.h"
#include "GlfwViewportHost.h"

GlfwRuntimePlatform::GlfwRuntimePlatform(int width, int height)
    : windowWidth_(width), windowHeight_(height) {}

GlfwRuntimePlatform::~GlfwRuntimePlatform() {
    for (auto* host : hosts_) {
        delete host;
    }
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate();
}

bool GlfwRuntimePlatform::init() {
    if (window_) {
        return true;
    }

    // Init GLFW
    if (!glfwInit()) {
        return false;
    }

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(windowWidth_, windowHeight_, "Viewport", nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        return false;
    }

    //host_ = new GlfwViewportHost(window_);

    return true;
}

int GlfwRuntimePlatform::run() {
    // if (!host_ || !window_) {
    //     return -1;
    // }
    //
    // while (!glfwWindowShouldClose(window_)) {
    //     glfwPollEvents();
    //
    //     if (host_->continuousRedrawEnabled() || host_->consumeRedrawRequest()) {
    //
    //     }
    // }
    //
    // return 0;

    return false;
}

IViewportHost* GlfwRuntimePlatform::createViewportHost() {
    // return host_;

    return nullptr;
}
void GlfwRuntimePlatform::destroyViewportHost(IViewportHost* viewportHost) {}



