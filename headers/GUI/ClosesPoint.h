#ifndef OURPAINT_CLOSESPOINT_H
#define OURPAINT_CLOSESPOINT_H

#include <vector>
#include <limits>
#include <cmath>
#include <QPointF>
#include "paint.h"

// Класс для поиска ближайших точек к курсору
class ClosesPoint {

public:

    // Функция поиска ближайшей точки
    static QPointF findClosestPoint(const std::list<Point> &points) {
        if (points.size()>0) {
            QPointF closest;
            double minDistance = std::numeric_limits<float>::max();

            // Берем координаты курсора
            int cursorX = Scaling::logicCursorX();
            int cursorY = Scaling::logicCursorY();


            // Проходимся по всем точкам вычисляя дистанцию
            for (const auto &pt: points) {
                double distance = std::sqrt(std::pow(pt.x - cursorX, 2) + std::pow(pt.y - cursorY, 2));
                if (distance < minDistance) {
                    minDistance = distance;
                    closest = QPointF(pt.x, pt.y);
                }
            }

            return closest;
        }

        return {0, 0};
    }


    // Функция проверки координат курсора и точки для перемещения
    static bool checkFigure(double x, double y) {
        double RANGE = 3.0; // Задаем погрешность

        double logicalX = Scaling::logicCursorX();
        double logicalY = Scaling::logicCursorY();

        if (std::abs(logicalX - x) <= RANGE &&
            std::abs(logicalY - y) <= RANGE) {

            return true;
        }

        return false;
    }

// Функция проверки координат курсора и круга для перемещения
    static bool checkFigure(double x0, double y0, double r) {
        int cursorX = Scaling::logicCursorX();
        int cursorY = Scaling::logicCursorY();

        double dx = (cursorX - x0);
        double dy = (cursorY - y0);

        double distance = std::sqrt(dx * dx + dy * dy);

        double RANGE = 5.0; // Погрешность

        if (std::abs(distance - r) <= RANGE) {
            return true;
        }

        return false;
    }

// Функция проверки координат курсора и прямой для перемещения
    static bool checkFigure(double x0, double y0, double x1, double y1) {

        // Вычисляем вектор направления между начальной и конечной точками
        double dx = x1 - x0;
        double dy = y1 - y0;
        double dist = sqrt(dx * dx + dy * dy);

        if (dist > 7) {
            // Нормализуем вектор направления
            double unitX = dx / dist;
            double unitY = dy / dist;

            // Сдвигаем начальную и конечную точки на 7 в сторону друг друга
            // Чтобы не задеть точку
            x0 += unitX * 7;
            y0 += unitY * 7;
            x1 -= unitX * 7;
            y1 -= unitY * 7;
        }

        double RANGE = 5.0; // Погрешность

        double mouseX = Scaling::logicCursorX();
        double mouseY = Scaling::logicCursorY();

        double distance = distancePointToSection(mouseX, mouseY, x0, y0, x1, y1);

        if (distance <= RANGE) {
            return true;
        }

        return false;
    }

// Вспомогательная функция для проверки курсора и линии
    static double distancePointToSection(double px, double py, double x0, double y0, double x1, double y1) {
        double dx = x1 - x0;
        double dy = y1 - y0;

        if (dx * dx + dy * dy == 0.0) {
            dx = px - x0;
            dy = py - y0;
            return std::sqrt(dx * dx + dy * dy);
        }

        // Параметр t для проекции
        double t = ((px - x0) * dx + (py - y0) * dy) / (dx * dx + dy * dy);

        // Погрешность
        t = std::max(0.0, std::min(1.0, t));

        // Ближайшая точка на отрезке
        double nearestX = x0 + t * dx;
        double nearestY = y0 + t * dy;

        // Расстояние до ближайшей точки
        dx = px - nearestX;
        dy = py - nearestY;

        return std::sqrt(dx * dx + dy * dy);
    }


};

#endif //OURPAINT_CLOSESPOINT_H