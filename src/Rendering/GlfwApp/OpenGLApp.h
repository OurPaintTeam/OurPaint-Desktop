#ifndef OURPAINT_MVK_OPENGLAPP_H_
#define OURPAINT_MVK_OPENGLAPP_H_

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <iostream>

#include "../../app/ViewportController.h"
#include "GlfwViewportHost.h"
#include "Logger.h"
#include "View2D.h"

class OpenGLApp {
public:
    OpenGLApp();
    int exec();

private:
    GLFWwindow* window = nullptr;

    GlfwViewportHost* host;
    ViewportController* controller;
    IRenderer* renderer;
    RenderScene* scene;
    View2D* view;

    bool dirty;
};

#endif // ! OURPAINT_MVK_OPENGLAPP_H_