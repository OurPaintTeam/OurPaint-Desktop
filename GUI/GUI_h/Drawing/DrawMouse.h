#ifndef OURPAINT_DRAWMOUSE_H
#define OURPAINT_DRAWMOUSE_H

#include <QObject>
#include <QPointF>
#include <QPainter>
#include <cmath>

#include "Modes.h"
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
    qint16 tabPressCount; // For hints

    static QColor hintColor(); // Grey line
    static void releaseTabIfPressed(); // Pressing tab

    void resetCoordinates(); // Reset everything

    // For rendering with a shift
    static QPointF getSnappedPoint(const QPointF& start, const QPointF& current);

    // Preliminary drawing of the gray lines of the suggestions
    static void drawPreviewSection(QPainter& painter, const QPointF& start, const QPointF& end);

public:
    [[maybe_unused]] explicit DrawMouse(QObject* parent = nullptr);

    // Calculating the angle
    static qreal snapAngle(qreal angle);

    // Drawing shapes with the mouse
    void DrawFiguresMouse(QPainter& painter,const QPointF& cursor);
    static void drawCircles(QPainter& painter, const QPointF& startCoordinates,const QPointF& cursor);
    static void drawSections(QPainter& painter, const QPointF& startCoordinates,const QPointF& cursor);

    // Drawing suggestions
    void drawHints(QPainter& painter, const QPointF& closesPoint,const QPointF& cursor);

    void clear();

signals:

    void SigPoint(const QPointF& point);
    void SigCircle(const QPointF& center,const qreal radius);
    void SigSection(const QPointF& stratPoint,const QPointF& endPoint);
    void SigArc(const QPointF& stratPoint,const QPointF& endPoint,const QPointF& centerPoint);
};

#endif // OURPAINT_DRAWMOUSE_H