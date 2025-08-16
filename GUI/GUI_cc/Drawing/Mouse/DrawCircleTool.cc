#include "DrawCircleTool.h"


void DrawCircleTool::draw(QPainter& painter, const QPointF& nowCursor) {
    const QPointF cursor = roundCursor(nowCursor);

    switch (state) {
        case DrawState::Started:
            drawPreview(painter, startCoordinates, cursor);
            break;
        case DrawState::Completed: {
            const QPointF center = (cursor + startCoordinates) / 2.0;
            qreal radius = qHypot(startCoordinates.x() - center.x(), startCoordinates.y() - center.y());
            emit SigCircle(center, radius);
            clear();
            break;
        }
        default:
            clear();
            break;
    }
}

void DrawCircleTool::drawPreview(QPainter& painter, const QPointF& startCoordinates, const QPointF& cursor) {
    const QPointF center = (startCoordinates + cursor) / 2.0;
    const qreal radius = qHypot(startCoordinates.x() - center.x(), startCoordinates.y() - center.y());

    DrawFigures::drawCircle(painter, center, radius);
    DrawFigures::drawPoint(painter, center);

    DrawFigures::setPen(hintColor());
    DrawFigures::drawSection(painter, center, cursor);
}