#include "Camera2D.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

void Camera2D::setViewport(int width, int height, double devicePixelRatio) {
    viewport_.logicalWidth = std::max(1, width);
    viewport_.logicalHeight = std::max(1, height);
    viewport_.devicePixelRatio = std::max(1e-9, devicePixelRatio);
}

void Camera2D::setCenter(const glm::dvec2& center) {
    center_ = center;
}

glm::dvec2 Camera2D::centerScreenLogical() const {
    return worldToScreenLogical(center_);
}

glm::dvec2 Camera2D::centerScreenFramebuffer() const {
    return worldToScreenFramebuffer(center_);
}

void Camera2D::setZoom(double zoom) {
    zoom_ = std::clamp(zoom, kMinZoom, kMaxZoom);
}

int Camera2D::wFramebuffer() const {
    return viewport_.framebufferWidth();
}

int Camera2D::hFramebuffer() const {
    return viewport_.framebufferHeight();
}

void Camera2D::panScreenLogical(double dx, double dy) {
    center_.x -= dx / zoom_;
    center_.y += dy / zoom_;
}

void Camera2D::panScreenFramebuffer(double dx, double dy) {
    panScreenLogical(dx / viewport_.devicePixelRatio,
                     dy / viewport_.devicePixelRatio);
}

void Camera2D::zoomAtScreenLogical(double factor, glm::dvec2 screenPointLogical) {
    if (factor <= 0.0) {
        return;
    }
    setZoomAtScreenLogical(zoom_ * factor, screenPointLogical);
}

void Camera2D::setZoomAtScreenLogical(double newZoom, glm::dvec2 screenPointLogical) {
    const glm::dvec2 before = screenLogicalToWorld(screenPointLogical);
    zoom_ = std::clamp(newZoom, kMinZoom, kMaxZoom);
    const glm::dvec2 after = screenLogicalToWorld(screenPointLogical);

    center_ += (before - after);
}

void Camera2D::zoomAtScreenFramebuffer(double factor, glm::dvec2 screenPointFramebuffer) {
    if (factor <= 0.0) {
        return;
    }
    setZoomAtScreenFramebuffer(zoom_ * factor, screenPointFramebuffer);
}

void Camera2D::setZoomAtScreenFramebuffer(double newZoom, glm::dvec2 screenPointFramebuffer) {
    const glm::dvec2 before = screenFramebufferToWorld(screenPointFramebuffer);
    zoom_ = std::clamp(newZoom, kMinZoom, kMaxZoom);
    const glm::dvec2 after = screenFramebufferToWorld(screenPointFramebuffer);

    center_ += (before - after);
}

glm::dvec2 Camera2D::screenLogicalToWorld(glm::dvec2 p) const {
    const double halfW = 0.5 * static_cast<double>(viewport_.logicalWidth);
    const double halfH = 0.5 * static_cast<double>(viewport_.logicalHeight);
    return {
        center_.x + (p.x - halfW) / zoom_,
        center_.y - (p.y - halfH) / zoom_
    };
}

glm::dvec2 Camera2D::worldToScreenLogical(glm::dvec2 p) const {
    const double halfW = 0.5 * static_cast<double>(viewport_.logicalWidth);
    const double halfH = 0.5 * static_cast<double>(viewport_.logicalHeight);

    return {
        (p.x - center_.x) * zoom_ + halfW,
        (center_.y - p.y) * zoom_ + halfH
    };
}

glm::dvec2 Camera2D::screenFramebufferToWorld(glm::dvec2 p) const {
    return screenLogicalToWorld(p / viewport_.devicePixelRatio);
}

glm::dvec2 Camera2D::worldToScreenFramebuffer(glm::dvec2 p) const {
    return worldToScreenLogical(p) * viewport_.devicePixelRatio;
}

double Camera2D::screenLogicalToWorld(double pixels) const {
    return pixels / zoom_;
}

double Camera2D::worldToScreenLogical(double worldUnit) const {
    return worldUnit * zoom_;
}

double Camera2D::screenFramebufferToWorld(double pixels) const {
    return pixels / (zoom_ * viewport_.devicePixelRatio);
}

double Camera2D::worldToScreenFramebuffer(double worldUnit) const {
    return worldUnit * zoom_ * viewport_.devicePixelRatio;
}

glm::mat4 Camera2D::viewMatrix() const {
    const float halfW = 0.5f * static_cast<float>(viewport_.logicalWidth);
    const float halfH = 0.5f * static_cast<float>(viewport_.logicalHeight);

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
        0.0f, static_cast<float>(viewport_.logicalWidth),
        static_cast<float>(viewport_.logicalHeight), 0.0f,
        -1.0f, 1.0f
    );
}

glm::mat4 Camera2D::viewProjectionMatrix() const {
    return projectionMatrix() * viewMatrix();
}

glm::dvec2 Camera2D::visibleMinWorld() const {
    // The minimum visible world coordinate is the center minus half the view size
    const double halfWidthWorld = 0.5 * static_cast<double>(viewport_.logicalWidth) / zoom_;
    const double halfHeightWorld = 0.5 * static_cast<double>(viewport_.logicalHeight) / zoom_;

    return {
        center_.x - halfWidthWorld,
        center_.y - halfHeightWorld
    };
}

glm::dvec2 Camera2D::visibleMaxWorld() const {
    // The maximum visible world coordinate is the center plus half the view size
    const double halfWidthWorld = 0.5 * static_cast<double>(viewport_.logicalWidth) / zoom_;
    const double halfHeightWorld = 0.5 * static_cast<double>(viewport_.logicalHeight) / zoom_;

    return {
        center_.x + halfWidthWorld,
        center_.y + halfHeightWorld
    };
}

RectD Camera2D::visibleWorldRect() const {
    return RectD{visibleMinWorld(), visibleMaxWorld()};
}



