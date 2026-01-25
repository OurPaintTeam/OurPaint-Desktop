#include "MainWindowController.h"

#include <QGestureEvent>
#include <QPinchGesture>
#include <QApplication>
#include <QScreen>
#include <QDebug>

#include "Mainwindow.h"
#include "Modes.h"
#include "Scaling.h"


MainWindowController::MainWindowController(MainWindow* window)
    : mainWindow(window)
{}




void MainWindowController::handleCloseEvent(QCloseEvent* event) {
    if (!ModeManager::getProject()) {
        event->accept();
        return;
    }

    mainWindow->closeProgram() ? event->accept() : event->ignore();
}



void MainWindowController::mousePress(QMouseEvent* event) {
    dragStartPos      = event->globalPosition().toPoint();
    originalGeometry  = mainWindow->geometry();

    if (event->button() == Qt::LeftButton) {
        resizing = (currentRegion != None);
        moving   = !resizing;
    }
}


void MainWindowController::mouseMove(QMouseEvent* event) {
    const QPoint globalPos = event->globalPosition().toPoint();

    if (resizing) {
        QRect geom = originalGeometry;
        const QPoint delta = globalPos - dragStartPos;

        switch (currentRegion) {
            case Top:         geom.setTop(geom.top() + delta.y()); break;
            case Bottom:      geom.setBottom(geom.bottom() + delta.y()); break;
            case Left:        geom.setLeft(geom.left() + delta.x()); break;
            case Right:       geom.setRight(geom.right() + delta.x()); break;
            case TopLeft:     geom.setTopLeft(geom.topLeft() + delta); break;
            case TopRight:    geom.setTopRight(geom.topRight() + delta); break;
            case BottomLeft:  geom.setBottomLeft(geom.bottomLeft() + delta); break;
            case BottomRight: geom.setBottomRight(geom.bottomRight() + delta); break;
            default: break;
        }

        mainWindow->setGeometry(geom);
    }
    else if (moving) {
        mainWindow->move(mainWindow->pos() + globalPos - dragStartPos);
        dragStartPos = globalPos;
    }
    else {
        updateShapeCursor(event->pos());
    }
}


void MainWindowController::mouseRelease(QMouseEvent*) {
    resizing = false;
    moving   = false;
    mainWindow->setCursor(Qt::ArrowCursor);
}

void MainWindowController::mouseDoubleClick(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        mainWindow->isMaximized()
            ? mainWindow->showNormal()
            : mainWindow->showMaximized();
    }
}


void MainWindowController::wheel(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) {
        Scaling::setZoomPlus();
    }
    else {
        Scaling::setZoomMinus();
    }

    event->accept();
}

bool MainWindowController::handleEvent(QEvent* event) {
    if (event->type() == QEvent::Gesture) {
        const auto* ge = static_cast<QGestureEvent*>(event);
        if (const auto* pinch =
                static_cast<QPinchGesture*>(ge->gesture(Qt::PinchGesture))) {

            if (pinch->scaleFactor() > 1.0) {
                Scaling::setZoomPlus();
            }
            else {
                Scaling::setZoomMinus();
            }

            return true;
        }
    }
    return false;
}


bool MainWindowController::handleEventFilter(QObject* obj, QEvent* event) {
    Q_UNUSED(obj)

    switch (event->type()) {
        case QEvent::MouseMove:
            mouseMove(static_cast<QMouseEvent*>(event));
            return true;

        case QEvent::MouseButtonPress:
            mousePress(static_cast<QMouseEvent*>(event));
            return true;

        case QEvent::MouseButtonRelease:
            mouseRelease(static_cast<QMouseEvent*>(event));
            return true;

        case QEvent::MouseButtonDblClick:
            mouseDoubleClick(static_cast<QMouseEvent*>(event));
            return true;

        case QEvent::Wheel:
            wheel(static_cast<QWheelEvent*>(event));
            return true;

        case QEvent::KeyPress:
            keyPress(static_cast<QKeyEvent*>(event));
            return true;

        default:
            break;
    }

    return false;
}


void MainWindowController::keyPress(QKeyEvent* event) {
    if (!(event->modifiers() & Qt::ControlModifier)) {
        return;
    }

    const QRect screen = QApplication::primaryScreen()->availableGeometry();

    switch (event->key()) {
        case Qt::Key_Left:
            mainWindow->setGeometry(
                screen.left(),
                screen.top(),
                screen.width() / 2,
                screen.height()
            );
            break;

        case Qt::Key_Right:
            mainWindow->setGeometry(
                screen.left() + screen.width() / 2,
                screen.top(),
                screen.width() / 2,
                screen.height()
            );
            break;

        case Qt::Key_Up:
            mainWindow->showMaximized();
            break;

        case Qt::Key_Down:
            mainWindow->showMinimized();
            break;

        default:
            break;
    }
}


void MainWindowController::updateShapeCursor(const  QPoint& pos) {
    const int x = pos.x();
    const int y = pos.y();
    const int w = mainWindow->width();
    const int h = mainWindow->height();

    ResizeRegion r = None;

    if (x < edgeMargin && y < edgeMargin)          r = TopLeft;
    else if (x > w - edgeMargin && y < edgeMargin) r = TopRight;
    else if (x < edgeMargin && y > h - edgeMargin) r = BottomLeft;
    else if (x > w - edgeMargin && y > h - edgeMargin) r = BottomRight;
    else if (x < edgeMargin)                       r = Left;
    else if (x > w - edgeMargin)                   r = Right;
    else if (y < edgeMargin)                       r = Top;
    else if (y > h - edgeMargin)                   r = Bottom;

    currentRegion = r;

    switch (r) {
        case TopLeft:
        case BottomRight:
            mainWindow->setCursor(Qt::SizeFDiagCursor);
            break;
        case TopRight:
        case BottomLeft:
            mainWindow->setCursor(Qt::SizeBDiagCursor);
            break;
        case Left:
        case Right:
            mainWindow->setCursor(Qt::SizeHorCursor);
            break;
        case Top:
        case Bottom:
            mainWindow->setCursor(Qt::SizeVerCursor);
            break;
        default:
            mainWindow->setCursor(Qt::ArrowCursor);
            break;
    }
}