#ifndef OURPAINT_RENDERING_QT_VIEWPORT_HOST_H_
#define OURPAINT_RENDERING_QT_VIEWPORT_HOST_H_

#include <glad/gl.h>

#include <QOpenGLContext>
#include <QWindow>

#include "IRenderer.h"
#include "IViewportHost.h"
#include "InputEvents.h"
#include "Camera2D.h"

class QWidget;
class QTimer;

class QtViewportHost : public QWindow, public IViewportHost {
public:
    explicit QtViewportHost(uint32_t id, QScreen* screen = nullptr);
    ~QtViewportHost() override;

    uint32_t         id()                   const override;
    Viewport2D       getSize()              const override;
    float            getDevicePixelRatio()  const override;

    void requestRedraw()               override;
    void setContinuousRedraw(bool on)  override;
    void presentFrame()                override;
    void setEventSink(IViewportController* controller) override;

    // Для встраивания в Qt layout
    QWidget* createContainer(QWidget* parent = nullptr);

    // Для shared context между несколькими viewport-ами
    QOpenGLContext* context() const { return context_; }

protected:
    // QWindow events
    void exposeEvent            (QExposeEvent   *ev) override;
    void paintEvent             (QPaintEvent    *ev) override;
    void resizeEvent            (QResizeEvent   *ev) override;

    void keyPressEvent          (QKeyEvent      *ev) override;
    void keyReleaseEvent        (QKeyEvent      *ev) override;

    void mouseMoveEvent         (QMouseEvent    *ev) override;
    void mousePressEvent        (QMouseEvent    *ev) override;
    void mouseReleaseEvent      (QMouseEvent    *ev) override;
    void wheelEvent             (QWheelEvent    *ev) override;

    // void focusOutEvent         (QFocusEvent  *ev) override;
    // void mouseDoubleClickEvent (QMouseEvent  *ev) override;

private:
    void initContext();
    void renderFrame();

    static input::MouseButton      toMouseButtonEnum   (Qt::MouseButtons button);
    static input::Modifiers        toModifiersEnum     (Qt::KeyboardModifiers mods);
    static input::KeyCode          toKeyCodeEnum       (Qt::Key key);

    static input::MouseMoveEvent   toMouseMoveEvent    (QMouseEvent* e);
    static input::MouseButtonEvent toMouseButtonEvent  (QMouseEvent* e, input::MouseButtonAction action);
    static input::WheelEvent       toWheelEvent        (QWheelEvent* e);
    static input::KeyEvent         toKeyEvent          (QKeyEvent* e, input::KeyAction action);
    static input::ResizeEvent      toResizeEvent       (QResizeEvent* e, float dpr);

private:
    uint32_t             id_;
    QOpenGLContext*      context_          = nullptr;
    IViewportController* controller_       = nullptr;
    QTimer*              continuousTimer_  = nullptr;
    bool                 initialized_      = false;
};

#endif // ! OURPAINT_RENDERING_QT_VIEWPORT_HOST_H_