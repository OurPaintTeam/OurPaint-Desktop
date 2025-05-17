#ifndef OURPAINT_CLOSESPOINT_H
#define OURPAINT_CLOSESPOINT_H

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

class ClosesPoint {

public:
    // Nearest point search function
    static QPointF findClosestPoint(std::unordered_map<ID, Point *> &points,const QPointF cursor);

    // Checking for entry into a rectangle
    [[maybe_unused]] static QVector<ID>  enteringInRect(std::unordered_map<ID,  Point*>& points,QRectF &rect,QVector<ID>&);
    [[maybe_unused]] static QVector<ID>  enteringInRect(std::unordered_map<ID, Section*>& sections,QRectF &rect,QVector<ID>&);
    [[maybe_unused]] static QVector<ID>  enteringInRect(std::unordered_map<ID,  Circle*>& circles,QRectF &rect,QVector<ID>&);
    [[maybe_unused]] static QVector<ID>  enteringInRect(std::unordered_map<ID,  Arc*>& arcs,QRectF &rect,QVector<ID>&);

    static QVector<ID> enteringInRect(std::unordered_map<ID, Arc *> &arcs, QRectF &rect);
    static QVector<ID> enteringInRect(std::unordered_map<ID, Section *> &sections, QRectF &rect);
    static QVector<ID> enteringInRect(std::unordered_map<ID, Circle *> &circles, QRectF &rect);
    static QVector<ID> enteringInRect(std::unordered_map<ID, Point *> &points, QRectF &rect);

    // The function of checking the coordinates of the cursor and the figure to move
    static bool checkFigure(const QPointF point,const QPointF cursor ,const qreal zoom);
    static bool checkFigure(const QPointF center, qreal radius, const QPointF cursor,const qreal zoom);
    static bool checkFigure(const QPointF startPoint, const QPointF endPoint, const QPointF cursor,const qreal zoom);

    [[maybe_unused]] static bool checkFigure(const QPointF startPoint, const QPointF endPoint, const QPointF centerPoint,const QPointF cursor,const qreal zoom);

    // Auxiliary function for checking cursor and line
    static double distancePointToSection(const QPointF cursor, const QPointF startPoint, const QPointF endPoint);
};

#endif //OURPAINT_CLOSESPOINT_H