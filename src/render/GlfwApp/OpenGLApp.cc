#include "OpenGLApp.h"

#include "OpenGLRenderer.h"

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

OpenGLApp::OpenGLApp() :    dirty(false),
                            window(nullptr),
                            host(nullptr),
                            controller(nullptr),
                            renderer(nullptr),
                            scene(nullptr) {
    mLog("[OpenGLApp] Init start;");

    // Init GLFW
    if (!glfwInit()) {
        std::cout << "Failed to init GLFW\n";
        return;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(800, 600, "OpenGL Sandbox", nullptr, nullptr);
    if (!window) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cout << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return;
    }

    // Create glfw adapter
    host = new GlfwViewportHost(window);
    renderer = new OpenGLRenderer();
    scene = new RenderScene();
    view = new View2D();
    controller = new ViewportController(*host, *renderer, *scene, *view);
    controller->initialize();
    host->setEventSink(controller);
    mLog("[OpenGLApp] Init end;");
}

int OpenGLApp::exec() {
    mLog("[OpenGLApp] exec start;");
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (host->continuousRedrawEnabled() || host->consumeRedrawRequest()) {
            mLog("Host give true");

            controller->onFrame();
            host->swapBuffers();
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    delete host;
    delete renderer;
    delete scene;
    delete view;
    delete controller;

    mLog("[OpenGLApp] exec end;");
    return 0;
}


