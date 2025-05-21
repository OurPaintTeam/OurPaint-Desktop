#include "DrawPointTool.h"


void DrawPointTool::draw(QPainter& painter, const QPointF& nowCursor)  {
    // Rounding the mouse to 1 decimal place
    const QPointF cursor = roundCursor(nowCursor);
    switch (countClick) {
        case 0:
            drawPreview(painter, cursor,cursor/*unused*/);
            break;
        case 1:
            emit SigPoint(cursor);
            clear();
            break;
        default:
            clear();
            break;
    }
}


void DrawPointTool::drawPreview(QPainter& painter, const QPointF& point, const QPointF& /*unused*/) {
    DrawFigures::setPen(hintColor());
    DrawFigures::drawPoint(painter, point);
}
