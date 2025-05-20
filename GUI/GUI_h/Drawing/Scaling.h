#ifndef SCALING_H
#define SCALING_H
#include <QVector>
#include <QPointF>
#include <QSize>

class Scaling {
private:
    static qreal scale;      // Scale factor for window resizing
    static qreal zoom;       // Zoom level
    [[maybe_unused]] static bool usersResize;  // Flag for user resizing
    const static qint16 userUnitSize; // Custom size of 20 pixels = 1 unit

    static QPoint Delta; // Movement along the X,Y axis
    static QPoint LastMousePos;
    static QPoint Cursor;
    static QSize StartMonitorSize;
    static QSizeF CenteredCoordinates;


    struct ActualMonitorSize {
        static quint16 ActualMonitorWidth;
        static quint16 ActualMonitorHeight;
    };
public:

    static QSize getActualMonitorSize();
    static QSizeF getCenteredCoordinates();


    static void updateScaling();

    static void setStartMonitorSize(const QSize &size);
    static void setActualMonitorSize(const QSize &size);

    static qreal scaleCoordinate(qreal X);
    static qreal scaleCoordinateX(qreal X);
    static qreal scaleCoordinateY(qreal Y);
    static qreal logic(qreal X);

    static void setZoomPlus();
    static void setZoomMinus();
    static void setZoomZero();

    [[maybe_unused]]
    static void setZoom(qreal z);

    static void setDelta(const QPoint &delta);

    [[maybe_unused]]
    static void setDeltaX();

    [[maybe_unused]]
    static void setDeltaY();

    static void setCursor(const QPoint& cursor);

    [[maybe_unused]]
    static qreal getScale();
    static qint16 getUserUnitSize();
    static qreal getZoom();

    static qreal getDeltaX();
    static qreal getDeltaY();
    static QPointF getDelta();

    static qint32 getCursorX();
    static qint32 getCursorY();

    [[maybe_unused]]
    static QPoint getCursor();

    static void startMousePress(const QPoint& pos);
    static void mouseMove();

    static qint32 getCursorDeltaX();
    static qint32 getCursorDeltaY();

    static qreal logicCursorX();
    static qreal logicCursorY();
    static QPointF logicCursor();

    [[maybe_unused]]
    static void resetUsersResize();

};

#endif // SCALING_H
