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
    static QPointF findClosestPoint(std::unordered_map<ID,  Point*>& points,double yourX,double yourY);

    // Checking for entry into a rectangle

    [[maybe_unused]] static std::vector<ID>  enteringInRect(std::unordered_map<ID,  Point*>& points,QRectF &rect,std::vector<ID>&);
    [[maybe_unused]] static std::vector<ID>  enteringInRect(std::unordered_map<ID, Section*>& sections,QRectF &rect,std::vector<ID>&);
    [[maybe_unused]] static std::vector<ID>  enteringInRect(std::unordered_map<ID,  Circle*>& circles,QRectF &rect,std::vector<ID>&);
    [[maybe_unused]] static std::vector<ID>  enteringInRect(std::unordered_map<ID,  Arc*>& arcs,QRectF &rect,std::vector<ID>&);

    static std::vector<ID> enteringInRect(std::unordered_map<ID, Arc *> &arcs, QRectF &rect);
    static std::vector<ID> enteringInRect(std::unordered_map<ID, Section *> &sections, QRectF &rect);
    static std::vector<ID> enteringInRect(std::unordered_map<ID, Circle *> &circles, QRectF &rect);
    static  std::vector<ID> enteringInRect(std::unordered_map<ID, Point *> &points, QRectF &rect);

    // The function of checking the coordinates of the cursor and the figure to move
    static bool checkFigure(double pointX, double pointY, double yourX,double yourY,double zoom);
    static bool checkFigure(double x0, double y0, double r, double yourX,double yourY,double zoom);
    static bool checkFigure(double x0, double y0, double x1, double y1,double yourX,double yourY,double zoom);

    [[maybe_unused]] static bool checkFigure(double x0, double y0, double x1, double y1,double xc, double yc,double yourX,double yourY,double zoom);

    // Auxiliary function for checking cursor and line
    static double distancePointToSection(double px, double py, double x0, double y0, double x1, double y1);
};

#endif //OURPAINT_CLOSESPOINT_H