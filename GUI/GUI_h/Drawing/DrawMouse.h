#ifndef OURPAINT_DRAWMOUSE_H
#define OURPAINT_DRAWMOUSE_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <cmath>


#include "Modes.h"
#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "DrawFigures.h"

class DrawMouse : public QObject {
Q_OBJECT

//  Класс для отрисовки мышью
//  Добавлены подсказки по ближайшим точкам

private:
    QPointF closestStartPoint; // Ближайшая точка
    QPointF closestPointNext;  // Следующая ближайшая точка
    QPointF startCoordinates; // Точка нажатия курсора
    bool drawingInProgress; // Для отслеживания состояния
    short int tabPressCount; // Для подсказок

    static QColor hintColor(); // Серая линия

    void resetCoordinates(); // Обнуление всего

    static void releaseTabIfPressed(); // Нажатие таба

    // Для отрисовки с шифтом
    static QPointF getSnappedPoint(const QPointF &start, const QPointF &current) ;

    // Предварительная отрисовка серых линий подсказок
    static void drawPreviewSection(QPainter &painter, const QPointF &start, const QPointF &end) ;

public:
    explicit DrawMouse(QObject *parent = nullptr);

    // Вычисление угла
    static double snapAngle(double angle) ;

    // Отрисовка фигур мышью
    void DrawFiguresMouse(QPainter &painter);
    static void drawSections(QPainter &painter, const QPointF &startCoordinates);

    // Отрисовка подсказок
    void drawHints(QPainter &painter, const QPointF &closesPoint) ;

signals:
    void SigPoint(double x, double y);
    void SigCircle(double x, double y, double r);
    void SigSection(double x, double y, double x1, double y1);
    void SigArc(double x, double y, double x1, double y1, double xc, double yc);
};

#endif // OURPAINT_DRAWMOUSE_H