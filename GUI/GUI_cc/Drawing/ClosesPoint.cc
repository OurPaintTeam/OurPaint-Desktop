#include "ClosesPoint.h"

// Функция поиска ближайшей точки
QPointF ClosesPoint::findClosestPoint(std::unordered_map<ID, Point *> &points) {
    if (points.size() > 0) {
        QPointF closest;
        double minDistance = std::numeric_limits<float>::max();

        // Берем координаты курсора
        double cursorX = Scaling::logicCursorX();
        double cursorY = Scaling::logicCursorY();


        // Проходимся по всем точкам вычисляя дистанцию
        for (const auto &pt: points) {
            const Point *point = pt.second;
            double distance = std::sqrt(std::pow(point->x - cursorX, 2) + std::pow(point->y - cursorY, 2));
            if (distance < minDistance) {
                minDistance = distance;
                closest = QPointF(point->x, point->y);
            }
        }

        return closest;
    }

    return QPointF{};
}


// Функция проверки координат курсора и точки для перемещения
bool ClosesPoint::checkFigure(double x, double y) {
    double RANGE = 3.0 / Scaling::getZoom(); // Задаем погрешность

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

    double RANGE = 3.0 / Scaling::getZoom(); // Погрешность

    if (std::abs(distance - r) <= RANGE) {
        return true;
    }

    return false;
}


// Функция проверки координат курсора и арки для перемещения
bool ClosesPoint::checkFigure(double x0, double y0, double x1, double y1, double xc, double yc) {
    double cursorX = Scaling::logicCursorX();
    double cursorY = Scaling::logicCursorY();

    double r = std::hypot(x0 - xc, y0 - yc);
    double distToCenter = std::hypot(cursorX - xc, cursorY - yc);

    double RANGE = 3.0 / Scaling::getZoom();
    if (std::abs(distToCenter - r) > RANGE) { return false; }

    double angleEnd = QLineF(QPointF(xc, yc), QPointF(x0, y0)).angle();
    double angleStart = QLineF(QPointF(xc, yc), QPointF(x1, y1)).angle();
    double angleCursor = QLineF(QPointF(xc, yc), QPointF(cursorX, cursorY)).angle();

    angleStart = fmod(angleStart + 360.0, 360.0);
    angleEnd = fmod(angleEnd + 360.0, 360.0);
    angleCursor = fmod(angleCursor + 360.0, 360.0);

    if (angleStart > angleEnd) {
        return (angleCursor >= angleStart || angleCursor <= angleEnd);
    }

        return (angleCursor >= angleStart && angleCursor <= angleEnd);
}


// Функция проверки координат курсора и прямой для перемещения
bool ClosesPoint::checkFigure(double x0, double y0, double x1, double y1) {

    double RANGE = 5.0 / Scaling::getZoom(); // Погрешность

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

std::vector<ID>
ClosesPoint::enteringInRect(std::unordered_map<ID, Point *> &points, QRectF &rect, std::vector<ID> &vec_id) {

    vec_id.clear();

    for (const auto &pt: points) {
        const Point *point = pt.second;
        QPointF p(point->x, point->y);
        if (rect.contains(p)) {
            vec_id.push_back(pt.first);
        }
    }
    return vec_id;
}

std::vector<ID>
ClosesPoint::enteringInRect(std::unordered_map<ID, Circle *> &circles, QRectF &rect, std::vector<ID> &vec_id) {
    vec_id.clear();
    for (const auto &pair: circles) {
        const Circle *circle = pair.second;
        QPointF center(circle->center->x, circle->center->y);
        double radius = circle->r;

        QRectF circleBounds(center.x() - radius, center.y() - radius,
                            2 * radius, 2 * radius);

        if (rect.contains(circleBounds)) {
            vec_id.push_back(pair.first);
        }
    }
    return vec_id;
}

std::vector<ID>
ClosesPoint::enteringInRect(std::unordered_map<ID, Section *> &sections, QRectF &rect, std::vector<ID> &vec_id) {
    vec_id.clear();
    for (const auto &sec: sections) {
        const Section *section = sec.second;
        QPointF p1(section->beg->x, section->beg->y);
        QPointF p2(section->end->x, section->end->y);

        if (rect.contains(p1) || rect.contains(p2)) {
            vec_id.push_back(sec.first);
        } else {
            QLineF line(p1, p2);
            QPolygonF polygon;
            polygon << rect.topLeft() << rect.topRight()
                    << rect.bottomRight() << rect.bottomLeft();

            for (int i = 0; i < polygon.size(); ++i) {
                QLineF edge(polygon[i], polygon[(i + 1) % polygon.size()]);
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


std::vector<ID>
ClosesPoint::enteringInRect(std::unordered_map<ID, Arc *> &arcs, QRectF &rect, std::vector<ID> &vec_id) {
    vec_id.clear();

    for (const auto &elem : arcs) {
        const ID &id = elem.first;
        const Arc *arc = elem.second;

        QPointF beg(arc->beg->x, arc->beg->y);
        QPointF end(arc->end->x, arc->end->y);
        QPointF center(arc->center->x, arc->center->y);

        double r = QLineF(center, beg).length();
        double angleStart = QLineF(center, end).angle();
        double angleEnd = QLineF(center, beg).angle();

        angleStart = fmod(angleStart + 360.0, 360.0);
        angleEnd = fmod(angleEnd + 360.0, 360.0);

        double span = angleEnd - angleStart;
        if (span <= 0.1) { span += 360.0; }
        double midAngleDeg = fmod(angleStart + span / 2.0, 360.0);
        double midAngleRad = qDegreesToRadians(midAngleDeg);

        QPointF mid(
                center.x() + r * std::cos(midAngleRad),
                center.y() - r * std::sin(midAngleRad)
        );

        if (rect.contains(beg) || rect.contains(end) || rect.contains(center) || rect.contains(mid)) {
            vec_id.push_back(id);
        }
    }

    return vec_id;
}

