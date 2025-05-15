#ifndef OURPAINT_DRAWFIGURES_H
#define OURPAINT_DRAWFIGURES_H

#include <QPainter>
#include <unordered_set>
#include <cmath>

#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "GeometricObjects.h"
#include "Colors.h"

// A class for drawing shapes
// Coordinate transformation taking into account scaling and the inverted Y-axis,
// and also uses additional functions to select objects


class DrawFigures {
private:
    static QPen MyColor;

public:
    static void setPen(const QPen &color);

    static double angleBetween(const QPointF &center, const QPointF &point);

    // Function for drawing points
    static void drawPoint(QPainter &painter, const std::unordered_map<ID, Point *> &points,
                                             const std::unordered_map<ID, Color> &vec_id);


    // Function for drawing circles
    static void drawCircle(QPainter &painter, const std::unordered_map<ID, Circle *> &circles,
                                              const  std::unordered_map<ID,Color> &vec_id);


    // Function for drawing lines
    static void drawSection(QPainter &painter, const std::unordered_map<ID, Section *> &sections,
                                               const std::unordered_map<ID,Color> &vec_id);



    // Function for drawing arcs
    static void drawArc(QPainter &painter, const std::unordered_map<ID,  Arc*> &arcs,
                                           const  std::unordered_map<ID,Color> &vec_id);


    /**** Functions for single rendering (with mouse) *****/

    static void drawPoint(QPainter &painter, QPointF point);
    static void drawCircle(QPainter &painter, QPointF center, double radius);
    static void drawSection(QPainter &painter, QPointF beg, QPointF end);
    static void drawArc(QPainter &painter, QPointF &s, QPointF &e,QPointF& c);
    static void drawRectangle(QPainter &painter, QPointF &X, QPointF &Y);


};

#endif // OURPAINT_DRAWFIGURES_H
