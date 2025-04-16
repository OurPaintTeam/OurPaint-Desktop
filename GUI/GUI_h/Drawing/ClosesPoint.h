#ifndef OURPAINT_CLOSESPOINT_H
#define OURPAINT_CLOSESPOINT_H

#include <vector>
#include <limits>
#include <cmath>
#include <QPointF>

#include "Paint.h"
#include "Scaling.h"

// Класс для поиска ближайших точек к курсору
class ClosesPoint {

public:

    // Функция поиска ближайшей точки
    static QPointF findClosestPoint(const std::list<Point> &points);

    // Функция проверки координат курсора и точки для перемещения
    static bool checkFigure(double x, double y);

    // Функция проверки координат курсора и круга для перемещения
    static bool checkFigure(double x0, double y0, double r);

    // Функция проверки координат курсора и прямой для перемещения
    static bool checkFigure(double x0, double y0, double x1, double y1);

    // Вспомогательная функция для проверки курсора и линии
    static double distancePointToSection(double px, double py, double x0, double y0, double x1, double y1);


};

#endif //OURPAINT_CLOSESPOINT_H