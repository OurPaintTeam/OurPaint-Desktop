#ifndef OURPAINT_RENDERING_VIEW2D_H_
#define OURPAINT_RENDERING_VIEW2D_H_

#include <cstdint>
#include <glm/glm.hpp>
#include <cmath>

struct Viewport2D {
    int logicalWidth = 1;
    int logicalHeight = 1;
    double devicePixelRatio = 1.0;

    int framebufferWidth() const {
        return static_cast<int>(std::round(logicalWidth * devicePixelRatio));
    }

    int framebufferHeight() const {
        return static_cast<int>(std::round(logicalHeight * devicePixelRatio));
    }
};

struct RectD {
    glm::dvec2 min{0.0, 0.0};
    glm::dvec2 max{0.0, 0.0};

    double width() const { return max.x - min.x; }
    double height() const { return max.y - min.y; }
    glm::dvec2 center() const { return (min + max) * 0.5; }
};

class Camera2D {
public:
    void setViewport(int logicalWidth, int logicalHeight, double devicePixelRatio = 1.0);
    const Viewport2D& viewport() const { return viewport_; }

    void setCenter(const glm::dvec2& center);
    const glm::dvec2& centerWorld() const { return center_; }
    glm::dvec2 centerScreenLogical() const;
    glm::dvec2 centerScreenFramebuffer() const;

    void setZoom(double zoom);

    double zoom() const { return zoom_; }
    double minZoom() const { return kMinZoom; }
    double maxZoom() const { return kMaxZoom; }

    int wLogical() const { return viewport_.logicalWidth; }
    int hLogical() const { return viewport_.logicalHeight; }
    int wFramebuffer() const;
    int hFramebuffer() const;

    void panScreenLogical(double dx, double dy);
    void panScreenFramebuffer(double dx, double dy);

    void zoomAtScreenLogical(double factor, glm::dvec2 screenPointLogical);
    void setZoomAtScreenLogical(double newZoom, glm::dvec2 screenPointLogical);

    void zoomAtScreenFramebuffer(double factor, glm::dvec2 screenPointFramebuffer);
    void setZoomAtScreenFramebuffer(double newZoom, glm::dvec2 screenPointFramebuffer);

    glm::dvec2 screenLogicalToWorld(glm::dvec2 p) const;
    glm::dvec2 worldToScreenLogical(glm::dvec2 p) const;

    glm::dvec2 screenFramebufferToWorld(glm::dvec2 p) const;
    glm::dvec2 worldToScreenFramebuffer(glm::dvec2 p) const;

    double screenLogicalToWorld(double pixels) const;
    double worldToScreenLogical(double worldUnit) const;

    double screenFramebufferToWorld(double pixels) const;
    double worldToScreenFramebuffer(double worldUnit) const;

    // Matrices map world coordinates to logical screen coordinates, then to clip space.
    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;
    glm::mat4 viewProjectionMatrix() const;

    glm::dvec2 visibleMinWorld() const;
    glm::dvec2 visibleMaxWorld() const;

    RectD visibleWorldRect() const;

private:
    Viewport2D viewport_;

    glm::dvec2 center_{0.0, 0.0};

    // Logical pixels per world unit
    double zoom_ = 100.0;

    // World units
    static constexpr double kMinZoom = 1e-6;
    static constexpr double kMaxZoom = 1e6;
};

#endif // ! OURPAINT_RENDERING_VIEW2D_H_