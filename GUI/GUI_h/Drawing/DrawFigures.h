#ifndef OURPAINT_DRAWFIGURES_H
#define OURPAINT_DRAWFIGURES_H

#include <QPainter>
#include <unordered_set>
#include <cmath>

#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "GeometricObjects.h"

//     Класс для отрисовки фигур
//     Преобразование координат с учётом масштабирования и инвертированной оси Y,
//     а также использует дополнительные функции для выделения объектов


class DrawFigures {
private:
    static QPen MyColor;  // Статическая переменная для хранения текущего цвета кисти

public:
    // Установить цвет кисти для отрисовки
    static void setPen(const QPen &color) {
        MyColor = color;
    }

    // Функция для отрисовки точек
    static void drawPoint(QPainter &painter, const std::unordered_map<ID, Point *> &points,
                          std::vector<ID> &vec_id) {
        if (points.empty()) { return; }

        painter.setBrush(QBrush(Qt::black));
        painter.setPen(Qt::NoPen);

        short int pointRadius = 1;

        if (vec_id.empty()) {
            for (const auto &pt : points) {
                const Point *point = pt.second;
                painter.drawEllipse(QPointF(Scaling::scaleCoordinate(point->x),
                                            Scaling::scaleCoordinate(-point->y)),
                                    pointRadius, pointRadius);
            }


        } else {

            std::unordered_set<ID> selectedIds(vec_id.begin(), vec_id.end());

            for (const auto &pt : points) {
                const Point *point = pt.second;

                QPointF logicPoint(Scaling::scaleCoordinate(point->x), Scaling::scaleCoordinate(-point->y));

                bool isSelected = selectedIds.contains(pt.first);
                pointRadius = isSelected ? 2 : 1;

                painter.setBrush(QBrush(Qt::black));
                painter.setPen(Qt::NoPen);

                painter.drawEllipse(logicPoint, pointRadius, pointRadius);

                if (isSelected) {
                    DrawAdditionalInf::drawPointID(painter, logicPoint, pt.first);
                    DrawAdditionalInf::drawPointGlow(painter, logicPoint);
                }
            }
        }
    }


    // Функция для отрисовки кругов
    static void drawCircle(QPainter &painter, const std::unordered_map<ID, Circle *> &circles, std::vector<ID> &vec_id) {
        if (circles.empty()) { return; }

        QPen pen(Qt::black);
        pen.setWidth(1);
        pen.setJoinStyle(Qt::RoundJoin);
        pen.setCapStyle(Qt::RoundCap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(pen);

        if (vec_id.empty()) {
            for (const auto &cir : circles) {
                const Circle *circle = cir.second;
                painter.drawEllipse(QPointF(Scaling::scaleCoordinate(circle->center->x),
                                            Scaling::scaleCoordinate(-circle->center->y)),
                                    Scaling::scaleCoordinate(circle->r),
                                    Scaling::scaleCoordinate(circle->r));
            }
        }else {

            std::unordered_set<ID> selectedIds(vec_id.begin(), vec_id.end());


            for (const auto &pair: circles) {
                const ID &id = pair.first;
                const Circle *circle = pair.second;

                double scaledRadius = Scaling::scaleCoordinate(circle->r);
                QPointF logicCenter(Scaling::scaleCoordinate(circle->center->x),
                                    Scaling::scaleCoordinate(-circle->center->y));

                bool selected = selectedIds.contains(id);

                pen.setWidth(selected ? 2 : 1);
                painter.setPen(pen);


                painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

                if (selected) {
                    DrawAdditionalInf::drawCircleID(painter, logicCenter, scaledRadius, id);
                    DrawAdditionalInf::drawCircleGlow(painter, logicCenter, scaledRadius);
                }
            }
        }

    }

    // Функция для отрисовки линий
    static void drawSection(QPainter &painter, const std::unordered_map<ID, Section *> &sections, std::vector<ID> &vec_id) {
        if (sections.empty()) { return; }

        painter.setRenderHint(QPainter::Antialiasing);
        QPen currentPen(Qt::black);
        currentPen.setWidth(1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        if (vec_id.empty()) {
            for (const auto &sec : sections) {
                const Section *section = sec.second;
                QPointF start(Scaling::scaleCoordinate(section->beg->x),
                              Scaling::scaleCoordinate(-section->beg->y));
                QPointF end(Scaling::scaleCoordinate(section->end->x),
                            Scaling::scaleCoordinate(-section->end->y));
                painter.drawLine(start, end);

                // Отрисовка длины
                DrawAdditionalInf::drawCoordinateLine(painter, start, end);
            }
        }else {
            std::unordered_set<ID> selectedIds(vec_id.begin(), vec_id.end());


            for (const auto &pair: sections) {
                const ID &id = pair.first;
                const Section *section = pair.second;

                QPointF start(Scaling::scaleCoordinate(section->beg->x),
                              Scaling::scaleCoordinate(-section->beg->y));
                QPointF end(Scaling::scaleCoordinate(section->end->x),
                            Scaling::scaleCoordinate(-section->end->y));

                bool selected = selectedIds.contains(id);

                currentPen.setWidth(selected ? 2 : 1);
                painter.setPen(currentPen);

                painter.drawLine(start, end);

                DrawAdditionalInf::drawCoordinateLine(painter, start, end);

                if (selected) {
                    DrawAdditionalInf::drawSectionID(painter, start, end, id);
                    DrawAdditionalInf::drawSectionGlow(painter, start, end);
                }
            }
        }

    }


    // Функция для рисования сектора
    static void drawSector(QPainter &painter, bool selected) {
        // Настройка пера
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);


        // Сброс цвета
        MyColor = QPen(Qt::black);
    }

    // Вспомогательная функция для угла в градусах
    static double angleBetween(const QPointF &center, const QPointF &point) {
        return std::atan2(point.y() - center.y(), point.x() - center.x()) * 180.0 / M_PI;
    }

    static double normalizeAngle(double angle) {
        while (angle < 0) angle += 360;
        while (angle >= 360) angle -= 360;
        return angle;
    }

    // Функция для рисования дуги
    static void drawArc(QPainter &painter, const std::unordered_map<ID,  Arc*> &arcs,std::vector<ID> &vec_id) {
        if (arcs.size() == 0) { return; }
        else {
            // Настройка пера
            QPen currentPen = QPen(Qt::black);
            currentPen.setWidth(1);
            currentPen.setCapStyle(Qt::RoundCap);
            painter.setPen(currentPen);

            if (!vec_id.empty()) {
                std::unordered_set<ID> selectedIds(vec_id.begin(), vec_id.end());

                for (const auto &elem: arcs) {
                    const ID &id = elem.first;
                    const Arc *arc = elem.second;

                    bool selected = selectedIds.contains(id);

                    currentPen.setWidth(selected ? 2 : 1);
                    painter.setPen(currentPen);

                    QPointF center(arc->center->x, arc->center->y);
                    QPointF start(arc->beg->x, arc->beg->y);
                    QPointF end(arc->end->x, arc->end->y);

                    double radius = std::hypot(start.x() - center.x(), start.y() - center.y());

                    double startAngleDeg = normalizeAngle(angleBetween(center, start));
                    double endAngleDeg = normalizeAngle(angleBetween(center, end));
                    double spanAngleDeg = endAngleDeg - startAngleDeg;

                    if (spanAngleDeg <= 0) spanAngleDeg += 360;

                    QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
                    int qtStart = static_cast<int>(startAngleDeg * 16);
                    int qtSpan = static_cast<int>(spanAngleDeg * 16);

                    painter.drawArc(rect, qtStart, qtSpan);
                }
            } else {
                // Учитываем масштабирование
                for (const auto &elem: arcs) {

                    const Arc *arc = elem.second;

                    QPointF center(Scaling::scaleCoordinate(arc->center->x), Scaling::scaleCoordinate(-arc->center->y));
                    QPointF start(Scaling::scaleCoordinate(arc->beg->x), Scaling::scaleCoordinate(-arc->beg->y));
                    QPointF end(Scaling::scaleCoordinate(arc->end->x), Scaling::scaleCoordinate(-arc->end->y));

                    double radius = std::hypot(start.x() - center.x(), start.y() - center.y());

                    double startAngleDeg = normalizeAngle(angleBetween(center, start));
                    double endAngleDeg = normalizeAngle(angleBetween(center, end));
                    double spanAngleDeg = endAngleDeg - startAngleDeg;

                    if (spanAngleDeg <= 0) spanAngleDeg += 360;

                    QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
                    int qtStart = static_cast<int>(startAngleDeg * 16);
                    int qtSpan = static_cast<int>(spanAngleDeg * 16);

                    painter.drawArc(rect, qtStart, qtSpan);
                }
            }
        }
    }


    /**** Функции для одиночной отрисовки (мышью) */

    // Функция для отрисовки одной точки
    static void drawPoint(QPainter &painter, QPointF point) {
        short int pointRadius =  1;

        QPointF logicPoint(Scaling::scaleCoordinate(point.x()), Scaling::scaleCoordinate(-point.y()));

        QBrush pointBrush = Qt::black;
        painter.setBrush(pointBrush);
        painter.setPen(Qt::NoPen);


        painter.drawEllipse(logicPoint, pointRadius, pointRadius);
        MyColor = QPen(Qt::black);
    }


    // Функция для отрисовки круга
    static void drawCircle(QPainter &painter, QPointF center, double radius) {
        double scaledRadius = radius * Scaling::getZoom();
        QPointF logicCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));

        QPen currentPen = QPen(Qt::black);
        currentPen.setWidth( 1);
        currentPen.setJoinStyle(Qt::RoundJoin);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);
        painter.setBrush(Qt::NoBrush);

        painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для рисования линии
    static void drawSection(QPainter &painter, QPointF beg, QPointF end) {
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        QPointF start(Scaling::scaleCoordinate(beg.x()), Scaling::scaleCoordinate(-beg.y()));
        QPointF endLogic(Scaling::scaleCoordinate(end.x()), Scaling::scaleCoordinate(-end.y()));

        painter.drawLine(start, endLogic);

        currentPen = QColor(Qt::black);
        painter.setPen(currentPen);

        // Длина
        DrawAdditionalInf::drawCoordinateLine(painter, start, endLogic);

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для рисования сектора
    static void drawSector(QPainter &painter, QPointF center, double radius, double startAngleDeg, double spanAngleDeg) {
        // Настройка пера
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        // Масштабирование центра
        QPointF scaledCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));
        double scaledRadius = Scaling::scaleCoordinate(radius);

        // Прямоугольник, в который вписана дуга
        QRectF arcRect(scaledCenter.x() - scaledRadius, scaledCenter.y() - scaledRadius,
                       2 * scaledRadius, 2 * scaledRadius);


        double startAngle16 = Scaling::scaleCoordinate(-startAngleDeg); // Минус, т.к. Y идёт вниз
        double spanAngle16 = Scaling::scaleCoordinate(-spanAngleDeg);

        painter.drawPie(arcRect, startAngle16, spanAngle16);

        // Сброс цвета
        MyColor = QPen(Qt::black);
    }

    static void drawHalfCircle(QPainter &painter, QPointF start, QPointF cursor) {
        QPen pen(Qt::black);
        pen.setWidth(1);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);

        // Масштабирование с переворотом Y
        QPointF p0 = QPointF(Scaling::scaleCoordinate(start.x()), Scaling::scaleCoordinate(-start.y()));
        QPointF p1 = QPointF(Scaling::scaleCoordinate(cursor.x()), Scaling::scaleCoordinate(-cursor.y()));

        // Центр и радиус
        QPointF center = (p0 + p1) / 2.0;
        double radius = std::hypot(p1.x() - p0.x(), p1.y() - p0.y()) / 2.0;
        QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

        // Угол к стартовой точке
        double angleStart = std::atan2(-p0.y() + center.y(), p0.x() - center.x()) * 180.0 / M_PI;

        // Векторы от центра к p0 и p1
        QPointF v0 = p0 - center;
        QPointF v1 = p1 - center;

        // Векторное произведение — инвертируем знак, так как Y уже инвертирован
        double cross = -(v0.x() * v1.y() - v0.y() * v1.x());

        // Определяем сторону дуги по знаку
        double span = (cross > 0) ? 180.0 : -180.0;

        int qtStartAngle = static_cast<int>(angleStart * 16);
        int qtSpanAngle = static_cast<int>(span * 16);

        painter.drawArc(rect, qtStartAngle, qtSpanAngle);
    }



    static void drawArc(QPainter &painter, QPointF start, QPointF end, QPointF cursor) {
        QPen pen(Qt::black);
        pen.setWidth(1);
        pen.setCapStyle(Qt::RoundCap);
        painter.setPen(pen);

        // Масштабирование с переворотом Y
        QPointF p0 = QPointF(Scaling::scaleCoordinate(start.x()), Scaling::scaleCoordinate(-start.y()));
        QPointF p1 = QPointF(Scaling::scaleCoordinate(cursor.x()), Scaling::scaleCoordinate(-cursor.y()));

        // Центр и радиус
        QPointF center = (p0 + p1) / 2.0;
        double radius = std::hypot(p1.x() - p0.x(), p1.y() - p0.y()) / 2.0;
        QRectF rect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);

        // Угол к стартовой точке
        double angleStart = std::atan2(-p0.y() + center.y(), p0.x() - center.x()) * 180.0 / M_PI;

        // Векторы от центра к p0 и p1
        QPointF v0 = p0 - center;
        QPointF v1 = p1 - center;

        // Векторное произведение — инвертируем знак, так как Y уже инвертирован
        double cross = -(v0.x() * v1.y() - v0.y() * v1.x());

        // Определяем сторону дуги по знаку
        double span = (cross > 0) ? 180.0 : -180.0;

        int qtStartAngle = static_cast<int>(angleStart * 16);
        int qtSpanAngle = static_cast<int>(span * 16);

        painter.drawArc(rect, qtStartAngle, qtSpanAngle);
    }

    // Функция для рисования прямоугольника
    static void drawRectangle(QPainter &painter, QPointF &X, QPointF &Y) {
        // Настройка пера
        QPen currentPen = QPen(Qt::blue);
        currentPen.setCapStyle(Qt::RoundCap); // Устанавливаем стиль окончания линии
        painter.setPen(currentPen);

        QPointF start(Scaling::scaleCoordinate(X.x()), Scaling::scaleCoordinate(-X.y()));
        QPointF end(Scaling::scaleCoordinate(Y.x()), Scaling::scaleCoordinate(-Y.y()));

        painter.setBrush(
                QBrush(QColor(173, 216, 230, 128), Qt::SolidPattern)); // Цвет: светло-голубой с 50% прозрачностью

        // Рисуем прямоугольник
        painter.drawRect(QRectF(start, end));

        // Сброс цвета (если нужно)
        MyColor = QPen(Qt::black);
    }


};

#endif // OURPAINT_DRAWFIGURES_H
