#ifndef OURPAINT_DRAWMOUSE_H
#define OURPAINT_DRAWMOUSE_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <cmath>
#include <QDebug>

#include "Modes.h"
#include "Scaling.h"
#include "DrawAdditionalInf.h"
#include "DrawFigures.h"

class DrawMouse : public QObject {
Q_OBJECT
//  Класс для отрисовки мышью
// Добавлены подсказки по ближайшим точкам

private:

    QPointF closestStartPoint;
    QPointF closestPointNext;
    QPointF startCoordinates;
    bool drawingInProgress;
    int tabPressCount;

    static QColor hintColor() ;

    void resetCoordinates();

    static void releaseTabIfPressed() ;

    static QPointF getSnappedPoint(const QPointF &start, const QPointF &current) ;

    static void drawPreviewSection(QPainter &painter, const QPointF &start, const QPointF &end) ;

public:
    explicit DrawMouse(QObject *parent = nullptr);

    static double snapAngle(double angle) ;

    void DrawFiguresMouse(QPainter &painter);

    static void drawSections(QPainter &painter, const QPointF &startCoordinates);
    void drawHints(QPainter &painter, const QPointF &closesPoint) ;

signals:
    void SigPoint(double x, double y);
    void SigCircle(double x, double y, double r);
    void SigSection(double x, double y, double x1, double y1);
};

#endif // OURPAINT_DRAWMOUSE_H