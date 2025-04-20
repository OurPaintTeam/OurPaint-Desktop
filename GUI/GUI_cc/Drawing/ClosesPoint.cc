#include <QRectF>
#include <QLineF>
#include <QPolygonF>
#include "ClosesPoint.h"

// Функция поиска ближайшей точки
QPointF ClosesPoint::findClosestPoint(const std::list<Point> &points) {
    if (points.size()>0) {
        QPointF closest;
        double minDistance = std::numeric_limits<float>::max();

        // Берем координаты курсора
        double cursorX = Scaling::logicCursorX();
        double cursorY = Scaling::logicCursorY();


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
bool ClosesPoint::checkFigure(double x, double y) {
    double RANGE = 3.0/Scaling::getZoom(); // Задаем погрешность

    double logicalX = Scaling::logicCursorX();
    double logicalY = Scaling::logicCursorY();

    double dx = logicalX - x;
    double dy = logicalY - y;
    return std::sqrt(dx * dx + dy * dy) <= RANGE;
}


// Функция проверки координат курсора и круга для перемещения
 bool ClosesPoint::checkFigure(double x0, double y0, double r) {
    double cursorX = Scaling::logicCursorX();
    double cursorY = Scaling::logicCursorY();

    double dx = (cursorX - x0);
    double dy = (cursorY - y0);
    double distance = std::sqrt(dx * dx + dy * dy);

    double RANGE = 3.0/Scaling::getZoom();; // Погрешность

    if (std::abs(distance - r) <= RANGE) {
        return true;
    }

    return false;
}


// Функция проверки координат курсора и прямой для перемещения
bool ClosesPoint::checkFigure(double x0, double y0, double x1, double y1) {

    // Вычисляем вектор направления между начальной и конечной точками
    double dx = x1 - x0;
    double dy = y1 - y0;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist > 3.0) {
        // Нормализуем вектор направления
        double unitX = dx / dist;
        double unitY = dy / dist;

        // Сдвигаем начальную и конечную точки на 1 в сторону друг друга
        // Чтобы не задеть точку
        x0 += unitX * 1;
        y0 += unitY * 1;
        x1 -= unitX * 1;
        y1 -= unitY * 1;
    }

    double RANGE = 5.0/Scaling::getZoom(); // Погрешность

    double mouseX = Scaling::logicCursorX();
    double mouseY = Scaling::logicCursorY();

    double distance = distancePointToSection(mouseX, mouseY, x0, y0, x1, y1);

    if (distance <= RANGE) {
        return true;
    }

    return false;
}


// Вспомогательная функция для проверки курсора и линии
double ClosesPoint::distancePointToSection(double px, double py, double x0, double y0, double x1, double y1) {
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

void ClosesPoint::enteringInRect(const std::list<Point> &points, QRectF &rect) {

    for (const Point &p : points) {
        QPointF pt(p.x, p.y);
        if (rect.contains(pt)) {

        }
    }
}

void ClosesPoint::enteringInRect(const std::list<Circle> &circles, QRectF &rect) {
    for (const Circle &c : circles) {
        QPointF center(c.center->x, c.center->y);
        double radius = c.R;

        QRectF circleBounds(center.x() - radius, center.y() - radius,
                            2 * radius, 2 * radius);

        if (rect.contains(circleBounds)) {

        }
    }
}

void ClosesPoint::enteringInRect(const std::list<Section> &sections, QRectF &rect) {
    for (const Section &s : sections) {
        QPointF p1(s.beg->x, s.beg->y);
        QPointF p2(s.end->x, s.end->y);

        if (rect.contains(p1) || rect.contains(p2)) {

        }
        else {
            QLineF line(p1, p2);
            QPolygonF polygon;
            polygon << rect.topLeft() << rect.topRight()
                    << rect.bottomRight() << rect.bottomLeft();

            for (int i = 0; i < polygon.size(); ++i) {
                QLineF edge(polygon[i], polygon[(i + 1) % polygon.size()]);
                QPointF intersection;
                if (line.intersects(edge, &intersection) == QLineF::BoundedIntersection) {

                    break;
                }
            }
        }
    }
}

