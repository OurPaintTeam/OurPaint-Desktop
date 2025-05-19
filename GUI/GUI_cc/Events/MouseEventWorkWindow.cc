#include "MouseEventWorkWindow.h"


MouseWorkWindow::MouseWorkWindow(QWidget* parent)
        : QObject(parent), m_parent(parent) {
    if (m_parent) {
        m_parent->installEventFilter(this);

        // Enabling mouse tracking for the parent widget
        setMouseTrackingRecursively(m_parent, true);
    }

    m_mouseIdleTimer.setInterval(200);
    m_mouseIdleTimer.setSingleShot(true);
    connect(&m_mouseIdleTimer, &QTimer::timeout, this, [this]() {
        if (QCursor::pos() == m_lastMousePos) {
            ModeManager::setNotActiveMode(MouseMode::MouseMove);
        }
    });
}

void MouseWorkWindow::setMouseTrackingRecursively(QWidget* widget, bool enable) {
    if (!widget) {
        return;
    }

    widget->setMouseTracking(enable);

    for (auto child: widget->findChildren<QWidget*>()) {
        setMouseTrackingRecursively(child, enable);
    }
}


bool MouseWorkWindow::eventFilter(QObject* obj, QEvent* event) {

    if (obj == m_parent) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mousePressEvent(mouseEvent);
            m_parent->update();

            return true; // Event processed
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouseMoveEvent(mouseEvent);
            m_parent->update();

            return false;
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouseReleaseEvent(mouseEvent);
            m_parent->update();

            return true;
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouseDoubleClickEvent(mouseEvent);
            m_parent->update();

            return true;
        } else if (event->type() == QEvent::Enter) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            mouseMoveEvent(mouseEvent); // Event handling when the mouse enters the object area
            ModeManager::setCursor(true);
            m_parent->update();

            return true;
        } else if (event->type() == QEvent::Leave) {
            ModeManager::setCursor(false);
            m_parent->update();

            return false;
        }
    }
    return QObject::eventFilter(obj, event);
}


void MouseWorkWindow::mousePressEvent(QMouseEvent* event) {

    if (event->button() == Qt::RightButton) {
        ModeManager::setActiveMode(MouseMode::RightClick);
        Scaling::startMousePress(event->pos().x(), event->pos().y());
        m_parent->update();
    }
    if (event->button() == Qt::LeftButton) {
        ModeManager::setActiveMode(MouseMode::LeftClick);
        Scaling::startMousePress(event->pos().x(), event->pos().y());
        m_parent->update();
    }
}


void MouseWorkWindow::mouseMoveEvent(QMouseEvent* event) {

    ModeManager::setActiveMode(MouseMode::MouseMove);

    m_lastMousePos = QCursor::pos();
    m_mouseIdleTimer.start();

    if (ModeManager::getActiveMode(MouseMode::RightClick)) {
        Scaling::mouseMove();
        m_parent->update();
    }

    Scaling::setCursor(event->pos().x(), event->pos().y());

}


void MouseWorkWindow::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::RightButton) {
        ModeManager::setActiveMode(MouseMode::ReleasingRight);
        m_parent->update();

    }
    if (event->button() == Qt::LeftButton) {
        ModeManager::setActiveMode(MouseMode::ReleasingLeft);
        m_parent->update();

    }
}


void MouseWorkWindow::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        ModeManager::setActiveMode(MouseMode::DoubleClickLeft);
        m_parent->update();
    }
}
