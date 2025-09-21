#ifndef SCALING_H
#define SCALING_H
#include <QVector>
#include <QPointF>
#include <QSize>
#include <QRectF>
#include <QLineF>

class Scaling {
private:
    static qreal scale;      // Scale factor for window resizing
    static qreal zoom;       // Zoom level
    [[maybe_unused]] static bool usersResize;  // Flag for user resizing
    const static qint16 userUnitSize; // Custom size of 20 pixels = 1 unit

    static QPoint Delta; // Movement along the X,Y axis
    static QPoint LastMousePos;
    static QPoint Cursor;
    [[maybe_unused]] static QSize StartMonitorSize;
    static QSize ActualMonitorSize;
    static QSizeF CenteredCoordinates;

public:
    static void updateScaling();

    static QSize getActualMonitorSize();
    static QSizeF getCenteredCoordinates();

    static void setStartMonitorSize(const QSize &size);
    static void setActualMonitorSize(const QSize &size);

    [[maybe_unused]] static QRectF scaleCoordinate(QRectF X);
    static QPointF scaleCoordinate(QPointF X);
    static qreal scaleCoordinate(qreal X);
    static qreal scaleCoordinateX(qreal X);
    static qreal scaleCoordinateY(qreal Y);

    static void setZoomPlus();
    static void setZoomMinus();
    static void setZoomZero();

    [[maybe_unused]] static void setZoom(qreal z);

    static qint16 getUserUnitSize();
    static qreal getZoom();

    static void setDelta(const QPoint &delta);
    static qint32 getDeltaX();
    static qint32 getDeltaY();
    static QPoint getDelta();

    static QPoint getCursorDelta();
    static QPointF getCursorLogicDelta();

    static void startMousePress(const QPoint& pos);
    static void mouseMove();

    static void setCursor(const QPoint& cursor);

    [[maybe_unused]] static QPoint getCursor();
    static qint32 getCursorX();
    static qint32 getCursorY();

    static qreal logic(qreal X);
    static QRectF logic(QRectF X);
    static QLineF logic(QPointF& p1,QPointF& p2);
    static QLineF logic(QLineF& p);
    static QPointF logic(QPointF X);
    static QPointF logic(QPoint X);
    static qreal logicCursorX();
    static qreal logicCursorY();
    static QPointF logicCursor();
    static QPointF scaleCursor();
};

#endif // SCALING_H
