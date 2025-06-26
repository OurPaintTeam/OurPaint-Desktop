#include "DrawArcTool.h"


void DrawArcTool::draw(QPainter& painter, const QPointF& nowCursor) {
    const QPointF cursor = roundCursor(nowCursor);

    switch (state) {
        case DrawState::Started:
            drawPreview(painter, startCoordinates, cursor);
            break;
        case DrawState::Completed: {
            const QPointF center = (cursor + startCoordinates) / 2.0;
            emit SigArc(cursor, startCoordinates, center);
            clear();
            break;
        }
        default:
            clear();
            break;
    }
}


void DrawArcTool::drawPreview(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) {
    const QPointF center = (startCoordinates + cursor) / 2.0;
//    DrawFigures::drawArc(painter, cursor, startCoordinates, center);
}
