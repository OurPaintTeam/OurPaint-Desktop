#include "Scaling.h"

// Static member initializations
constexpr qint16 Scaling::userUnitSize = 20;
qreal Scaling::zoom = userUnitSize;
qreal Scaling::scale = 1.0;
[[maybe_unused]] bool Scaling::usersResize = false;

qreal Scaling::Delta::X = 0;
qreal Scaling::Delta::Y = 0;

qint16 Scaling::LastMousePos::x = 0;
qint16 Scaling::LastMousePos::y = 0;

qint16 Scaling::StartMonitorSize::StartMonitorWidth = 1;
qint16 Scaling::StartMonitorSize::StartMonitorHeight = 1;

qreal Scaling::CenteredCoordinates::CenteredCoordinatesX = 1.0;
qreal Scaling::CenteredCoordinates::CenteredCoordinatesY = 1.0;

qint16 Scaling::ActualMonitorSize::ActualMonitorWidth = 1;
qint16 Scaling::ActualMonitorSize::ActualMonitorHeight = 1;

qint16 Scaling::Cursor::x = 0;
qint16 Scaling::Cursor::y = 0;

[[maybe_unused]] qint16 Scaling::getStartWidth() {
    return Scaling::StartMonitorSize::StartMonitorWidth;
}

[[maybe_unused]] qint16 Scaling::getStartHeight() {
    return Scaling::StartMonitorSize::StartMonitorHeight;
}

qint16 Scaling::getActualMonitorWidth() {
    return Scaling::ActualMonitorSize::ActualMonitorWidth;
}

qint16 Scaling::getActualMonitorHeight() {
    return Scaling::ActualMonitorSize::ActualMonitorHeight;
}

QPoint Scaling::getActualMonitorSize() {
    return {Scaling::getActualMonitorWidth(), Scaling::getActualMonitorHeight()};
}

qreal Scaling::getCenteredCoordinatesX() {
    return Scaling::CenteredCoordinates::CenteredCoordinatesX;
}

qreal Scaling::getCenteredCoordinatesY() {
    return Scaling::CenteredCoordinates::CenteredCoordinatesY;
}

QPointF Scaling::getCenteredCoordinates() {
    return {Scaling::getCenteredCoordinatesX(), Scaling::getCenteredCoordinatesY()};
}

[[maybe_unused]]
void Scaling::setCenteredCoordinatesX(qreal x) {
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x;
}

[[maybe_unused]]
void Scaling::setCenteredCoordinatesY(qreal y) {
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y;
}


void Scaling::updateScaling() {
    scale = 1.0;
    zoom = userUnitSize;
    usersResize = false;
}

void Scaling::setStartMonitorSize(qint16 x, qint16 y) {
    Scaling::StartMonitorSize::StartMonitorWidth = x;
    Scaling::StartMonitorSize::StartMonitorHeight = y;
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x / 2.0;
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y / 2.0;
}

void Scaling::setActualMonitorSize(qint16 x, qint16 y) {
    Scaling::ActualMonitorSize::ActualMonitorWidth = x;
    Scaling::ActualMonitorSize::ActualMonitorHeight = y;
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x / 2.0;
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y / 2.0;
}


qreal Scaling::logic(qreal X) {
    return X / (scale * zoom);
}

qreal Scaling::logicCursorX() {
    return ((Scaling::getCursorX() - Scaling::CenteredCoordinates::CenteredCoordinatesX - Scaling::Delta::X) /
            (scale * zoom));
}

qreal Scaling::logicCursorY() {
    // The y-axis is inverted
    return ((-Scaling::getCursorY() + Scaling::CenteredCoordinates::CenteredCoordinatesY + Scaling::Delta::Y) /
            (scale * zoom));
}

QPointF Scaling::logicCursor() {
    return {Scaling::logicCursorX(), Scaling::logicCursorY()};
}


qreal Scaling::scaleCoordinate(qreal X) {
    return (X * scale * zoom);
}

qreal Scaling::scaleCoordinateX(qreal X) {
    return (X - Scaling::Delta::X - Scaling::CenteredCoordinates::CenteredCoordinatesX);
}

qreal Scaling::scaleCoordinateY(qreal Y) {
    return (Y - Scaling::Delta::Y - Scaling::CenteredCoordinates::CenteredCoordinatesY);
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
    Scaling::Delta::X = 0;
    Scaling::Delta::Y = 0;
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
    return Scaling::Delta::X;
}

qreal Scaling::getDeltaY() {
    return Scaling::Delta::Y;
}

QPointF Scaling::getDelta() {
    return {Scaling::getDeltaX(), Scaling::getDeltaY()};
}

int Scaling::getCursorX() {
    return Scaling::Cursor::x;
}

int Scaling::getCursorY() {
    return Scaling::Cursor::y;
}

[[maybe_unused]]
QPoint Scaling::getCursor() {
    return {getCursorX(), getCursorY()};
}

void Scaling::setCursor(qint16 x, qint16 y) {
    Scaling::Cursor::x = x;
    Scaling::Cursor::y = y;
}

void Scaling::setDelta(qreal dx, qreal dy) {
    Scaling::Delta::X += dx;
    Scaling::Delta::Y += dy;
}

[[maybe_unused]]
void Scaling::setDeltaX() {
    const qint16 step = 10;
    Scaling::Delta::X += step;
}

[[maybe_unused]]
void Scaling::setDeltaY() {
    const qint16 step = 10;
    Scaling::Delta::Y += step;
}

void Scaling::startMousePress(qint16 x, qint16 y) {
    Scaling::LastMousePos::x = x;
    Scaling::LastMousePos::y = y;
}

void Scaling::mouseMove() {
    usersResize = true;
    int deltaX_ = Scaling::Cursor::x - Scaling::LastMousePos::x;
    int deltaY_ = Scaling::Cursor::y - Scaling::LastMousePos::y;

    setDelta(deltaX_, deltaY_);

    Scaling::LastMousePos::x = Scaling::Cursor::x;
    Scaling::LastMousePos::y = Scaling::Cursor::y;
}

int Scaling::getCursorDeltaX() {
    int temp = Scaling::Cursor::x - Scaling::LastMousePos::x;
    Scaling::LastMousePos::x = Scaling::Cursor::x;
    return temp;
}

int Scaling::getCursorDeltaY() {
    int temp = -Scaling::Cursor::y + Scaling::LastMousePos::y;
    Scaling::LastMousePos::y = Scaling::Cursor::y;
    return temp;
}

[[maybe_unused]]
void Scaling::resetUsersResize() {
    usersResize = false;
}
