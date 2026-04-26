#define GLAD_GL_NO_INCLUDE_H
#include "QtViewportHost.h"

#include <glad/gl.h>

#include <QDebug>
#include <QKeyEvent>
#include <QTimer>
#include <QWidget>
#include <QSurface>

#include "Logger.h"
#include "IViewportController.h"

QtViewportHost::QtViewportHost(uint32_t id, QScreen* screen)
    : QWindow(screen)
    , id_(id)
{
    setSurfaceType(QWindow::OpenGLSurface);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    fmt.setSwapInterval(0); // 1 V-Sync on, 0 V-Sync off.
    setFormat(fmt);

    continuousTimer_ = new QTimer(this);
    continuousTimer_->setInterval(16);

    QObject::connect(continuousTimer_, &QTimer::timeout,
                     [this]() { requestRedraw(); });


    // Без InitContext, т.к. когда окно реально покажется будет вызван exposeEvent.
}

QtViewportHost::~QtViewportHost() {
    continuousTimer_->stop();
    if (context_) {
        context_->doneCurrent();
        delete context_;
        context_ = nullptr;
    }
}

// --- IViewportHost ---

uint32_t QtViewportHost::id() const { return id_; }

ViewportSize QtViewportHost::getSize() const {
    int w = static_cast<int>(QWindow::width() * devicePixelRatio());
    int h = static_cast<int>(QWindow::height() * devicePixelRatio());
    return { w, h };
}

float QtViewportHost::getDevicePixelRatio() const {
    return static_cast<float>(QWindow::devicePixelRatio());
}

RenderTargetDesc QtViewportHost::renderTargetDesc() const {
    return {};
}

void QtViewportHost::requestRedraw() {
    if (initialized_ && isExposed()) {
        renderFrame();
    }
}

void QtViewportHost::setContinuousRedraw(bool on) {
    on ? continuousTimer_->start() : continuousTimer_->stop();
}

void QtViewportHost::presentFrame() {
    if (context_) {
        context_->swapBuffers(this);
    }
}

void QtViewportHost::setEventSink(IViewportController* controller) {
    controller_ = controller;
}



QWidget* QtViewportHost::createContainer(QWidget* parent) {
    auto* container = QWidget::createWindowContainer(this, parent);
    container->setFocusPolicy(Qt::StrongFocus);
    container->setAttribute(Qt::WA_MouseTracking);
    return container;
}




// --- QWindow events ---

void QtViewportHost::exposeEvent(QExposeEvent*) {
    if (!isExposed()) {
        return;
    }
    if (!initialized_) {
        initContext();
    }
    renderFrame();
}

void QtViewportHost::paintEvent(QPaintEvent* ev) {
    QWindow::paintEvent(ev);
}

void QtViewportHost::resizeEvent(QResizeEvent* e) {
    if (!initialized_) {
        initContext();
    }
    if (controller_) {
        controller_->onResize(toResizeEvent(e, static_cast<float>(devicePixelRatio())));
        while (controller_->dirty_ || controller_->continuousRedraw_) {
            renderFrame();
            controller_->dirty_ = false;
        }
    }
}


void QtViewportHost::keyPressEvent(QKeyEvent* e) {
    if (!initialized_) {
        initContext();
    }
    if (controller_) {
        controller_->onKey(toKeyEvent(e, input::KeyAction::Press));
        while (controller_->dirty_ || controller_->continuousRedraw_) {
            renderFrame();
            controller_->dirty_ = false;
        }
    }
}

void QtViewportHost::keyReleaseEvent(QKeyEvent* e) {
    if (!initialized_) {
        initContext();
    }
    if (controller_) {
        controller_->onKey(toKeyEvent(e, input::KeyAction::Release));
        while (controller_->dirty_ || controller_->continuousRedraw_) {
            renderFrame();
            controller_->dirty_ = false;
        }
    }
}

void QtViewportHost::mouseMoveEvent(QMouseEvent* e) {
    if (!initialized_) {
        initContext();
    }
    if (controller_) {
        controller_->onMouseMove(toMouseMoveEvent(e));
        while (controller_->dirty_ || controller_->continuousRedraw_) {
            renderFrame();
            controller_->dirty_ = false;
        }
    }
}

void QtViewportHost::mousePressEvent(QMouseEvent* e) {
    if (!initialized_) {
        initContext();
    }
    if (controller_) {
        controller_->onMouseButton(toMouseButtonEvent(e, input::MouseButtonAction::Press));
        while (controller_->dirty_ || controller_->continuousRedraw_) {
            renderFrame();
            controller_->dirty_ = false;
        }
    }
}

void QtViewportHost::mouseReleaseEvent(QMouseEvent* e) {
    if (!initialized_) {
        initContext();
    }
    if (controller_) {
        controller_->onMouseButton(toMouseButtonEvent(e, input::MouseButtonAction::Release));
        while (controller_->dirty_ || controller_->continuousRedraw_) {
            renderFrame();
            controller_->dirty_ = false;
        }
    }
}

void QtViewportHost::wheelEvent(QWheelEvent* e) {
    if (!initialized_) {
        initContext();
    }
    if (controller_) {
        controller_->onWheel(toWheelEvent(e));
        while (controller_->dirty_ || controller_->continuousRedraw_) {
            renderFrame();
            controller_->dirty_ = false;
        }
    }
}





// --- Private ---

static void* getGLProcAddress(const char* name) {
    QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if (!ctx) {
        qWarning() << "No current OpenGL context when loading:" << name;
        return nullptr;
    }
    return reinterpret_cast<void*>(ctx->getProcAddress(name));
}

void QtViewportHost::initContext() {
    qDebug() << "initContext() started";

    context_ = new QOpenGLContext();
    context_->setFormat(requestedFormat());

    if (!context_->create()) {
        qWarning() << "Failed to create OpenGL context";
        delete context_;
        context_ = nullptr;
        return;
    }

    qDebug() << "Context created, making current...";

    if (!context_->makeCurrent(this)) {
        qWarning() << "Failed to make context current";
        delete context_;
        context_ = nullptr;
        return;
    }

    qDebug() << "Context is current, loading GLAD...";

    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(getGLProcAddress))) {
        qWarning() << "Failed to initialize GLAD";
        context_->doneCurrent();
        delete context_;
        context_ = nullptr;
        return;
    }

    if (!glad_glClear) {
        qWarning() << "glClear not loaded!";
        return;
    }

    initialized_ = true;

    //renderer_->initialize();

    const int w = int(width() * devicePixelRatio());
    const int h = int(height() * devicePixelRatio());

    glViewport(0, 0, w, h);

    auto s = size();
    if (controller_) {
        input::ResizeEvent e;
        e.width = w;
        e.height = h;
        e.devicePixelRatio = devicePixelRatio();
        controller_->onResize(e);
    }

    qDebug() << "initContext() finished successfully";
}

void QtViewportHost::renderFrame() {
    if (!context_ || !initialized_) {
        return;
    }

    context_->makeCurrent(this);

    if (controller_) {
        controller_->render();
    }

    presentFrame();
}

input::MouseButton QtViewportHost::toMouseButtonEnum(Qt::MouseButtons bs) {
    input::MouseButton me = input::MouseButton::None;
    if (bs & Qt::LeftButton) {
        me = me | input::MouseButton::Left;
    }
    if (bs & Qt::MiddleButton) {
        me = me | input::MouseButton::Middle;
    }
    if (bs & Qt::RightButton) {
        me = me | input::MouseButton::Right;
    }
    return me;
}

input::KeyCode QtViewportHost::toKeyCodeEnum(Qt::Key key) {
    input::KeyCode kc;
    switch (key) {
        case Qt::Key_Escape:    kc = input::KeyCode::Escape;    break;
        case Qt::Key_Tab:       kc = input::KeyCode::Tab;       break;
        case Qt::Key_Enter:     kc = input::KeyCode::Enter;     break;
        case Qt::Key_Space:     kc = input::KeyCode::Space;     break;
        case Qt::Key_Backspace: kc = input::KeyCode::Backspace; break;
        case Qt::Key_Delete:    kc = input::KeyCode::Delete;    break;
        case Qt::Key_Insert:    kc = input::KeyCode::Insert;    break;
        case Qt::Key_Home:      kc = input::KeyCode::Home;      break;
        case Qt::Key_End:       kc = input::KeyCode::End;       break;
        case Qt::Key_PageUp:    kc = input::KeyCode::PageUp;    break;
        case Qt::Key_PageDown:  kc = input::KeyCode::PageDown;  break;
        case Qt::Key_Left:      kc = input::KeyCode::Left;      break;
        case Qt::Key_Right:     kc = input::KeyCode::Right;     break;
        case Qt::Key_Up:        kc = input::KeyCode::Up;        break;
        case Qt::Key_Down:      kc = input::KeyCode::Down;      break;
        case Qt::Key_A:         kc = input::KeyCode::A;         break; // 1
        case Qt::Key_B:         kc = input::KeyCode::B;         break; // 2
        case Qt::Key_C:         kc = input::KeyCode::C;         break; // 3
        case Qt::Key_D:         kc = input::KeyCode::D;         break; // 4
        case Qt::Key_E:         kc = input::KeyCode::E;         break; // 5
        case Qt::Key_F:         kc = input::KeyCode::F;         break; // 6
        case Qt::Key_G:         kc = input::KeyCode::G;         break; // 7
        case Qt::Key_H:         kc = input::KeyCode::H;         break; // 8
        case Qt::Key_I:         kc = input::KeyCode::I;         break; // 9
        case Qt::Key_J:         kc = input::KeyCode::J;         break; // 10
        case Qt::Key_K:         kc = input::KeyCode::K;         break; // 11
        case Qt::Key_L:         kc = input::KeyCode::L;         break; // 12
        case Qt::Key_M:         kc = input::KeyCode::M;         break; // 13
        case Qt::Key_N:         kc = input::KeyCode::N;         break; // 14
        case Qt::Key_O:         kc = input::KeyCode::O;         break; // 15
        case Qt::Key_P:         kc = input::KeyCode::P;         break; // 16
        case Qt::Key_Q:         kc = input::KeyCode::Q;         break; // 17
        case Qt::Key_R:         kc = input::KeyCode::R;         break; // 18
        case Qt::Key_S:         kc = input::KeyCode::S;         break; // 19
        case Qt::Key_T:         kc = input::KeyCode::T;         break; // 20
        case Qt::Key_U:         kc = input::KeyCode::U;         break; // 21
        case Qt::Key_V:         kc = input::KeyCode::V;         break; // 22
        case Qt::Key_W:         kc = input::KeyCode::W;         break; // 23
        case Qt::Key_X:         kc = input::KeyCode::X;         break; // 24
        case Qt::Key_Y:         kc = input::KeyCode::Y;         break; // 25
        case Qt::Key_Z:         kc = input::KeyCode::Z;         break; // 26
        case Qt::Key_0:         kc = input::KeyCode::Num0;      break;
        case Qt::Key_1:         kc = input::KeyCode::Num1;      break;
        case Qt::Key_2:         kc = input::KeyCode::Num2;      break;
        case Qt::Key_3:         kc = input::KeyCode::Num3;      break;
        case Qt::Key_4:         kc = input::KeyCode::Num4;      break;
        case Qt::Key_5:         kc = input::KeyCode::Num5;      break;
        case Qt::Key_6:         kc = input::KeyCode::Num6;      break;
        case Qt::Key_7:         kc = input::KeyCode::Num7;      break;
        case Qt::Key_8:         kc = input::KeyCode::Num8;      break;
        case Qt::Key_9:         kc = input::KeyCode::Num9;      break;
        case Qt::Key_Comma:     kc = input::KeyCode::Comma;     break;
        case Qt::Key_Period:    kc = input::KeyCode::Period;    break;
        case Qt::Key_Backslash: kc = input::KeyCode::Backslash; break;
        case Qt::Key_BracketLeft: kc=input::KeyCode::LeftBracket;break;
        case Qt::Key_Minus:     kc = input::KeyCode::Minus;     break;
        case Qt::Key_Plus:      kc = input::KeyCode::Plus;      break;
        case Qt::Key_Asterisk:  kc = input::KeyCode::Asterisk;  break;
        case Qt::Key_F1:        kc = input::KeyCode::F1;        break;
        case Qt::Key_F2:        kc = input::KeyCode::F2;        break;
        case Qt::Key_F3:        kc = input::KeyCode::F3;        break;
        case Qt::Key_F4:        kc = input::KeyCode::F4;        break;
        case Qt::Key_F5:        kc = input::KeyCode::F5;        break;
        case Qt::Key_F6:        kc = input::KeyCode::F6;        break;
        case Qt::Key_F7:        kc = input::KeyCode::F7;        break;
        case Qt::Key_F8:        kc = input::KeyCode::F8;        break;
        case Qt::Key_F9:        kc = input::KeyCode::F9;        break;
        case Qt::Key_F10:       kc = input::KeyCode::F10;       break;
        case Qt::Key_F11:       kc = input::KeyCode::F11;       break;
        case Qt::Key_F12:       kc = input::KeyCode::F12;       break;
            default: kc = input::KeyCode::Unknown;
    }
    return kc;
}

input::Modifiers QtViewportHost::toModifiersEnum(Qt::KeyboardModifiers mods) {
    input::Modifiers m = input::Modifiers::None;
    if (mods & Qt::ShiftModifier) {
        m = m | input::Modifiers::Shift;
    }
    if (mods & Qt::ControlModifier) {
        m = m | input::Modifiers::Ctrl;
    }
    if (mods & Qt::AltModifier) {
        m = m | input::Modifiers::Alt;
    }
    if (mods & Qt::MetaModifier) {
        m = m | input::Modifiers::Meta;
    }

    return m;
}


input::MouseMoveEvent QtViewportHost::toMouseMoveEvent(QMouseEvent* e) {
    input::MouseMoveEvent me;
    me.x = e->position().x();
    me.y = e->position().y();
    me.buttons = toMouseButtonEnum(e->buttons());
    me.modifiers = toModifiersEnum(e->modifiers());
    me.timestampMs = e->timestamp();

    return me;
}

input::MouseButtonEvent QtViewportHost::toMouseButtonEvent(QMouseEvent* e, input::MouseButtonAction action) {
    input::MouseButtonEvent me;
    me.x = e->position().x();
    me.y = e->position().y();
    me.button = toMouseButtonEnum(e->button());
    me.buttons = toMouseButtonEnum(e->buttons());
    me.action = action;
    me.modifiers = toModifiersEnum(e->modifiers());
    me.timestampMs = e->timestamp();

    return me;
}

input::WheelEvent QtViewportHost::toWheelEvent(QWheelEvent* e) {
    input::WheelEvent we;
    we.x = e->position().x();
    we.y = e->position().y();
    we.modifiers = toModifiersEnum(e->modifiers());
    we.timestampMs = e->timestamp();

    if (!e->pixelDelta().isNull()) {
        we.deltaX = e->pixelDelta().x();
        we.deltaY = e->pixelDelta().y();
        we.isPixelPrecise = true;
    } else {
        we.deltaX = e->angleDelta().x() / 120.0;
        we.deltaY = e->angleDelta().y() / 120.0;
        we.isPixelPrecise = false;
    }

    return we;
}

input::KeyEvent QtViewportHost::toKeyEvent(QKeyEvent* e, input::KeyAction action) {
    input::KeyEvent ke;
    ke.key = toKeyCodeEnum(static_cast<Qt::Key>(e->key()));
    ke.action = action;
    ke.modifiers = toModifiersEnum(e->modifiers());
    ke.nativeScanCode = e->nativeScanCode();
    ke.isAutoRepeat = e->isAutoRepeat();
    ke.timestampMs = e->timestamp();

    return ke;
}

input::ResizeEvent QtViewportHost::toResizeEvent(QResizeEvent* e, float dpr) {
    input::ResizeEvent re;
    re.width = e->size().width();
    re.height = e->size().height();
    re.devicePixelRatio = dpr;

    return re;
}






