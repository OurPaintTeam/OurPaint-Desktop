#ifndef OURPAINT_RENDERING_VIEW2D_H_
#define OURPAINT_RENDERING_VIEW2D_H_

#include <cstdint>
#include <glm/glm.hpp>

enum class RenderBackend {
    OpenGL,
    Vulkan,
};

struct ViewportSize {
    int width = 1;
    int height = 1;
};


struct RenderTargetDesc {
    int           width;
    int           height;
    float         devicePixelRatio;
    RenderBackend backend;

    // OpenGL: всегда 0 (default framebuffer)
    // Vulkan: зарезервировано, используй platformHandle
    uint32_t      framebufferId = 0;

    // Платформенный хэндл если нужен (HWND, ANativeWindow, etc.)
    // nullptr если не нужен на этой платформе
    void*         platformHandle = nullptr;
};

struct Vec2 {
    double x = 0.0;
    double y = 0.0;
};

struct Mat3 {
    double m[9] = {0};
};

class Camera2D {
public:
    void setViewport(int width, int height, double devicePixelRatio = 1.0);
    void setCenter(const glm::dvec2& center);
    void setZoom(double zoom);

    const glm::dvec2& center() const { return center_; }
    double zoom() const { return zoom_; }

    void panScreen(double dx, double dy);
    void zoomAtScreen(double factor, const glm::dvec2& screenPoint);

    glm::dvec2 screenToWorld(const glm::dvec2& p) const;
    glm::dvec2 worldToScreen(const glm::dvec2& p) const;

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;
    glm::mat4 viewProjectionMatrix() const;

    // glm::dvec2 viewportCenterScreen() const;
    // glm::dvec2 visibleMinWorld() const;
    // glm::dvec2 visibleMaxWorld() const;
    // void fitToBox(glm::dvec2 minP, glm::dvec2 maxP, double paddingPixels);

private:
    glm::dvec2 center_{0.0, 0.0};

    // pixels per world unit
    double zoom_ = 100.0;

    int viewportW_ = 1;
    int viewportH_ = 1;
    double dpr_ = 1.0;

    static constexpr double kMinZoom = 1e-6;
    static constexpr double kMaxZoom = 1e6;
};

#endif // ! OURPAINT_RENDERING_VIEW2D_H_