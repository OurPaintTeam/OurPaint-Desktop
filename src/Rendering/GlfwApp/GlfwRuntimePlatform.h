#ifndef OURPAINT_APPLICATION_GLFW_RUNTIME_PLATFORM_H_
#define OURPAINT_APPLICATION_GLFW_RUNTIME_PLATFORM_H_

#include "IPlatformRuntime.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>

class GlfwViewportHost;

class GlfwRuntimePlatform : public IPlatformRuntime {
public:
    explicit GlfwRuntimePlatform(int width = 800, int height = 600);
    ~GlfwRuntimePlatform() override;

    bool init() override;
    int run() override;

    IViewportHost* createViewportHost() override;
    void destroyViewportHost(IViewportHost* viewportHost) override;

private:
    int windowWidth_;
    int windowHeight_;

    GLFWwindow* window_ = nullptr;

    std::vector<GlfwViewportHost*> hosts_;
    uint32_t nextHostId_ = 1;
};

#endif // ! OURPAINT_APPLICATION_GLFW_RUNTIME_PLATFORM_H_