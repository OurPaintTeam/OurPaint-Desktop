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

// Class for drawing with the mouse
// Added hints for the nearest points

private:
    QPointF closestStartPoint; // Nearest point
    QPointF closestPointNext;  // Next nearest point
    QPointF startCoordinates; // Cursor click point
    bool drawingInProgress; // To track the status
    short int tabPressCount; // For hints

    static QColor hintColor(); // Grey line
    static void releaseTabIfPressed(); // Pressing tab

    void resetCoordinates(); // Reset everything

    // For rendering with a shift
    static QPointF getSnappedPoint(const QPointF &start, const QPointF &current) ;

    // Preliminary drawing of the gray lines of the suggestions
    static void drawPreviewSection(QPainter &painter, const QPointF &start, const QPointF &end) ;

public:
    explicit DrawMouse(QObject *parent = nullptr);

    // Calculating the angle
    static double snapAngle(double angle) ;

    // Drawing shapes with the mouse
    void DrawFiguresMouse(QPainter &painter);
    static void drawSections(QPainter &painter, const QPointF &startCoordinates);

    // Drawing suggestions
    void drawHints(QPainter &painter, const QPointF &closesPoint) ;

    void clear();

signals:
    void SigPoint(double x, double y);
    void SigCircle(double x, double y, double r);
    void SigSection(double x, double y, double x1, double y1);
    void SigArc(double x, double y, double x1, double y1, double xc, double yc);
};

#endif // OURPAINT_DRAWMOUSE_H