#include "Drawing/Mouse/DrawArcTool.h"


void DrawArcTool::draw(QPainter& painter, const QPointF& nowCursor) {
    const QPointF Cursor = roundCursor(nowCursor);

    switch (countClick) {
        case 1:
            drawPreview(painter, startCoordinates, Cursor);
            break;
        case 2: {
            const QPointF center = (Cursor + startCoordinates) / 2.0;
            emit SigArc(Cursor, startCoordinates, center);
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
    DrawFigures::drawArc(painter, cursor, startCoordinates, center);
}
