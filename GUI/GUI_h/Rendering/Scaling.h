#ifndef SCALING_H
#define SCALING_H
#include <QVector>
#include <QPointF>
#include <QSize>
#include <QRectF>
#include <QLineF>

struct ScalingState {
    qreal scale;           // Scale factor for window resizing
    qreal zoom;            // Zoom level
    bool usersResize;      // Flag for user resizing

    QPoint Delta;          // Movement along the X,Y axis
    QPoint LastMousePos;   // Last mouse position
    QPoint Cursor;         // Current cursor position
    QSize StartMonitorSize; // Initial monitor size
    QSize ActualMonitorSize; // Current monitor size
    QSizeF CenteredCoordinates; // Centered coordinates

    bool operator==(const ScalingState& other) const;
    bool operator!=(const ScalingState& other) const;

    static ScalingState fromCurrent();
    void apply() const;
};

class Scaling {
private:
    static qreal scale;      // Scale factor for window resizing
    static qreal zoom;       // Zoom level
    static bool usersResize;  // Flag for user resizing
    const static qint16 userUnitSize; // Custom size of 20 pixels = 1 unit

    static QPoint Delta; // Movement along the X,Y axis
    static QPoint LastMousePos;
    static QPoint Cursor;
    static QSize StartMonitorSize;
    static QSize ActualMonitorSize;
    static QSizeF CenteredCoordinates;

    friend struct ScalingState;

public:
    static ScalingState copyState();
    static void restoreState(const ScalingState& state);

    static void updateScaling();

    static QSize getActualMonitorSize();
    static QSizeF getCenteredCoordinates();

    static void setStartMonitorSize(const QSize &size);
    static void setActualMonitorSize(const QSize &size);
    static QSize getStartMonitorSize();
    static bool getUsersResize();

    static QRectF scaleCoordinate(const QRectF &X);
    static QPointF scaleCoordinate(QPointF X);
    static qreal scaleCoordinate(qreal X);
    static qreal scaleCoordinateX(qreal X);
    static qreal scaleCoordinateY(qreal Y);
    static void setScale(qreal x);
    static qreal getScale();

    static void setZoomPlus();
    static void setZoomMinus();
    static void setZoomZero();
    static void setZoom(qreal z);
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

    static QPoint getLastMousePos();
    static void setCursor(const QPoint& cursor);
    static QPoint getCursor();
    static qint32 getCursorX();
    static qint32 getCursorY();

    static qreal logic(qreal X);
    static QRectF logic(const QRectF &X);
    static QLineF logic(const QPointF& p1, const QPointF& p2);
    static QLineF logic(const QLineF& p);
    static QPointF logic(QPointF X);
    static QPointF logic(QPoint X);
    static qreal logicCursorX();
    static qreal logicCursorY();
    static QPointF logicCursor();
    static QPointF scaleCursor();
};

#endif // SCALING_H
