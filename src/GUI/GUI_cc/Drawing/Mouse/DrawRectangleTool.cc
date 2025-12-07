#include "DrawRectangleTool.h"

void DrawRectangleTool::draw(QPainter& painter, const QPointF& nowCursor) {
    const QPointF cursor = roundCursor(nowCursor);
    switch (state) {
        case DrawState::NotStarted:
            break;
        case DrawState::Started:
            drawPreview(painter, startCoordinates, cursor);
            rect = QRectF(startCoordinates, cursor);
            break;
        case DrawState::Completed:
            rect = QRectF(startCoordinates, cursor);
            clear();
            break;
        default:
            clear();
            break;
    }
}


void DrawRectangleTool::drawPreview(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) {
    DrawFigures::drawRectangle(painter, startCoordinates, cursor);
}


void DrawRectangleTool::clear() {
    startCoordinates = QPointF();
    state = DrawState::NotStarted;
    rect = QRectF{};
}


void DrawRectangleTool::pressButton( const QPointF& cursor){
    if (state == DrawState::NotStarted) {
        startCoordinates = roundCursor(cursor);
        state = DrawState::Started;
    }
}


QRectF DrawRectangleTool::getRect() {
    return rect;
}


void DrawRectangleTool::releasingButton() {
    state = DrawState::NotStarted;
}