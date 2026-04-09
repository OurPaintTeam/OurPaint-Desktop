#include "Camera2D.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

void Camera2D::setViewport(int width, int height, double devicePixelRatio) {
    viewportW_ = std::max(1, width);
    viewportH_ = std::max(1, height);
    dpr_ = std::max(1e-9, devicePixelRatio);
}

void Camera2D::setCenter(const glm::dvec2& center) {
    center_ = center;
}

void Camera2D::setZoom(double zoom) {
    zoom_ = std::clamp(zoom, kMinZoom, kMaxZoom);
}

void Camera2D::panScreen(double dx, double dy) {
    center_.x -= dx / zoom_;
    center_.y += dy / zoom_;
}

void Camera2D::zoomAtScreen(double factor, const glm::dvec2& screenPoint) {
    if (factor <= 0.0) {
        return;
    }

    const glm::dvec2 before = screenToWorld(screenPoint);

    zoom_ = std::clamp(zoom_ * factor, kMinZoom, kMaxZoom);

    const glm::dvec2 after = screenToWorld(screenPoint);

    center_ += (before - after);
}

glm::dvec2 Camera2D::screenToWorld(const glm::dvec2& p) const {
    const double halfW = 0.5 * static_cast<double>(viewportW_);
    const double halfH = 0.5 * static_cast<double>(viewportH_);
    return {
        center_.x + (p.x - halfW) / zoom_,
        center_.y - (p.y - halfH) / zoom_
    };
}

glm::dvec2 Camera2D::worldToScreen(const glm::dvec2& p) const {
    const double halfW = 0.5 * static_cast<double>(viewportW_);
    const double halfH = 0.5 * static_cast<double>(viewportH_);

    return {
        (p.x - center_.x) * zoom_ + halfW,
        (center_.y - p.y) * zoom_ + halfH
    };
}

glm::mat4 Camera2D::viewMatrix() const {
    const float halfW = 0.5f * static_cast<float>(viewportW_);
    const float halfH = 0.5f * static_cast<float>(viewportH_);

    glm::mat4 m(1.0f);

    m = glm::translate(m, glm::vec3(halfW, halfH, 0.0f));
    m = glm::scale(m, glm::vec3(static_cast<float>(zoom_),
                                static_cast<float>(-zoom_),
                                1.0f));
    m = glm::translate(m, glm::vec3(static_cast<float>(-center_.x),
                                    static_cast<float>(-center_.y),
                                    0.0f));

    return m;
}

glm::mat4 Camera2D::projectionMatrix() const {
    return glm::ortho(
        0.0f, static_cast<float>(viewportW_),
        static_cast<float>(viewportH_), 0.0f,
        -1.0f, 1.0f
    );
}

glm::mat4 Camera2D::viewProjectionMatrix() const {
    return projectionMatrix() * viewMatrix();
}




