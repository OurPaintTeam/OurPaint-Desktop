#include "DrawFigures.h"


QPen DrawFigures::MyColor(Qt::black);


void DrawFigures::setPen(const QPen &color) {
    MyColor = color;
}





void DrawFigures::drawRectangle(QPainter &painter,const QPointF firstPoint,const QPointF endPoint) {
    QPen currentPen = QPen(Qt::blue);
    currentPen.setCapStyle(Qt::RoundCap);
    painter.setPen(currentPen);

    const QPointF start(Scaling::scaleCoordinate(firstPoint.x()), Scaling::scaleCoordinate(-firstPoint.y()));
    const QPointF end(Scaling::scaleCoordinate(endPoint.x()), Scaling::scaleCoordinate(-endPoint.y()));

    constexpr QColor fillColor(173, 216, 230, 128); // light blue with transparency
    const QBrush fillBrush(fillColor, Qt::SolidPattern);
    painter.setBrush(fillBrush);

    painter.drawRect(QRectF(start, end));

    MyColor = QPen(Qt::black);
}