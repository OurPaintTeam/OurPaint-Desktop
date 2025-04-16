#ifndef OURPAINT_DRAWFIGURES_H
#define OURPAINT_DRAWFIGURES_H

#include <QPainter>
#include <QDebug>
#include <QElapsedTimer>
#include <QPainterPath>
#include <QGraphicsEllipseItem>
#include <list>

#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "GeometricObjects.h"

// Класс для отрисовки фигур
// Преобразует координаты с учётом масштабирования и инвертированной оси Y,
// а также использует дополнительные функции для выделения объектов.
class DrawFigures {
    static QPen MyColor;  // Статическая переменная для хранения текущего цвета кисти

public:
    // Установить цвет кисти для отрисовки
    static void setPen(const QPen &color) {
        MyColor = color;
    }

    // Функция для отрисовки точек
    static void drawPoint(QPainter &painter, const std::list<Point> &points) {
        if (points.size() == 0) return;

        // Настройка кисти и отключение обводки
        QBrush pointBrush = (QColor(Qt::black));
        painter.setRenderHint(QPainter::Antialiasing);  // Включаем сглаживание
        painter.setBrush(pointBrush);
        painter.setPen(Qt::NoPen);

        double pointRadius = 1;

        // Отрисовываем каждую точку
        for (const auto &pt: points) {

            painter.drawEllipse(QPointF(Scaling::scaleCoordinate(pt.x),
                                        Scaling::scaleCoordinate(-pt.y)),
                                pointRadius, pointRadius);
        }
    }

    // Функция для отрисовки кругов
    static void drawCircle(QPainter &painter, const std::list<Circle> &circles) {
        if (circles.size() == 0) return;

        // Настройка пера для рисования кругов
        QPen currentPen = QColor(Qt::black);
        currentPen.setWidth(1 );
        currentPen.setJoinStyle(Qt::RoundJoin);
        currentPen.setCapStyle(Qt::RoundCap);

        painter.setPen(currentPen);
        painter.setBrush(Qt::NoBrush);
        painter.setRenderHint(QPainter::Antialiasing);

        // Отрисовываем каждый круг
        for (const auto &cir: circles) {
            painter.drawEllipse(QPointF(Scaling::scaleCoordinate(cir.center->x), Scaling::scaleCoordinate(-cir.center->y)),
                                        Scaling::scaleCoordinate(cir.R), Scaling::scaleCoordinate(cir.R));
        }

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для отрисовки сечений (линий)
    static void drawSection(QPainter &painter, const std::list<Section> &sections) {
        if (sections.size() == 0) return;

        painter.setRenderHint(QPainter::Antialiasing);  // Включаем сглаживание
        QPen currentPen = QPen(Qt::black);
        currentPen.setWidth(1);
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        // Отрисовываем каждое сечение
        for (const auto &sec: sections) {
            QPointF start(Scaling::scaleCoordinate(sec.beg->x), Scaling::scaleCoordinate(-sec.beg->y));
            QPointF end(Scaling::scaleCoordinate(sec.end->x), Scaling::scaleCoordinate(-sec.end->y));
            painter.drawLine(start, end);
            // Если нужно, можно включить дополнительные элементы для рисования
            DrawAdditionalInf::drawCoordinateLine(painter, start, end);
        }

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для отрисовки одной точки с учётом её состояния
    static void drawPoint(QPainter &painter, QPointF point, bool selected) {

        short int pointRadius = selected ? 2 : 1;  // Устанавливаем радиус точки в зависимости от того, выбрана ли она

        // Если точка выбрана, рисуем дополнительные элементы (например, подсветку)
        QPointF logicPoint(Scaling::scaleCoordinate(point.x()), Scaling::scaleCoordinate(-point.y()));
        if (selected) {
            DrawAdditionalInf::drawPointID(painter, logicPoint);
            DrawAdditionalInf::drawPointGlow(painter, logicPoint);
        }

        // Устанавливаем кисть для рисования точки
        QBrush pointBrush = (MyColor.color() != Qt::black) ? MyColor.brush() : Qt::black;
        painter.setBrush(pointBrush);
        painter.setPen(Qt::NoPen);

        // Если точка не выбрана и её цвет - красный, увеличиваем радиус
        if (!selected && pointBrush.color() == QColor(Qt::red)) {
            pointRadius = 2;
        }

        // Отрисовываем точку
        painter.drawEllipse(logicPoint, pointRadius, pointRadius);
        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Функция для отрисовки круга с учётом его состояния (выбран или нет)
    static void drawCircle(QPainter &painter, QPointF center, double radius, bool selected) {
        double scaledRadius = radius*Scaling::getZoom();
        QPointF logicCenter(Scaling::scaleCoordinate(center.x()), Scaling::scaleCoordinate(-center.y()));

        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);  // Устанавливаем толщину линии в зависимости от выбранности

        currentPen.setJoinStyle(Qt::RoundJoin);
        currentPen.setCapStyle(Qt::RoundCap);

        // Если круг выбран, рисуем дополнительные элементы
        if (selected) {
            DrawAdditionalInf::drawCircleID(painter, logicCenter, scaledRadius);
            DrawAdditionalInf::drawCircleGlow(painter, logicCenter, scaledRadius);
        } else if (currentPen.color() == QColor(Qt::red)) {
            currentPen.setWidth(2);
            DrawAdditionalInf::drawCircleLeftID(painter, logicCenter, scaledRadius);
        }

        // Отрисовываем круг
        painter.setPen(currentPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(logicCenter, scaledRadius, scaledRadius);

        MyColor = QPen(Qt::black);  // Сброс цвета
    }

    // Универсальная функция для рисования линии с учётом её состояния (выбрана или нет)
    static void drawSection(QPainter &painter, QPointF beg, QPointF end, bool selected) {
        QPen currentPen = (MyColor.color() == Qt::black) ? QPen(Qt::black) : MyColor;
        currentPen.setWidth(selected ? 2 : 1);  // Устанавливаем толщину линии в зависимости от выбранности
        currentPen.setCapStyle(Qt::RoundCap);
        painter.setPen(currentPen);

        QPointF start(Scaling::scaleCoordinate(beg.x()), Scaling::scaleCoordinate(-beg.y()));
        QPointF endLogic(Scaling::scaleCoordinate(end.x()), Scaling::scaleCoordinate(-end.y()));

        // Отрисовываем линию
        painter.drawLine(start, endLogic);

        currentPen = QColor(Qt::black);  // Сброс цвета
        painter.setPen(currentPen);

        // Рисуем дополнительные элементы для линии (например, координатные линии)
        DrawAdditionalInf::drawCoordinateLine(painter, start, endLogic);

        // Если линия выбрана, рисуем дополнительные элементы
        if (selected) {
            DrawAdditionalInf::drawSectionID(painter, start, endLogic);
            DrawAdditionalInf::drawSectionGlow(painter, start, endLogic);
        } else if (MyColor.color() == QColor(Qt::red)) {
            DrawAdditionalInf::drawSectionLeftID(painter, start, endLogic);
        }

        MyColor = QPen(Qt::black);  // Сброс цвета
    }
};

#endif // OURPAINT_DRAWFIGURES_H
