#include "DrawFigures.h"


QPen DrawFigures::MyColor(Qt::black);


void DrawFigures::setPen(const QPen &color) {
    MyColor = color;
}


qreal DrawFigures::angleBetween(const QPointF &center, const QPointF &point) {
    return QLineF(center, point).angle();
}


void DrawFigures::drawPoint(QPainter &painter, const std::unordered_map<ID, Point *> &points,
                                               const std::unordered_map<ID, Color> &vec_id) {

    if (points.empty()) {
        return;
    }

    const QBrush blackBrush = QBrush(Qt::black);
    const QBrush otherBrush = QBrush(MyColor.color());
    const QBrush brush = (MyColor.color() == Qt::black) ? blackBrush : otherBrush;
    painter.setBrush(brush);

    painter.setPen(Qt::NoPen);

    constexpr qint16 SMALL_RADIUS = 1;
    constexpr qint16 BIG_RADIUS = 2;

    
    if (vec_id.empty()) {
        
        for (const auto &pt : points) {
            const Point *point = pt.second;
            painter.drawEllipse(QPointF(Scaling::scaleCoordinate(point->x),
                                               Scaling::scaleCoordinate(-point->y)),
                                                     SMALL_RADIUS, SMALL_RADIUS);
        }
        
    } else {
        
        for (const auto &pt : points) {
            const Point *point = pt.second;
            const QPointF logicPoint(Scaling::scaleCoordinate(point->x), Scaling::scaleCoordinate(-point->y));

            bool isSelected = vec_id.contains(pt.first);
            const qint16 pointRadius = isSelected ? BIG_RADIUS : SMALL_RADIUS;

            painter.setBrush(QBrush(Qt::black));
            painter.setPen(Qt::NoPen);

            painter.drawEllipse(logicPoint, pointRadius, pointRadius);

            if (isSelected) {
                DrawAdditionalInf::drawPointID(painter, pt.first,logicPoint);
                DrawAdditionalInf::drawPointGlow(painter, logicPoint,vec_id.at(pt.first));
            }
            
        }
    }
    MyColor = QPen(Qt::black);
}


void DrawFigures::drawCircle(QPainter &painter, const std::unordered_map<ID, Circle *> &circles,const  std::unordered_map<ID,Color> &vec_id) {
    if (circles.empty()) {
        return;
    }

    const QPen blackPen=QPen(Qt::black);
    const QPen otherPen=QPen(MyColor);
    QPen pen = (MyColor.color() == Qt::black) ? blackPen : otherPen;
    
    constexpr qint16 SIZE_PEN = 1;
    constexpr qint16 BIG_SIZE = 2;
    
    pen.setWidth(SIZE_PEN);
    
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    
    painter.setBrush(Qt::NoBrush);
    painter.setPen(pen);

    if (vec_id.empty()) {
        for (const auto &cir : circles) {
            const Circle *circle = cir.second;
            painter.drawEllipse(QPointF(Scaling::scaleCoordinate(circle->center->x),
                                               Scaling::scaleCoordinate(-circle->center->y)),
                                                 Scaling::scaleCoordinate(circle->r),
                                                 Scaling::scaleCoordinate(circle->r));
        }
    }else {

        for (const auto &pair: circles) {
            const ID &id = pair.first;
            const Circle *circle = pair.second;

            const qreal scaledRadius = Scaling::scaleCoordinate(circle->r);
            const QPointF logicCenter(Scaling::scaleCoordinate(circle->center->x),
                                Scaling::scaleCoordinate(-circle->center->y));

            bool selected = vec_id.contains(id);

            pen.setWidth(selected ? BIG_SIZE : SIZE_PEN);
            painter.setPen(pen);
            
            painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

            if (selected) {
                DrawAdditionalInf::drawCircleID(painter,id, logicCenter, scaledRadius);
                DrawAdditionalInf::drawCircleGlow(painter, logicCenter, scaledRadius,vec_id.at(id));
            }
        }
    }

    MyColor = QPen(Qt::black);
}


void DrawFigures::drawSection(QPainter &painter, const std::unordered_map<ID, Section *> &sections,const  std::unordered_map<ID,Color> &vec_id) {
    if (sections.empty()) { 
        return;
    }

    const QPen blackPen=QPen(Qt::black);
    const QPen otherPen=QPen(MyColor);
    QPen currentPen = (MyColor.color() == Qt::black) ? blackPen : otherPen;

    constexpr qint16 SIZE_PEN = 1;
    constexpr qint16 BIG_SIZE = 2;
    
    currentPen.setCapStyle(Qt::RoundCap);
    painter.setPen(currentPen);

    if (vec_id.empty()) {
        for (const auto &sec : sections) {
            const Section *section = sec.second;

            const QPointF start(Scaling::scaleCoordinate(section->beg->x),
                          Scaling::scaleCoordinate(-section->beg->y));
            const QPointF end(Scaling::scaleCoordinate(section->end->x),
                        Scaling::scaleCoordinate(-section->end->y));

            painter.drawLine(start, end);

            DrawAdditionalInf::drawCoordinateLine(painter, start, end);
        }
    }else {

        for (const auto &pair: sections) {
            const ID &id = pair.first;
            const Section *section = pair.second;

            const QPointF start(Scaling::scaleCoordinate(section->beg->x),
                                Scaling::scaleCoordinate(-section->beg->y));

            const QPointF end(Scaling::scaleCoordinate(section->end->x),
                              Scaling::scaleCoordinate(-section->end->y));

            bool selected = vec_id.contains(id);

            currentPen.setWidth(selected ? BIG_SIZE : SIZE_PEN);
            painter.setPen(currentPen);

            painter.drawLine(start, end);

            DrawAdditionalInf::drawCoordinateLine(painter, start, end);

            if (selected) {
                DrawAdditionalInf::drawSectionID(painter,id, start, end);
                DrawAdditionalInf::drawSectionGlow(painter, start, end,vec_id.at(id));
            }
        }
    }

    MyColor = QPen(Qt::black);
}


void DrawFigures::drawArc(QPainter &painter, const std::unordered_map<ID,  Arc*> &arcs, const  std::unordered_map<ID,Color> &vec_id) {
    if (arcs.size() == 0) {
        return;
    }

    const QPen blackPen=QPen(Qt::black);
    const QPen otherPen=QPen(MyColor);
    QPen currentPen = (MyColor.color() == Qt::black) ? blackPen : otherPen;

    constexpr qint16 SIZE_PEN = 1;
    constexpr qint16 BIG_SIZE = 2;

    currentPen.setCapStyle(Qt::RoundCap);
    painter.setPen(currentPen);

    if (!vec_id.empty()) {
        for (const auto &elem: arcs) {
            const ID &id = elem.first;
            const Arc *arc = elem.second;

            bool selected = vec_id.contains(id);

            currentPen.setWidth(selected ? BIG_SIZE : SIZE_PEN);
            painter.setPen(currentPen);

            const QPointF center(Scaling::scaleCoordinate(arc->center->x), Scaling::scaleCoordinate(-arc->center->y));
            const QPointF start(Scaling::scaleCoordinate(arc->beg->x), Scaling::scaleCoordinate(-arc->beg->y));
            const QPointF end(Scaling::scaleCoordinate(arc->end->x), Scaling::scaleCoordinate(-arc->end->y));

            const qreal radius = std::hypot(start.x() - center.x(), start.y() - center.y());

            const qreal startAngleDeg = angleBetween(center, start);
            const qreal endAngleDeg = angleBetween(center, end);
            qreal spanAngleDeg = endAngleDeg - startAngleDeg;

            if (spanAngleDeg <= 0) {
                spanAngleDeg += 360;
            }

            const QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

            const int qtStart = static_cast<int>(startAngleDeg * 16);
            const int qtSpan = static_cast<int>(spanAngleDeg * 16);

            painter.drawArc(rect, qtStart, qtSpan);

            if (selected) {
                DrawAdditionalInf::drawArcGlow(painter, rect, qtStart, qtSpan, vec_id.at(id));
                DrawAdditionalInf::drawArcID(painter, id, center, startAngleDeg, endAngleDeg, radius);
            }

        }
    } else {
        for (const auto &elem: arcs) {

            const Arc *arc = elem.second;

            const QPointF center(Scaling::scaleCoordinate(arc->center->x), Scaling::scaleCoordinate(-arc->center->y));
            const QPointF start(Scaling::scaleCoordinate(arc->beg->x), Scaling::scaleCoordinate(-arc->beg->y));
            const QPointF end(Scaling::scaleCoordinate(arc->end->x), Scaling::scaleCoordinate(-arc->end->y));

            const qreal radius = std::hypot(start.x() - center.x(), start.y() - center.y());

            const qreal startAngleDeg = angleBetween(center, start);
            const qreal endAngleDeg = angleBetween(center, end);
            qreal spanAngleDeg = endAngleDeg - startAngleDeg;

            if (spanAngleDeg <= 0){
                spanAngleDeg += 360;
            }

            const QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

            const int qtStart = static_cast<int>(startAngleDeg * 16);
            const int qtSpan = static_cast<int>(spanAngleDeg * 16);

            painter.drawArc(rect, qtStart, qtSpan);
        }
    }

    MyColor = QPen(Qt::black);
}


void DrawFigures::drawPoint(QPainter &painter,const QPointF point) {
    const QPointF logicPoint(Scaling::scaleCoordinate(point.x()), Scaling::scaleCoordinate(-point.y()));

    const QBrush blackBrush = QBrush(Qt::black);
    const QBrush otherBrush = QBrush(MyColor.color());
    const QBrush brush = (MyColor.color() == Qt::black) ? blackBrush : otherBrush;
    painter.setBrush(brush);

    painter.setPen(Qt::NoPen);

    constexpr qint16 RADIUS = 1;

    painter.drawEllipse(logicPoint, RADIUS, RADIUS);
    MyColor = QPen(Qt::black);
}


void DrawFigures::drawCircle(QPainter &painter,const QPointF center,const qreal radius) {
    const qreal scaledRadius = radius * Scaling::getZoom();
    const QPointF logicCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));

    const QPen blackPen=QPen(Qt::black);
    const QPen otherPen=QPen(MyColor);
    QPen currentPen = (MyColor.color() == Qt::black) ? blackPen : otherPen;

    constexpr qint16 SIZE_PEN = 1;
    currentPen.setWidth(SIZE_PEN);
    currentPen.setJoinStyle(Qt::RoundJoin);
    currentPen.setCapStyle(Qt::RoundCap);
    painter.setPen(currentPen);
    painter.setBrush(Qt::NoBrush);

    painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

    MyColor = QPen(Qt::black);
}


void DrawFigures::drawSection(QPainter &painter,const QPointF beg,const QPointF end) {

    const QPen blackPen=QPen(Qt::black);
    const QPen otherPen=QPen(MyColor);
    QPen currentPen = (MyColor.color() == Qt::black) ? blackPen : otherPen;

    constexpr qint16 SIZE_PEN = 1;
    currentPen.setWidth(SIZE_PEN);
    currentPen.setCapStyle(Qt::RoundCap);
    painter.setPen(currentPen);

    const QPointF start(Scaling::scaleCoordinate(beg.x()), Scaling::scaleCoordinate(-beg.y()));
    const QPointF endLogic(Scaling::scaleCoordinate(end.x()), Scaling::scaleCoordinate(-end.y()));

    painter.drawLine(start, endLogic);

    DrawAdditionalInf::drawCoordinateLine(painter, start, endLogic);

    MyColor = QPen(Qt::black);
}


void DrawFigures::drawArc(QPainter &painter,const QPointF startPoint,const QPointF endPoint,const QPointF centerPoint) {

    const QPen blackPen=QPen(Qt::black);
    const QPen otherPen=QPen(MyColor);
    QPen pen = (MyColor.color() == Qt::black) ? blackPen : otherPen;

    constexpr qint16 SIZE_PEN = 1;
    pen.setWidth(SIZE_PEN);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    const QPointF center(Scaling::scaleCoordinate(centerPoint.x()), Scaling::scaleCoordinate(-centerPoint.y()));
    const QPointF start(Scaling::scaleCoordinate(startPoint.x()), Scaling::scaleCoordinate(-startPoint.y()));
    const QPointF end(Scaling::scaleCoordinate(endPoint.x()), Scaling::scaleCoordinate(-endPoint.y()));

    const qreal radius = std::hypot(start.x() - center.x(), start.y() - center.y());

    const qreal startAngleDeg = angleBetween(center, start);
    const qreal endAngleDeg = angleBetween(center, end);
    qreal spanAngleDeg = endAngleDeg - startAngleDeg;

    if (spanAngleDeg <= 0) {
        spanAngleDeg += 360;
    }

    const QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

    const int qtStart = static_cast<int>(startAngleDeg * 16);
    const int qtSpan = static_cast<int>(spanAngleDeg * 16);

    painter.drawArc(rect, qtStart, qtSpan);

    MyColor = QPen(Qt::black);
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