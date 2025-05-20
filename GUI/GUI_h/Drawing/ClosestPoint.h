#ifndef OURPAINT_CLOSESTPOINT_H
#define OURPAINT_CLOSESTPOINT_H

#include <vector>
#include <limits>
#include <cmath>
#include <QPointF>
#include <QRectF>
#include <QLineF>
#include <QPolygonF>
#include <list>
#include <QtMath>

#include "GeometricObjects.h"
#include "Scaling.h"

// A class for searching for the closest points to the cursor

class ClosestPoint {

public:
    // Nearest point search function
    static QPointF findClosestPoint(const std::unordered_map<ID, Point*>& points, const QPointF cursor);

    // Checking for entry into a rectangle
    [[maybe_unused]]
    static QVector<ID> enteringInRect(const std::unordered_map<ID, Point*>& points,const QRectF& rect,QVector<ID>&);

    [[maybe_unused]]
    static QVector<ID> enteringInRect(const std::unordered_map<ID, Section*>& sections,const QRectF& rect,QVector<ID>&);

    [[maybe_unused]]
    static QVector<ID> enteringInRect(const std::unordered_map<ID, Circle*>& circles,const QRectF& rect,QVector<ID>&);

    [[maybe_unused]]
    static QVector<ID> enteringInRect(const std::unordered_map<ID, Arc*>& arcs,const QRectF& rect, QVector<ID>&);

    static QPolygonF rectToPolygon(const QRectF& rect);
    static bool pointInRect(const Point* pt, const QRectF& rect);
    static bool circleInRect(const Circle* circle, const QRectF& rect);
    static bool sectionInRect(const Section* section, const QRectF& rect);
    static bool arcInRect(const Arc* arc, const QRectF& rect);

    static QVector<ID> enteringInRect(const std::unordered_map<ID, Arc*>& arcs,const QRectF& rect);
    static QVector<ID> enteringInRect(const std::unordered_map<ID, Section*>& sections,const QRectF& rect);
    static QVector<ID> enteringInRect(const std::unordered_map<ID, Circle*>& circles,const QRectF& rect);
    static QVector<ID> enteringInRect(const std::unordered_map<ID, Point*>& points,const QRectF& rect);

    // The function of checking the coordinates of the cursor and the figure to move
    static bool checkFigure(const QPointF point, const QPointF cursor, const qreal zoom);
    static bool checkFigure(const QPointF center,const qreal radius, const QPointF cursor, const qreal zoom);
    static bool checkFigure(const QPointF startPoint, const QPointF endPoint, const QPointF cursor, const qreal zoom);
    static bool
    checkFigure(const QPointF startPoint, const QPointF endPoint, const QPointF centerPoint, const QPointF cursor,
                const qreal zoom);

    // Auxiliary function for checking cursor and line
    static double distancePointToSection(const QPointF cursor, const QPointF startPoint, const QPointF endPoint);
    static qreal distanceBetweenPoints(const QPointF& a, const QPointF& b);
    static qreal roundRange(const qreal zoom,const qreal range);
};

#endif //OURPAINT_CLOSESTPOINT_H