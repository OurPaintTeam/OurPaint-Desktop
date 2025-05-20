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
    qint16 tabPressCount; // For hints
    bool drawingInProgress; // To track the status

    static inline QColor hintColor(); // Grey line
    static void releaseTabIfPressed(); // Pressing tab

    // For rendering with a shift
    static QPointF getSnappedPoint(const QPointF& start, const QPointF& current);

    // Preliminary drawing of the gray lines of the suggestions
    static void drawRunningSection(QPainter& painter, const QPointF& startCoordinates,const QPointF& cursor);
    static void drawPreviewArcs(QPainter& painter, const QPointF& startCoordinates,const QPointF& cursor);
    static void drawPreviewCircles(QPainter& painter, const QPointF& startCoordinates,const QPointF& cursor);
    static void drawPreviewSection(QPainter& painter, const QPointF& start, const QPointF& end);
    static void drawPreviewPoint(QPainter& painter, const QPointF& point);
public:
   explicit DrawMouse(QObject* parent = nullptr);

    // Calculating the angle
    static inline qreal snapAngle(qreal angle);
    static inline QPointF roundCursor(const QPointF& point);

    // Drawing shapes with the mouse
    void DrawSection(QPainter& painter, const QPointF& cursor);
    void DrawPoint(QPainter& painter, const QPointF& cursor);
    void DrawCircle(QPainter& painter, const QPointF& cursor);
    void DrawArc(QPainter& painter, const QPointF& cursor);

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