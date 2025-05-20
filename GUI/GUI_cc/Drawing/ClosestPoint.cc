#include "ClosestPoint.h"

QPointF ClosestPoint::findClosestPoint(const std::unordered_map<ID, Point*>& points, const QPointF cursor) {
    if (points.size() == 0) {
        return QPointF{};
    }

    QPointF closest;
    qreal minDistance = std::numeric_limits<qreal>::max();

    for (const auto& pt: points) {
        const Point* point = pt.second;
        const qreal distance = qSqrt(qPow(point->x - cursor.x(), 2) +
                                     qPow(point->y - cursor.y(), 2));
        if (distance < minDistance) {
            minDistance = distance;
            closest = QPointF(point->x, point->y);
        }
    }

    return closest;
}


[[maybe_unused]] QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Point*>& points,const QRectF& rect, QVector<ID>& vec_id) {

    vec_id.clear();

    for (const auto& pt: points) {
        const Point* point = pt.second;
        const QPointF p(point->x, point->y);
        if (rect.contains(p)) {
            vec_id.push_back(pt.first);
        }
    }
    return vec_id;
}


[[maybe_unused]] QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Circle*>& circles,const QRectF& rect, QVector<ID>& vec_id) {
    vec_id.clear();
    for (const auto& pair: circles) {
        const Circle* circle = pair.second;
        const QPointF center(circle->center->x, circle->center->y);
        const qreal radius = circle->r;
        const qreal diam = 2 * radius;

        QRectF circleBounds(center.x() - radius, center.y() - radius,
                            diam, diam);

        if (rect.contains(circleBounds)) {
            vec_id.push_back(pair.first);
        }
    }
    return vec_id;
}


[[maybe_unused]] QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Section*>& sections,const QRectF& rect, QVector<ID>& vec_id) {
    vec_id.clear();
    for (const auto& sec: sections) {
        const Section* section = sec.second;
        const QPointF p1(section->beg->x, section->beg->y);
        const QPointF p2(section->end->x, section->end->y);

        if (rect.contains(p1) || rect.contains(p2)) {
            vec_id.push_back(sec.first);
        } else {
            const QLineF line(p1, p2);
            QPolygonF polygon=rectToPolygon(rect);

            for (qsizetype i = 0; i < polygon.size(); ++i) {
                const QLineF edge(polygon[i], polygon[(i + 1) % polygon.size()]);
                QPointF intersection;
                if (line.intersects(edge, &intersection) == QLineF::BoundedIntersection) {
                    vec_id.push_back(sec.first);
                    break;
                }
            }
        }
    }
    return vec_id;
}


[[maybe_unused]] QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Arc*>& arcs,const QRectF& rect, QVector<ID>& vec_id) {
    vec_id.clear();

    for (const auto& elem: arcs) {
        const ID& id = elem.first;
        const Arc* arc = elem.second;

        const QPointF beg(arc->beg->x, arc->beg->y);
        const QPointF end(arc->end->x, arc->end->y);
        const QPointF center(arc->center->x, arc->center->y);

        const qreal radius = QLineF(center, beg).length();
        qreal angleStart = QLineF(center, end).angle();
        qreal angleEnd = QLineF(center, beg).angle();

        angleStart = fmod(angleStart + 360.0, 360.0);
        angleEnd = fmod(angleEnd + 360.0, 360.0);

        qreal span = angleEnd - angleStart;
        if (span <= 0.1) {
            span += 360.0;
        }
        const qreal midAngleDeg = fmod(angleStart + span / 2.0, 360.0);
        const qreal midAngleRad = qDegreesToRadians(midAngleDeg);

        const QPointF mid(
                center.x() + radius * qCos(midAngleRad),
                center.y() - radius * qSin(midAngleRad)
        );

        if (rect.contains(beg) || rect.contains(end) || rect.contains(center) || rect.contains(mid)) {
            vec_id.push_back(id);
        }
    }

    return vec_id;
}


QPolygonF ClosestPoint::rectToPolygon(const QRectF& rect) {
    return QPolygonF{ rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft() };
}


bool ClosestPoint::pointInRect(const Point* pt, const QRectF& rect) {
    return rect.contains(QPointF(pt->x, pt->y));
}


bool ClosestPoint::circleInRect(const Circle* circle, const QRectF& rect) {
    const QPointF center(circle->center->x, circle->center->y);
    const qreal r = circle->r;
    return rect.contains(QRectF(center.x() - r, center.y() - r, 2*r, 2*r));
}


bool ClosestPoint::sectionInRect(const Section* section, const QRectF& rect) {
    const QPointF p1(section->beg->x, section->beg->y);
    const QPointF p2(section->end->x, section->end->y);

    if (rect.contains(p1) || rect.contains(p2)) {
        return true;
    }

    QLineF line(p1, p2);
    QPolygonF poly = rectToPolygon(rect);
    for (int i = 0; i < poly.size(); ++i) {
        QLineF edge(poly[i], poly[(i+1) % poly.size()]);
        QPointF intersection;
        if (line.intersects(edge, &intersection) == QLineF::BoundedIntersection) {
            return true;
        }
    }
    return false;
}


bool ClosestPoint::arcInRect(const Arc* arc, const QRectF& rect){
    const QPointF beg(arc->beg->x, arc->beg->y);
    const QPointF end(arc->end->x, arc->end->y);
    const QPointF center(arc->center->x, arc->center->y);

    const qreal radius = QLineF(center, beg).length();
    qreal angleStart = QLineF(center, end).angle();
    qreal angleEnd = QLineF(center, beg).angle();

    angleStart = fmod(angleStart + 360.0, 360.0);
    angleEnd = fmod(angleEnd + 360.0, 360.0);

    qreal span = angleEnd - angleStart;
    if (span <= 0.1) {
        span += 360.0;
    }

    const qreal midAngleDeg = fmod(angleStart + span / 2.0, 360.0);
    const qreal midAngleRad = qDegreesToRadians(midAngleDeg);

    const QPointF mid(
            center.x() + radius * qCos(midAngleRad),
            center.y() - radius * qSin(midAngleRad)
    );

    if (rect.contains(beg) || rect.contains(end) || rect.contains(center) || rect.contains(mid)) {
        return true;
    }

    return false;
}


QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Point*>& points,const QRectF& rect) {
    QVector<ID> vec_id;

    for (const auto& pt: points) {
        if (pointInRect(pt.second,rect)) {
            vec_id.push_back(pt.first);
        }
    }
    return vec_id;
}


QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Circle*>& circles,const QRectF& rect) {
    QVector<ID> vec_id;
    for (const auto& pair: circles) {
        if (circleInRect(pair.second,rect)) {
            vec_id.push_back(pair.first);
        }
    }
    return vec_id;
}


QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Section*>& sections, const QRectF& rect) {
    QVector<ID> vec_id;

    for (const auto& sec: sections) {
        if (sectionInRect(sec.second, rect)) {
            vec_id.push_back(sec.first);
        }
    }
    return vec_id;
}


QVector<ID>
ClosestPoint::enteringInRect(const std::unordered_map<ID, Arc*>& arcs,const QRectF& rect) {
    QVector<ID> vec_id;

    for (const auto& arc: arcs) {
        if (arcInRect(arc.second, rect)) {
            vec_id.push_back(arc.first);
        }
    }

    return vec_id;
}


bool ClosestPoint::checkFigure(const QPointF point, const QPointF cursor, const qreal zoom) {
    return distanceBetweenPoints(point, cursor) <= roundRange(zoom,3.0);
}


bool ClosestPoint::checkFigure(const QPointF center,const qreal radius, const QPointF cursor, const qreal zoom) {
    const qreal distance = distanceBetweenPoints(center, cursor);
    return qAbs(distance - radius) <= roundRange(zoom,3.0);
}


bool
ClosestPoint::checkFigure(const QPointF startPoint, const QPointF endPoint, const QPointF cursor, const qreal zoom) {
    return distancePointToSection(cursor, startPoint, endPoint) <= roundRange(zoom, 5.0);
}


bool
ClosestPoint::checkFigure(const QPointF startPoint, const QPointF endPoint, const QPointF centerPoint,
                          const QPointF cursor, const qreal zoom) {
    const qreal radius = distanceBetweenPoints(startPoint, centerPoint);
    const qreal distToCenter = distanceBetweenPoints(cursor, centerPoint);

    const qreal RANGE = 3.0 / zoom;
    if (qAbs(distToCenter - radius) > RANGE) {
        return false;
    }

    qreal angleEnd = QLineF(centerPoint, startPoint).angle();
    qreal angleStart = QLineF(centerPoint, endPoint).angle();
    qreal angleCursor = QLineF(centerPoint, cursor).angle();

    angleStart = fmod(angleStart + 360.0, 360.0);
    angleEnd = fmod(angleEnd + 360.0, 360.0);
    angleCursor = fmod(angleCursor + 360.0, 360.0);

    if (angleStart > angleEnd) {
        return (angleCursor >= angleStart || angleCursor <= angleEnd);
    }

    return (angleCursor >= angleStart && angleCursor <= angleEnd);
}


qreal ClosestPoint::distancePointToSection(const QPointF cursor, const QPointF startPoint, const QPointF endPoint) {
    const QPointF segment = endPoint - startPoint;
    const qreal dx = segment.x();
    const qreal dy = segment.y();

    if (qFuzzyIsNull(dx) && qFuzzyIsNull(dy)) {
        return QLineF(cursor, startPoint).length();
    }

    const QPointF toCursor = cursor - startPoint;

    const qreal dot = QPointF::dotProduct(toCursor, segment);
    const qreal lenSq = QPointF::dotProduct(segment, segment);

    qreal t = dot / lenSq;
    t = qBound(0.0, t, 1.0);

    const QPointF projection = startPoint + segment * t;

    return QLineF(cursor, projection).length();
}


inline qreal ClosestPoint::distanceBetweenPoints(const QPointF& a, const QPointF& b) {
    return qSqrt((a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()));
}


inline qreal ClosestPoint::roundRange(const qreal zoom,const qreal range = 3.0) {
    return range / zoom;
}









