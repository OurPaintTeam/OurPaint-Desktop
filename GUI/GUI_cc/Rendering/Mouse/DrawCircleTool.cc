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

    CircleStyle style;
    style.figure.object.color = Color::Black;
    style.figure.object.size = 1;

    PointStyle PStyle;
    PStyle.figure.object.color = Color::Black;
    PStyle.figure.object.size = 1;

    render::drawFigure(painter, center, radius,style);
    render::drawFigure(painter, center,PStyle);


    LineStyle LStyle;
    LStyle.figure.object.color = Color::LightGray;
    LStyle.figure.object.size = 1;

    render::drawFigure(painter, center, cursor,LStyle);
}