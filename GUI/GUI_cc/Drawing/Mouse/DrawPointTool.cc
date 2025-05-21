#include "DrawPointTool.h"


void DrawPointTool::draw(QPainter& painter, const QPointF& nowCursor)  {
    // Rounding the mouse to 1 decimal place
    const QPointF cursor = roundCursor(nowCursor);
    switch (state) {
        case DrawState::NotStarted:
            drawPreview(painter, cursor,cursor/*unused*/);
            break;
        case DrawState::Started:
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
