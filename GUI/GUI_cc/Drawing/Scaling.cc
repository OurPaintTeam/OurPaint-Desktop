#include "Scaling.h"

// Static member initializations
constexpr qint16 Scaling::userUnitSize = 20;
qreal Scaling::zoom = userUnitSize;
qreal Scaling::scale = 1.0;
[[maybe_unused]] bool Scaling::usersResize = false;

QPoint Scaling::Delta(0, 0);
QPoint Scaling::LastMousePos(0, 0);
QPoint Scaling::Cursor(0, 0);
QSize Scaling::StartMonitorSize(0, 0);
QSize Scaling::ActualMonitorSize(0, 0);
QSizeF Scaling::CenteredCoordinates(1, 1);


void Scaling::updateScaling() {
    scale = 1.0;
    zoom = userUnitSize;
    usersResize = false;
}


QSize Scaling::getActualMonitorSize() {
    return Scaling::StartMonitorSize;
}

QSizeF Scaling::getCenteredCoordinates() {
    return Scaling::CenteredCoordinates;
}


void Scaling::setStartMonitorSize(const QSize& size) {
    if (size.isValid()) {
        Scaling::StartMonitorSize = size;
        Scaling::CenteredCoordinates = size / 2.0;
    }
}

void Scaling::setActualMonitorSize(const QSize& size) {
    if (size.isValid()) {
        Scaling::ActualMonitorSize = size;
        Scaling::CenteredCoordinates = size / 2.0;
    }
}


qreal Scaling::scaleCoordinate(qreal X) {
    return (X * scale * zoom);
}

qreal Scaling::scaleCoordinateX(qreal X) {
    return (X - Scaling::Delta.x() - Scaling::CenteredCoordinates.width());
}

qreal Scaling::scaleCoordinateY(qreal Y) {
    return (Y - Scaling::Delta.y() - Scaling::CenteredCoordinates.height());
}


void Scaling::setZoom(qreal z) {
    zoom = z;
}

void Scaling::setZoomPlus() {
    usersResize = true;

    const qint16 MAXSIZE = 50;
    if (zoom < MAXSIZE) {
        zoom *= 1.1;
    } else {
        zoom = MAXSIZE;
    }
    scale = 1.0;
}

void Scaling::setZoomMinus() {
    usersResize = true;
    const qreal MINSIZE = 9e-07;
    if (zoom > MINSIZE) {
        zoom /= 1.1;
    } else {
        zoom = MINSIZE;
    }
    scale = 1.0;
}

void Scaling::setZoomZero() {
    usersResize = true;
    zoom = userUnitSize;
    scale = 1.0;
    Scaling::Delta = {0, 0};
}


qint16 Scaling::getUserUnitSize() {
    return userUnitSize;
}

qreal Scaling::getZoom() {
    return zoom;
}


void Scaling::setDelta(const QPoint& delta) {
    Scaling::Delta += delta;
}

qreal Scaling::getDeltaX() {
    return Scaling::Delta.x();
}

qreal Scaling::getDeltaY() {
    return Scaling::Delta.y();
}

QPointF Scaling::getDelta() {
    return {Scaling::getDeltaX(), Scaling::getDeltaY()};
}


QPoint Scaling::getCursorDelta() {
    const QPoint temp = {Scaling::Cursor.x() - Scaling::LastMousePos.x(),
                         Scaling::LastMousePos.y() - Scaling::Cursor.y()};
    Scaling::LastMousePos = Scaling::Cursor;
    return temp;
}

QPointF Scaling::getCursorLogicDelta() {
    return Scaling::logic(Scaling::getCursorDelta());
}


void Scaling::startMousePress(const QPoint& pos) {
    Scaling::LastMousePos = pos;
}

void Scaling::mouseMove() {
    usersResize = true;
    const QPoint delta = Scaling::Cursor - Scaling::LastMousePos;
    setDelta(delta);
    Scaling::LastMousePos = Scaling::Cursor;
}


void Scaling::setCursor(const QPoint& cursor) {
    Scaling::Cursor = cursor;
}


QPoint Scaling::getCursor() {
    return {getCursorX(), getCursorY()};
}

qint32 Scaling::getCursorX() {
    return Scaling::Cursor.x();
}

qint32 Scaling::getCursorY() {
    return Scaling::Cursor.y();
}


qreal Scaling::logic(qreal X) {
    return X / (scale * zoom);
}

QPointF Scaling::logic(QPoint X) {
    return X / (scale * zoom);
}

QPointF Scaling::logic(QPointF X) {
    return X / (scale * zoom);
}

qreal Scaling::logicCursorX() {
    return ((Scaling::getCursorX() - Scaling::CenteredCoordinates.width() - Scaling::Delta.x()) /
            (zoom));
}

qreal Scaling::logicCursorY() {
    // The y-axis is inverted
    return ((-Scaling::getCursorY() + Scaling::CenteredCoordinates.height() + Scaling::Delta.y()) /
            (scale * zoom));
}

QPointF Scaling::logicCursor() {
    return {Scaling::logicCursorX(), Scaling::logicCursorY()};
}

QPointF Scaling::scaleCursor() {
    return {Scaling::scaleCoordinateX(Scaling::Cursor.x()), Scaling::scaleCoordinateY(Scaling::Cursor.y())};
}