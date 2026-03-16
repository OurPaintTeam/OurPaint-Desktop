#include "Scaling.h"

// Static member initializations
constexpr qint16 Scaling::userUnitSize = 20;
qreal Scaling::zoom = userUnitSize;
qreal Scaling::scale = 1.0;
bool Scaling::usersResize = false;

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
    return ActualMonitorSize;
}


QSizeF Scaling::getCenteredCoordinates() {
    return CenteredCoordinates;
}


void Scaling::setStartMonitorSize(const QSize& size) {
    if (size.isValid()) {
        StartMonitorSize = size;
        CenteredCoordinates = QSizeF(size) / 2.0;
    }
}


void Scaling::setActualMonitorSize(const QSize& size) {
    if (size.isValid()) {
        ActualMonitorSize = size;
        CenteredCoordinates = QSizeF(size) / 2.0;
    }
}


QSize Scaling::getStartMonitorSize() {
    return StartMonitorSize;
}


bool Scaling::getUsersResize() {
    return usersResize;
}


QRectF Scaling::scaleCoordinate(const QRectF &X) {
    return {scaleCoordinate(X.x()), scaleCoordinate(X.y()),
            scaleCoordinate(X.width()), scaleCoordinate(X.height())};
}


QPointF Scaling::scaleCoordinate(const QPointF X) {
    return {scaleCoordinate(X.x()), scaleCoordinate(X.y())};
}


qreal Scaling::scaleCoordinate(const qreal X) {
    return (X * scale * zoom);
}


qreal Scaling::scaleCoordinateX(const qreal X) {
    return (X - Delta.x() - CenteredCoordinates.width());
}


qreal Scaling::scaleCoordinateY(const qreal Y) {
    return (Y - Delta.y() - CenteredCoordinates.height());
}


void Scaling::setScale(const qreal x) {
    scale = x;
}


qreal Scaling::getScale() {
    return scale;
}


void Scaling::setZoom(const qreal z) {
    zoom = z;
}


void Scaling::setZoomPlus() {
    usersResize = true;

    if (constexpr qint16 MAXSIZE = 50; zoom < MAXSIZE) {
        zoom *= 1.1;
    } else {
        zoom = MAXSIZE;
    }
    scale = 1.0;
}


void Scaling::setZoomMinus() {
    usersResize = true;
    if (constexpr qreal MINSIZE = 9e-07; zoom > MINSIZE) {
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
    Delta = {0, 0};
}


qint16 Scaling::getUserUnitSize() {
    return userUnitSize;
}


qreal Scaling::getZoom() {
    return zoom;
}


void Scaling::setDelta(const QPoint& delta) {
    Delta += delta;
}


qint32 Scaling::getDeltaX() {
    return Delta.x();
}


qint32 Scaling::getDeltaY() {
    return Delta.y();
}


QPoint Scaling::getDelta() {
    return {getDeltaX(), getDeltaY()};
}


QPoint Scaling::getCursorDelta() {
    const QPoint temp = {Cursor.x() - LastMousePos.x(),
                         LastMousePos.y() - Cursor.y()};
    LastMousePos = Cursor;
    return temp;
}


QPointF Scaling::getCursorLogicDelta() {
    return logic(getCursorDelta());
}


void Scaling::startMousePress(const QPoint& pos) {
    LastMousePos = pos;
}


void Scaling::mouseMove() {
    usersResize = true;
    const QPoint delta = Cursor - LastMousePos;
    setDelta(delta);
    LastMousePos = Cursor;
}


QPoint Scaling::getLastMousePos() {
    return LastMousePos;
}


void Scaling::setCursor(const QPoint& cursor) {
    Cursor = cursor;
}


QPoint Scaling::getCursor() {
    return {getCursorX(), getCursorY()};
}


qint32 Scaling::getCursorX() {
    return Cursor.x();
}


qint32 Scaling::getCursorY() {
    return Cursor.y();
}


qreal Scaling::logic(qreal X) {
    return X / (scale * zoom);
}


QRectF Scaling::logic(const QRectF &X) {
    return {logic(X.x()), logic(X.y()), logic(X.width()), logic(X.height())};
}


QLineF Scaling::logic(const QPointF& p1, const QPointF& p2){
    return QLineF{logic(p1),logic(p2) };
}


QLineF Scaling::logic(const QLineF& p){
    return QLineF{logic(p.p1()), logic(p.p2())};
}


QPointF Scaling::logic(const QPoint X) {
    return QPointF(X)  / (scale * zoom);
}


QPointF Scaling::logic(QPointF X) {
    return X / (scale * zoom);
}


qreal Scaling::logicCursorX() {
    return ((getCursorX() - CenteredCoordinates.width() - Delta.x()) /
            (zoom));
}


qreal Scaling::logicCursorY() {
    // The y-axis is inverted
    return ((-getCursorY() + CenteredCoordinates.height() + Delta.y()) /
            (scale * zoom));
}


QPointF Scaling::logicCursor() {
    return {logicCursorX(), logicCursorY()};
}


QPointF Scaling::scaleCursor() {
    return {scaleCoordinateX(Cursor.x()), scaleCoordinateY(Cursor.y())};
}


bool ScalingState::operator==(const ScalingState& other) const {
    return qFuzzyCompare(scale, other.scale) &&
           qFuzzyCompare(zoom, other.zoom) &&
           usersResize == other.usersResize &&
           Delta == other.Delta &&
           LastMousePos == other.LastMousePos &&
           Cursor == other.Cursor &&
           StartMonitorSize == other.StartMonitorSize &&
           ActualMonitorSize == other.ActualMonitorSize &&
           CenteredCoordinates == other.CenteredCoordinates;
}


bool ScalingState::operator!=(const ScalingState& other) const {
    return !(*this == other);
}


ScalingState ScalingState::fromCurrent() {
    ScalingState state;
    
    state.scale = Scaling::getScale();
    state.zoom = Scaling::getZoom();
    state.usersResize = Scaling::getUsersResize();
    
    state.Delta = Scaling::getDelta();
    state.LastMousePos = Scaling::getLastMousePos();
    state.Cursor = Scaling::getCursor();
    state.StartMonitorSize = Scaling::getStartMonitorSize();
    state.ActualMonitorSize = Scaling::getActualMonitorSize();
    state.CenteredCoordinates = Scaling::getCenteredCoordinates();
    
    return state;
}


void ScalingState::apply() const {
    Scaling::scale = scale;
    Scaling::zoom = zoom;
    Scaling::usersResize = usersResize;
    
    Scaling::Delta = Delta;
    Scaling::LastMousePos = LastMousePos;
    Scaling::Cursor = Cursor;
    Scaling::StartMonitorSize = StartMonitorSize;
    Scaling::ActualMonitorSize = ActualMonitorSize;
    Scaling::CenteredCoordinates = CenteredCoordinates;
}


ScalingState Scaling::copyState() {
    return ScalingState::fromCurrent();
}


void Scaling::restoreState(const ScalingState& state) {
    state.apply();
}