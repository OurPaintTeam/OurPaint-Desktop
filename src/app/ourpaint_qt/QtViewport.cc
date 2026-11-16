#include "QtViewport.h"

#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QWheelEvent>

#include "ViewportController.h"

namespace {
input::Modifiers modifiers(Qt::KeyboardModifiers value) {
    auto result = input::Modifiers::None;
    if (value & Qt::ShiftModifier) {
        result = result | input::Modifiers::Shift;
    }
    if (value & Qt::ControlModifier) {
        result = result | input::Modifiers::Ctrl;
    }
    if (value & Qt::AltModifier) {
        result = result | input::Modifiers::Alt;
    }
    if (value & Qt::MetaModifier) {
        result = result | input::Modifiers::Meta;
    }
    return result;
}

input::MouseButton buttons(Qt::MouseButtons value) {
    auto result = input::MouseButton::None;
    if (value & Qt::LeftButton) {
        result = result | input::MouseButton::Left;
    }
    if (value & Qt::MiddleButton) {
        result = result | input::MouseButton::Middle;
    }
    if (value & Qt::RightButton) {
        result = result | input::MouseButton::Right;
    }
    return result;
}

input::KeyCode keyCode(int value) {
    using K = input::KeyCode;
    if (value >= Qt::Key_A && value <= Qt::Key_Z) {
        return K(unsigned(K::A) + value - Qt::Key_A);
    }
    if (value >= Qt::Key_0 && value <= Qt::Key_9) {
        return K(unsigned(K::Num0) + value - Qt::Key_0);
    }
    if (value >= Qt::Key_F1 && value <= Qt::Key_F12) {
        return K(unsigned(K::F1) + value - Qt::Key_F1);
    }
    switch (value) {
        case Qt::Key_Escape:
            return K::Escape;
        case Qt::Key_Tab:
            return K::Tab;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            return K::Enter;
        case Qt::Key_Space:
            return K::Space;
        case Qt::Key_Backspace:
            return K::Backspace;
        case Qt::Key_Delete:
            return K::Delete;
        case Qt::Key_Insert:
            return K::Insert;
        case Qt::Key_Home:
            return K::Home;
        case Qt::Key_End:
            return K::End;
        case Qt::Key_PageUp:
            return K::PageUp;
        case Qt::Key_PageDown:
            return K::PageDown;
        case Qt::Key_Left:
            return K::Left;
        case Qt::Key_Right:
            return K::Right;
        case Qt::Key_Up:
            return K::Up;
        case Qt::Key_Down:
            return K::Down;
        case Qt::Key_Comma:
            return K::Comma;
        case Qt::Key_Period:
            return K::Period;
        case Qt::Key_Slash:
            return K::Slash;
        case Qt::Key_Backslash:
            return K::Backslash;
        case Qt::Key_BracketLeft:
            return K::LeftBracket;
        case Qt::Key_BracketRight:
            return K::RightBracket;
        case Qt::Key_Minus:
            return K::Minus;
        case Qt::Key_Plus:
            return K::Plus;
        case Qt::Key_Asterisk:
            return K::Asterisk;
        default:
            return K::Unknown;
    }
}
}  // namespace

QtViewport::QtViewport() : QOpenGLWindow(NoPartialUpdate) {
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    setFormat(format);
    connect(this, &QOpenGLWindow::frameSwapped, this, [this] {
        if (current_ && current_->continuousRedraw_) {
            update();
        }
    });
}

QtViewport::~QtViewport() {
    releaseAll();
    if (context()) {
        disconnect(context(), nullptr, this, nullptr);
    }
}

void QtViewport::showViewport(app::ViewportController* controller) {
    current_ = controller;
    if (controller) {
        residents_.insert(controller);
    }
    update();
}

void QtViewport::releaseViewport(app::ViewportController* controller) {
    if (!controller) {
        return;
    }
    if (current_ == controller) {
        current_ = nullptr;
    }
    if (residents_.erase(controller) && glReady_) {
        makeCurrent();
        controller->shutdown();
        doneCurrent();
    }
    update();
}

void QtViewport::releaseAll() {
    if (glReady_) {
        makeCurrent();
        for (auto* controller : residents_) {
            controller->shutdown();
        }
        doneCurrent();
    }
    residents_.clear();
    current_ = nullptr;
}

void QtViewport::initializeGL() {
    glReady_ = gladLoadGL([](const char* name) -> GLADapiproc {
                   return reinterpret_cast<GLADapiproc>(QOpenGLContext::currentContext()->getProcAddress(name));
               }) != 0 &&
               GLAD_GL_VERSION_3_3;
    if (!glReady_) {
        qCritical("Could not load OpenGL 3.3 functions");
    }
    connect(
        context(), &QOpenGLContext::aboutToBeDestroyed, this,
        [this] {
            // Keep borrowed controllers attached: Qt may recreate the native context.
            if (glReady_) {
                makeCurrent();
                for (auto* controller : residents_) {
                    controller->shutdown();
                }
                doneCurrent();
            }
            glReady_ = false;
        },
        Qt::DirectConnection);
}

void QtViewport::paintGL() {
    if (!glReady_) {
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    if (!current_) {
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }
    // Resize may call GL. Do it here, with the correct context current, including
    // after a tab switch or a move between monitors with different DPR values.
    current_->onResize({width(), height(), float(devicePixelRatio())});
    current_->render();
    current_->dirty_ = false;
}

void QtViewport::mouseMoveEvent(QMouseEvent* event) {
    if (current_) {
        current_->onMouseMove({event->position().x(), event->position().y(), buttons(event->buttons()), modifiers(event->modifiers()), event->timestamp()});
    }
    update();
}

void QtViewport::mouseButton(QMouseEvent* event, bool pressed) {
    if (pressed) {
        requestActivate();
        if (activated) {
            activated();
        }
        setMouseGrabEnabled(true);
    }
    if (pressed && current_) {
        current_->onMouseMove(
            {event->position().x(), event->position().y(), buttons(event->buttons() & ~event->button()), modifiers(event->modifiers()), event->timestamp()});
    }
    if (current_) {
        current_->onMouseButton({event->position().x(), event->position().y(), buttons(event->button()),
                                 pressed ? input::MouseButtonAction::Press : input::MouseButtonAction::Release, buttons(event->buttons()),
                                 modifiers(event->modifiers()), event->timestamp()});
    }
    if (!pressed && event->buttons() == Qt::NoButton) {
        setMouseGrabEnabled(false);
    }
    update();
}

void QtViewport::mousePressEvent(QMouseEvent* event) { mouseButton(event, true); }
void QtViewport::mouseReleaseEvent(QMouseEvent* event) { mouseButton(event, false); }

void QtViewport::wheelEvent(QWheelEvent* event) {
    if (activated) {
        activated();
    }
    const bool precise = !event->pixelDelta().isNull();
    const QPointF delta = precise ? QPointF(event->pixelDelta()) : QPointF(event->angleDelta()) / 120.0;
    if (current_) {
        current_->onWheel({event->position().x(), event->position().y(), delta.x(), delta.y(), modifiers(event->modifiers()), event->timestamp(), precise});
    }
    update();
}

void QtViewport::key(QKeyEvent* event, bool pressed) {
    if (event->isAutoRepeat() && !pressed) {
        return;
    }
    if (current_) {
        current_->onKey({keyCode(event->key()),
                         pressed ? (event->isAutoRepeat() ? input::KeyAction::Repeat : input::KeyAction::Press) : input::KeyAction::Release,
                         modifiers(event->modifiers()), event->nativeScanCode(), event->isAutoRepeat(), event->timestamp()});
    }
    update();
}
void QtViewport::keyPressEvent(QKeyEvent* event) { key(event, true); }
void QtViewport::keyReleaseEvent(QKeyEvent* event) { key(event, false); }
void QtViewport::focusInEvent(QFocusEvent* event) {
    QOpenGLWindow::focusInEvent(event);
    if (activated) {
        activated();
    }
}
