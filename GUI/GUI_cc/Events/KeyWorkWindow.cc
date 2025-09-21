#include "KeyWorkWindow.h"


KeyWorkWindow::KeyWorkWindow(QWidget* parent)
        : QObject(parent) {
    m_parent = parent;
    parent->setFocusPolicy(Qt::StrongFocus); // Setting the focus policy for the parent widget
    parent->installEventFilter(this);  // Setting the event filter

}


bool KeyWorkWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj->isWidgetType()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            handleKeyPress(keyEvent);
            return true;
        } else if (event->type() == QEvent::KeyRelease) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            handleKeyRelease(keyEvent);
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}


void KeyWorkWindow::handleKeyPress(QKeyEvent* event) {

    switch (event->key()) {
        case Qt::Key_Up:
            upPressed = true;
            break;
        case Qt::Key_Left:
            leftPressed = true;
            break;
        case Qt::Key_Right:
            rightPressed = true;
            break;
        case Qt::Key_Down:
            downPressed = true;
            break;
        default:
            break;
    }

    if (upPressed && leftPressed) {
        Scaling::setDelta({0, 10});
        Scaling::setDelta({10, 0});
        m_parent->update();
    } else if (upPressed && rightPressed) {
        Scaling::setDelta({0, 10});
        Scaling::setDelta({-10, 0});
        m_parent->update();
    } else if (downPressed && leftPressed) {
        Scaling::setDelta({0, -10});
        Scaling::setDelta({10, 0});
        m_parent->update();
    } else if (downPressed && rightPressed) {
        Scaling::setDelta({0, -10});
        Scaling::setDelta({-10, 0});
        m_parent->update();
    } else {

        switch (event->key()) {
            case Qt::Key_Up:
                Scaling::setDelta({0, 10});
                m_parent->update();
                break;
            case Qt::Key_Left:
                Scaling::setDelta({10, 0});
                m_parent->update();
                break;
            case Qt::Key_Right:
                Scaling::setDelta({-10, 0});
                m_parent->update();
                break;
            case Qt::Key_Down:
                Scaling::setDelta({0, -10});
                m_parent->update();
                break;
            case Qt::Key_1:
                emit firstReq();
                break;
            case Qt::Key_2:
                emit secondReq();
                break;
            case Qt::Key_3:
                emit thirdReq();
                break;
            case Qt::Key_4:
                emit fourthReq();
                break;
            case Qt::Key_5:
                emit fifthReq();
                break;
            case Qt::Key_6:
                emit sixthReq();
                break;
            case Qt::Key_7:
                emit seventhReq();
                break;
            case Qt::Key_8:
                emit eighthReq();
                break;
            case Qt::Key_9:
                emit ninthReq();
                break;
            case Qt::Key_0:
                emit tenthReq();
                break;
            case Qt::Key_Delete:
                emit DELETE();
                m_parent->update();
                break;
            case Qt::Key_Backspace:
                emit DELETE();
                m_parent->update();
                break;
            case Qt::Key_Shift:
                ModeManager::setActiveMode(KeyMode::Shift);
                m_parent->update();
                break;
            case Qt::Key_Enter:
                ModeManager::setActiveMode(KeyMode::Enter);
                m_parent->update();
                break;
            case Qt::Key_Tab:
                ModeManager::setActiveMode(KeyMode::Tab);
                m_parent->update();
                event->accept();  // Accept the Tab event to prevent its further processing
                return;
        }

    }


    if (event->modifiers() & Qt::ControlModifier) {

       if((event->modifiers() & Qt::ShiftModifier) &&
        event->key() == Qt::Key_Z) {
            // Ctrl+Shift+Z
            emit REDO();
            m_parent->update();
        } else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Z || event->key() == 1071)) {
            // Ctrl+Z
            emit UNDO();
            m_parent->update();
        } else if (event->key() == Qt::Key_C || event->key() == 1057) {
            // Ctrl+C
            emit COPY();
        } else if (event->key() == Qt::Key_V || event->key() == 1052) {
            // Ctrl+V
            emit PASTE();
        } else if (event->key() == Qt::Key_X || event->key() == 1063) {
            // Ctrl+X
            emit CUT();
        }else {
           switch (event->key()) {
               case Qt::Key_Plus:
                   Scaling::setZoomPlus();
                   m_parent->update();
                   break;
               case Qt::Key_Minus:
                   Scaling::setZoomMinus();
                   m_parent->update();
                   break;
               case Qt::Key_0:
                   Scaling::setZoomZero();
                   m_parent->update();
                   break;
               case Qt::Key_1:
                   ModeManager::setActiveMode(WorkModes::Point);
                   break;
               case Qt::Key_2:
                   ModeManager::setActiveMode(WorkModes::Section);
                   break;
               case Qt::Key_3:
                   ModeManager::setActiveMode(WorkModes::Circle);
                   break;
               case Qt::Key_4:
                   ModeManager::setActiveMode(WorkModes::Arc);
                   break;
           }
       }
    }

    if (event->modifiers() & Qt::ShiftModifier) {

        switch (event->key()) {
            case Qt::Key_Exclam:
                ModeManager::setActiveMode(WorkModes::Editor);
                break;
            case Qt::Key_At:
                ModeManager::setActiveMode(WorkModes::Move);
                break;
            case Qt::Key_NumberSign:
                ModeManager::setActiveMode(WorkModes::Selected);
                break;
            case Qt::Key_Dollar:
                ModeManager::setActiveMode(WorkModes::ShowSize);
                break;

        }
    }

}


void KeyWorkWindow::handleKeyRelease(QKeyEvent* event) {
    if (event->key() == Qt::Key_Shift) {
        ModeManager::setActiveMode(KeyMode::ReleasingShift);
        m_parent->update();
    }

    if (event->key() == Qt::Key_Tab) {
        ModeManager::setActiveMode(KeyMode::ReleasingTab);
        m_parent->update();
        event->accept();  // Accept the event to prevent it from spreading
        return;
    }

    if (event->key() == Qt::Key_Enter) {
        ModeManager::setActiveMode(KeyMode::ReleasingEnter);
        m_parent->update();
    }


    switch (event->key()) {
        case Qt::Key_Up:
            upPressed = false;
            break;
        case Qt::Key_Left:
            leftPressed = false;
            break;
        case Qt::Key_Right:
            rightPressed = false;
            break;
        case Qt::Key_Down:
            downPressed = false;
            break;
        default:
            break;
    }

}


