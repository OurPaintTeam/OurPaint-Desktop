#include "KeyWorkWindow.h"

KeyWorkWindow::KeyWorkWindow(QWidget *parent)
        : QObject(parent)
{
    m_parent=parent;
    parent->setFocusPolicy(Qt::StrongFocus);  // Устанавливаем политику фокуса для родительского виджета
    parent->installEventFilter(this);  // Устанавливаем фильтр событий

}

bool KeyWorkWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->isWidgetType()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            handleKeyPress(keyEvent);
            return true;  // Событие обработано
        }
        else if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            handleKeyRelease(keyEvent);
            return true;  // Событие обработано
        }
    }
    return QObject::eventFilter(obj, event);  // Перехватываем событие и передаем его дальше
}

void KeyWorkWindow::handleKeyPress(QKeyEvent *event) {
    if (event->key() == Qt::Key_Shift) {
        ModeManager::setActiveMode(KeyMode::Shift);
        m_parent->update();
    }

    if (event->key() == Qt::Key_Enter) {
        ModeManager::setActiveMode(KeyMode::Enter);
        m_parent->update();
    }

    if (event->key() == Qt::Key_Tab) {
        ModeManager::setActiveMode(KeyMode::Tab);
        m_parent->update();
        event->accept();  // Принять событие Tab, чтобы предотвратить его дальнейшую обработку
        return;
    }
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_W) { // Ctrl+W
            //  emit REDO(); // Сигнал
        } else if (event->key() == Qt::Key_Z) { // Ctrl+Z
            //  emit UNDO(); // Сигнал
        } else if (event->key() == Qt::Key_Plus) {
            Scaling::setZoomPlus();
            m_parent->update();
        } else if (event->key() == Qt::Key_Minus) {
            Scaling::setZoomMinus();
            m_parent->update();
        } else if (event->key() == Qt::Key_0) {
            Scaling::setZoomZero();
            m_parent->update();
        }
    }}

void KeyWorkWindow::handleKeyRelease(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift) {
        ModeManager::setActiveMode(KeyMode::ReleasingShift);
        m_parent->update();
    }

    if (event->key() == Qt::Key_Tab) {
        ModeManager::setActiveMode(KeyMode::ReleasingTab);
        m_parent->update();
        event->accept();  // Принять событие, чтобы предотвратить его распространение
        return;
    }

    if (event->key() == Qt::Key_Enter) {
        ModeManager::setActiveMode(KeyMode::ReleasingEnter);
        m_parent->update();
    }

}


