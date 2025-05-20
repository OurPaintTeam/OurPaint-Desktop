#include "Scaling.h"

// Static member initializations
constexpr qint16 Scaling::userUnitSize = 20;
qreal Scaling::zoom = userUnitSize;
qreal Scaling::scale = 1.0;
[[maybe_unused]] bool Scaling::usersResize = false;

QPoint Scaling::Delta(0, 0);
QPoint Scaling::LastMousePos(0, 0);
QPoint Scaling::Cursor(0, 0);
QSize Scaling::StartMonitorSize(0,0);
QSizeF Scaling::CenteredCoordinates(0,0);


quint16 Scaling::ActualMonitorSize::ActualMonitorWidth = 1;
quint16 Scaling::ActualMonitorSize::ActualMonitorHeight = 1;

QSize Scaling::getActualMonitorSize() {
    return Scaling::StartMonitorSize;
}

QSizeF Scaling::getCenteredCoordinates() {
   return Scaling::CenteredCoordinates;
}


void Scaling::updateScaling() {
    scale = 1.0;
    zoom = userUnitSize;
    usersResize = false;
}

void Scaling::setStartMonitorSize(const QSize &size) {
    Scaling::StartMonitorSize = size;
    Scaling::CenteredCoordinates = size/2.0;
}

void Scaling::setActualMonitorSize(const QSize &size) {
    Scaling::ActualMonitorSize::ActualMonitorWidth = size.width();
    Scaling::ActualMonitorSize::ActualMonitorHeight = size.height();
    Scaling::CenteredCoordinates = size/2.0;
}


qreal Scaling::logic(qreal X) {
    return X / (scale * zoom);
}

qreal Scaling::logicCursorX() {
    return ((Scaling::getCursorX() - Scaling::CenteredCoordinates.width() - Scaling::Delta.x()) /
            (scale * zoom));
}

qreal Scaling::logicCursorY() {
    // The y-axis is inverted
    return ((-Scaling::getCursorY() + Scaling::CenteredCoordinates.height() + Scaling::Delta.y()) /
            (scale * zoom));
}

QPointF Scaling::logicCursor() {
    return {Scaling::logicCursorX(), Scaling::logicCursorY()};
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


[[maybe_unused]]
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

[[maybe_unused]]
qreal Scaling::getScale() {
    return scale;
}

qreal Scaling::getZoom() {
    return zoom;
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

qint32 Scaling::getCursorX() {
    return Scaling::Cursor.x();
}

qint32 Scaling::getCursorY() {
    return Scaling::Cursor.y();
}

[[maybe_unused]]
QPoint Scaling::getCursor() {
    return {getCursorX(), getCursorY()};
}

void Scaling::setCursor(const QPoint& cursor) {
    Scaling::Cursor = cursor;
}

void Scaling::setDelta(const QPoint& delta) {
    Scaling::Delta += delta;
}

[[maybe_unused]]
void Scaling::setDeltaX() {
    constexpr qint16 step = 10;
    Scaling::Delta.setX(Scaling::Delta.x() + step);
}

[[maybe_unused]]
void Scaling::setDeltaY() {
    constexpr qint16 step = 10;
    Scaling::Delta.setX(Scaling::Delta.y() + step);
}

void Scaling::startMousePress(const QPoint& pos) {
    Scaling::LastMousePos = pos;
}

void Scaling::mouseMove() {
    usersResize = true;
    const QPoint delta = Scaling::Cursor-Scaling::LastMousePos;
    setDelta(delta);
    Scaling::LastMousePos=Scaling::Cursor;
}

qint32 Scaling::getCursorDeltaX() {
    qint32 temp = Scaling::Cursor.x() - Scaling::LastMousePos.x();
    Scaling::LastMousePos.setX(Scaling::Cursor.x());
    return temp;
}

qint32 Scaling::getCursorDeltaY() {
    qint32 temp = Scaling::LastMousePos.y()-Scaling::Cursor.y();
    Scaling::LastMousePos.setY(Scaling::Cursor.y());
    return temp;
}

[[maybe_unused]]
void Scaling::resetUsersResize() {
    usersResize = false;
}
