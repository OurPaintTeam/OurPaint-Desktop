#ifndef OURPAINT_DRAWFIGURES_H
#define OURPAINT_DRAWFIGURES_H

#include <QPainter>
#include <unordered_set>

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
    static void drawPoint(QPainter &painter, const std::unordered_map<ID,  Point *> &points,
                          std::vector<ID> &vec_id) {
        if (points.size() == 0) { return; }
        else {

            QBrush pointBrush = (QColor(Qt::black));
            painter.setBrush(pointBrush);
            painter.setBrush(Qt::black);
            painter.setPen(Qt::NoPen);

            if (!vec_id.empty()) {

                    std::unordered_set<ID> selectedIds(vec_id.begin(), vec_id.end());

                    for (const auto &pt : points) {
                        const Point *point = pt.second;

                        QPointF logicPoint(Scaling::scaleCoordinate(point->x), Scaling::scaleCoordinate(-point->y));

                        bool isSelected = selectedIds.contains(pt.first);
                        short int pointRadius = isSelected ? 2 : 1;


                        painter.drawEllipse(logicPoint, pointRadius, pointRadius);

                        if (isSelected) {
                            DrawAdditionalInf::drawPointID(painter, logicPoint, pt.first);
                            DrawAdditionalInf::drawPointGlow(painter, logicPoint);
                        }
                    }

            } else {

                short int pointRadius = 1;

                // Учитываем масштабирование
                for (const auto &pt: points) {
                    const Point *point = pt.second;
                    painter.drawEllipse(QPointF(Scaling::scaleCoordinate(point->x),
                                                Scaling::scaleCoordinate(-point->y)),
                                        pointRadius, pointRadius);
                }
            }
        }
    }


    // Функция для отрисовки кругов
    static void drawCircle(QPainter &painter, const std::unordered_map<ID,  Circle *> &circles,std::vector<ID> &vec_id) {
        if (circles.size() == 0) { return; }
        else {
            QPen pen(Qt::black);
            pen.setWidth(1);
            pen.setJoinStyle(Qt::RoundJoin);
            pen.setCapStyle(Qt::RoundCap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setBrush(Qt::NoBrush);
            painter.setPen(pen);

            if(!vec_id.empty()) {
                std::unordered_set<ID> selectedIds(vec_id.begin(), vec_id.end());

                for (const auto &pair: circles) {
                    const ID &id = pair.first;
                    const Circle *circle = pair.second;

                    double scaledRadius = Scaling::scaleCoordinate(circle->r);
                    QPointF logicCenter(Scaling::scaleCoordinate(circle->center->x),
                                        Scaling::scaleCoordinate(-circle->center->y));

                    bool selected = selectedIds.contains(id);

                    QPen currentPen = selected ? QPen(Qt::blue) : QPen(Qt::black);
                    currentPen.setWidth(selected ? 2 : 1);
                    painter.setPen(currentPen);

                    if (selected) {
                        DrawAdditionalInf::drawCircleID(painter, logicCenter, scaledRadius,id);
                        DrawAdditionalInf::drawCircleGlow(painter, logicCenter, scaledRadius);
                    }

                    painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);
                }
            }else {
                // Учитываем масштабирование
                for (const auto &cir: circles) {
                    const Circle *circle = cir.second;
                    painter.drawEllipse(QPointF(Scaling::scaleCoordinate(circle->center->x),
                                                Scaling::scaleCoordinate(-circle->center->y)),
                                        Scaling::scaleCoordinate(circle->r),
                                        Scaling::scaleCoordinate(circle->r));
                }
            }

        }
    }


    // Функция для отрисовки линий
    static void drawSection(QPainter &painter, const std::unordered_map<ID,  Section *> &sections,std::vector<ID> &vec_id) {
        if (sections.size() == 0) { return; }
        else {
            painter.setRenderHint(QPainter::Antialiasing);  // Включаем сглаживание
            QPen currentPen = QPen(Qt::black);
            currentPen.setWidth(1);
            currentPen.setCapStyle(Qt::RoundCap);
            painter.setPen(currentPen);

            if(!vec_id.empty()) {
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
            else{
                // Учитываем масштабирование
                for (const auto &sec: sections) {
                    const Section *section = sec.second;
                    QPointF start(Scaling::scaleCoordinate(section->beg->x),
                                  Scaling::scaleCoordinate(-section->beg->y));
                    QPointF end(Scaling::scaleCoordinate(section->end->x), Scaling::scaleCoordinate(-section->end->y));
                    painter.drawLine(start, end);

                    // Отрисовка длины
                    DrawAdditionalInf::drawCoordinateLine(painter, start, end);
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

    // Функция для рисования дуги
    static void drawArc(QPainter &painter, bool selected) {
        // Настройка пера
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);


        // Сброс пера в черный
        MyColor = QPen(Qt::black);
    }

    //////////// Функции для одиночной отрисовки (мышью) ]

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

    // Функция для рисования дуги
    static void drawArc(QPainter &painter, QPointF center, double radius, double startAngleDeg, double spanAngleDeg,
                        bool selected) {
        // Настройка пера
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        // Масштабирование координат
        QPointF scaledCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));
        double scaledRadius = Scaling::scaleCoordinate(radius);

        // Прямоугольник, в который вписана дуга
        QRectF arcRect(
                scaledCenter.x() - scaledRadius,
                scaledCenter.y() - scaledRadius,
                2 * scaledRadius,
                2 * scaledRadius
        );

        // QPainter работает с углами в 1/16 градуса, по часовой стрелке
        int startAngle16 = static_cast<int>(-startAngleDeg * 16); // знак минус — переворот по Y
        int spanAngle16 = static_cast<int>(-spanAngleDeg * 16);

        painter.drawArc(arcRect, startAngle16, spanAngle16);

        // Сброс пера в черный
        MyColor = QPen(Qt::black);
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
