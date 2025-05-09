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

#include "GeometricObjects.h"
#include "Scaling.h"


// Класс для поиска ближайших точек к курсору
class ClosesPoint {

public:
    // Функция поиска ближайшей точки
    static QPointF findClosestPoint(std::unordered_map<ID,  Point*>& points);

    static std::vector<ID>  enteringInRect(std::unordered_map<ID,  Point*>& points,QRectF &rect,std::vector<ID>&);
    static std::vector<ID>  enteringInRect(std::unordered_map<ID, Section*>& sections,QRectF &rect,std::vector<ID>&);
    static std::vector<ID>  enteringInRect(std::unordered_map<ID,  Circle*>& circles,QRectF &rect,std::vector<ID>&);

    // Функция проверки координат курсора и точки для перемещения
    static bool checkFigure(double x, double y);

    // Функция проверки координат курсора и круга для перемещения
    static bool checkFigure(double x0, double y0, double r);

    // Функция проверки координат курсора и прямой для перемещения
    static bool checkFigure(double x0, double y0, double x1, double y1);

    // Функция проверки координат курсора и арки
    static bool checkFigure(double x0, double y0, double x1, double y1,double xc, double yc);

    // Вспомогательная функция для проверки курсора и линии
    static double distancePointToSection(double px, double py, double x0, double y0, double x1, double y1);

};

#endif //OURPAINT_CLOSESPOINT_H