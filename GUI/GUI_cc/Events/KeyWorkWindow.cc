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

    if ((event->modifiers() & Qt::ControlModifier) &&
        (event->modifiers() & Qt::ShiftModifier) &&
        event->key() == Qt::Key_Z) {
        // Ctrl+Shift+Z
        emit REDO();
        m_parent->update();
    } else if ((event->modifiers() & Qt::ControlModifier) &&
               !(event->modifiers() & Qt::ShiftModifier) &&
               event->key() == Qt::Key_Z) {
        // Ctrl+Z
        emit UNDO();
        m_parent->update();
    } else if ((event->modifiers() & Qt::ControlModifier) &&
             !(event->modifiers() & Qt::ShiftModifier) &&
             event->key() == Qt::Key_C) {
        // Ctrl+C
        emit COPY();
    } else if ((event->modifiers() & Qt::ControlModifier) &&
               !(event->modifiers() & Qt::ShiftModifier) &&
               event->key() == Qt::Key_V) {
        // Ctrl+V
        emit PASTE();
    } else if ((event->modifiers() & Qt::ControlModifier) &&
               !(event->modifiers() & Qt::ShiftModifier) &&
               event->key() == Qt::Key_X) {
        // Ctrl+X
        emit CUT();
    }

    if (event->modifiers() & Qt::ControlModifier) {
         if (event->key() == Qt::Key_Plus) {
            Scaling::setZoomPlus();
            m_parent->update();
        } else if (event->key() == Qt::Key_Minus) {
            Scaling::setZoomMinus();
            m_parent->update();
        } else if (event->key() == Qt::Key_0) {
            Scaling::setZoomZero();
            m_parent->update();
        }
    }else if (event->key() == Qt::Key_Down) {
        Scaling::setDelta(0,-10);
        m_parent->update();
    }else if (event->key() == Qt::Key_Up) {
        Scaling::setDelta(0,10);
        m_parent->update();
    }else if (event->key() == Qt::Key_Left) {
        Scaling::setDelta(10,0);
        m_parent->update();
    }else if (event->key() == Qt::Key_Right) {
        Scaling::setDelta(-10,0);
        m_parent->update();
    }else if(event->key() == Qt::Key_1){
        emit firstReq();
    }
    else if(event->key() == Qt::Key_2){
        emit secondReq();
    }
    else if(event->key() == Qt::Key_3){
        emit thirdReq();
    }
    else if(event->key() == Qt::Key_4){
        emit fourthReq();
    }
    else if(event->key() == Qt::Key_5){
        emit fifthReq();
    }
    else if(event->key() == Qt::Key_6){
        emit sixthReq();
    }
    else if(event->key() == Qt::Key_7){
        emit seventhReq();
    }
    else if(event->key() == Qt::Key_8){
        emit eighthReq();
    }
    else if(event->key() == Qt::Key_9){
        emit ninthReq();
    }
    else if(event->key() == Qt::Key_0){
        emit tenthReq();
    }

    if (event->key() == Qt::Key_Delete ||event->key() == Qt::Key_Backspace ) {
        emit DELETE();
        m_parent->update();
    }

    if (event->key() == Qt::Key_Shift) {
        ModeManager::setActiveMode(KeyMode::Shift);
        m_parent->update();
    }
}

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


