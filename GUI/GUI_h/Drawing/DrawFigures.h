#ifndef OURPAINT_DRAWFIGURES_H
#define OURPAINT_DRAWFIGURES_H

#include <QPainter>
#include <list>

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
    static void drawPoint(QPainter &painter, const std::list<Point> &points) {
        if (points.size() == 0) { return; }

        // Настройка кисти и отключение обводки
        QBrush pointBrush = (QColor(Qt::black));
        painter.setRenderHint(QPainter::Antialiasing);  // Включаем сглаживание
        painter.setBrush(pointBrush);
        painter.setPen(Qt::NoPen);

        double pointRadius = 1;

        // Учитываем масштабирование
        for (const auto &pt: points) {
            painter.drawEllipse(QPointF(Scaling::scaleCoordinate(pt.x),
                                               Scaling::scaleCoordinate(-pt.y)),
                                                 pointRadius, pointRadius);}
    }


    // Функция для отрисовки кругов
    static void drawCircle(QPainter &painter, const std::list<Circle> &circles) {
        if (circles.size() == 0) { return; }

        // Настройка пера для рисования кругов
        QPen currentPen = QColor(Qt::black);
        currentPen.setWidth(1 );
        currentPen.setJoinStyle(Qt::RoundJoin);
        currentPen.setCapStyle(Qt::RoundCap);

        painter.setPen(currentPen);
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing);

        // Учитываем масштабирование
        for (const auto &cir: circles) {
            painter.drawEllipse(QPointF(Scaling::scaleCoordinate(cir.center->x),
                                               Scaling::scaleCoordinate(-cir.center->y)),
                                                 Scaling::scaleCoordinate(cir.R),
                                                 Scaling::scaleCoordinate(cir.R));
        }

        MyColor = QPen(Qt::black);  // Сброс цвета
    }


    // Функция для отрисовки линий
    static void drawSection(QPainter &painter, const std::list<Section> &sections) {
        if (sections.size() == 0) { return; }

        painter.setRenderHint(QPainter::Antialiasing);  // Включаем сглаживание
        QPen currentPen = QPen(Qt::black);
        currentPen.setWidth(1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        // Учитываем масштабирование
        for (const auto &sec: sections) {
            QPointF start(Scaling::scaleCoordinate(sec.beg->x), Scaling::scaleCoordinate(-sec.beg->y));
            QPointF end(Scaling::scaleCoordinate(sec.end->x), Scaling::scaleCoordinate(-sec.end->y));
            painter.drawLine(start, end);

            // Отрисовка длины
            DrawAdditionalInf::drawCoordinateLine(painter, start, end);
        }

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для рисования сектора
    static void drawSector(QPainter &painter,bool selected) {
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
    //////////// Функции для одиночной отрисовки (мышью) или если выделен обьект

    // Функция для отрисовки одной точки
    static void drawPoint(QPainter &painter, QPointF point, bool selected) {
        short int pointRadius = selected ? 2 : 1;  // Устанавливаем радиус точки в зависимости от того, выбрана ли она

        // Если точка выбрана, рисуем айди и подсветку
        QPointF logicPoint(Scaling::scaleCoordinate(point.x()), Scaling::scaleCoordinate(-point.y()));

        if (selected) {
            DrawAdditionalInf::drawPointID(painter, logicPoint);
            DrawAdditionalInf::drawPointGlow(painter, logicPoint);
        }

        // Устанавливаем кисть для рисования точки
        QBrush pointBrush = (MyColor.color() != Qt::black) ? MyColor.brush() : Qt::black;
        painter.setBrush(pointBrush);
        painter.setPen(Qt::NoPen);

        if (!selected && pointBrush.color() == QColor(Qt::red)) {
            pointRadius = 2;
        }

        painter.drawEllipse(logicPoint, pointRadius, pointRadius);
        MyColor = QPen(Qt::black);  // Сброс цвета
    }


    // Функция для отрисовки круга
    static void drawCircle(QPainter &painter, QPointF center, double radius, bool selected) {
        double scaledRadius = radius*Scaling::getZoom();

        QPointF logicCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));

        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);  // Устанавливаем толщину линии в зависимости от выбранности
        currentPen.setJoinStyle(Qt::RoundJoin);
        currentPen.setCapStyle(Qt::RoundCap);

        if (selected) {
            DrawAdditionalInf::drawCircleID(painter, logicCenter, scaledRadius);
            DrawAdditionalInf::drawCircleGlow(painter, logicCenter, scaledRadius);
        } else if (currentPen.color() == QColor(Qt::red)) {
            currentPen.setWidth(2);
            DrawAdditionalInf::drawCircleLeftID(painter, logicCenter, scaledRadius);
        }

        painter.setPen(currentPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для рисования линии
    static void drawSection(QPainter &painter, QPointF beg, QPointF end, bool selected) {
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);  // Устанавливаем толщину линии в зависимости от выбранности
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        QPointF start(Scaling::scaleCoordinate(beg.x()), Scaling::scaleCoordinate(-beg.y()));
        QPointF endLogic(Scaling::scaleCoordinate(end.x()), Scaling::scaleCoordinate(-end.y()));

        painter.drawLine(start, endLogic);

        currentPen = QColor(Qt::black);
        painter.setPen(currentPen);

        // Длина
        DrawAdditionalInf::drawCoordinateLine(painter, start, endLogic);

        // Айди, подсветка
        if (selected) {
            DrawAdditionalInf::drawSectionID(painter, start, endLogic);
            DrawAdditionalInf::drawSectionGlow(painter, start, endLogic);
        } else if (MyColor.color() == QColor(Qt::red)) {
            DrawAdditionalInf::drawSectionLeftID(painter, start, endLogic);
        }

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для рисования сектора
    static void drawSector(QPainter &painter, QPointF center, double radius, double startAngleDeg, double spanAngleDeg, bool selected) {
        // Настройка пера
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        // Масштабирование центра
        QPointF scaledCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));
        double scaledRadius = Scaling::scaleCoordinate(radius);

        // Прямоугольник, в который вписана дуга
        QRectF arcRect(scaledCenter.x() - scaledRadius, scaledCenter.y() - scaledRadius,
                       2 * scaledRadius, 2 * scaledRadius);


        double startAngle16 =Scaling::scaleCoordinate(-startAngleDeg ); // Минус, т.к. Y идёт вниз
        double spanAngle16 =Scaling::scaleCoordinate(-spanAngleDeg);

        painter.drawPie(arcRect, startAngle16, spanAngle16);

        // Сброс цвета
        MyColor = QPen(Qt::black);
    }

    // Функция для рисования дуги
    static void drawArc(QPainter &painter, QPointF center, double radius, double startAngleDeg, double spanAngleDeg, bool selected) {
        // Настройка пера
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);
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



};

#endif // OURPAINT_DRAWFIGURES_H
