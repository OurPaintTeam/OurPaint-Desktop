#pragma once

#include <glad/gl.h>  // Must precede Qt OpenGL headers.

#include <QOpenGLWindow>
#include <functional>
#include <set>

namespace app {
class ViewportController;
}

// Qt owns the native surface/context; App owns the controllers. No document or
// editor internals cross this adapter. Coordinates remain logical pixels.
class QtViewport final : public QOpenGLWindow {
public:
    QtViewport();
    ~QtViewport() override;
    void showViewport(app::ViewportController* controller);
    void releaseViewport(app::ViewportController* controller);
    void releaseAll();
    std::function<void()> activated;

protected:
    void initializeGL() override;
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;

private:
    void mouseButton(QMouseEvent* event, bool pressed);
    void key(QKeyEvent* event, bool pressed);
    app::ViewportController* current_ = nullptr;
    // Inactive tabs can still own GPU objects in this context.
    std::set<app::ViewportController*> residents_;
    bool glReady_ = false;
};
