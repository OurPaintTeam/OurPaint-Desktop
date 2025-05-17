#ifndef SCALING_H
#define SCALING_H
#include <QVector>
#include <QPointF>

class Scaling {
private:
    static qreal scale;      // Scale factor for window resizing
    static qreal zoom;       // Zoom level
    [[maybe_unused]] static bool usersResize;  // Flag for user resizing
    const static qint16 userUnitSize; // Custom size of 20 pixels = 1 unit

    struct Delta {
        static qreal X;        // Movement along the X axis
        static qreal Y;        // Movement along the Y axis
    };

    struct LastMousePos {
        static qint16 x;
        static qint16 y;
    };

    struct Cursor {
        static qint16 x;
        static qint16 y;
    };

    struct StartMonitorSize {
        static qint16 StartMonitorWidth;     // Initial width of the window
        static qint16 StartMonitorHeight;    // Initial height of the window
    };

    struct CenteredCoordinates {
        static qreal CenteredCoordinatesX;
        static qreal CenteredCoordinatesY;
    };

    struct ActualMonitorSize {
        static qint16 ActualMonitorWidth;
        static int short ActualMonitorHeight;
    };
public:

    // Methods to get various properties of the scaling and window size
    [[maybe_unused]]
    static qint16 getStartWidth();
    [[maybe_unused]]
    static qint16 getStartHeight();

    static qint16 getActualMonitorWidth();
    static qint16 getActualMonitorHeight();
    static QPoint getActualMonitorSize();

    static qreal getCenteredCoordinatesX();
    static qreal getCenteredCoordinatesY();
    static QPointF getCenteredCoordinates();

    [[maybe_unused]]
    static void setCenteredCoordinatesX(qreal x);
    [[maybe_unused]]
    static void  setCenteredCoordinatesY(qreal y);

    static void updateScaling();
    static void setStartMonitorSize(qint16 x, qint16 y);
    static void setActualMonitorSize(qint16 x,qint16 y);

    static qreal scaleCoordinate(qreal X);
    static qreal scaleCoordinateX(qreal X);
    static qreal scaleCoordinateY(qreal Y);
    static qreal logic(qreal X);

    static void setZoomPlus();
    static void setZoomMinus();
    static void setZoomZero();

    [[maybe_unused]]
    static void setZoom(qreal z);

    static void setDelta(qreal dx, qreal dy);

    [[maybe_unused]]
    static void setDeltaX();

    [[maybe_unused]]
    static void setDeltaY();

    static void setCursor(qint16 x,qint16 y);

    [[maybe_unused]]
    static qreal getScale();
    static qint16 getUserUnitSize();
    static qreal getZoom();

    static qreal getDeltaX();
    static qreal getDeltaY();
    static QPointF getDelta();

    static int getCursorX();
    static int getCursorY();

    [[maybe_unused]]
    static QPoint getCursor();

    static void startMousePress(qint16 x, qint16 y);
    static void mouseMove();

    static int getCursorDeltaX();
    static int getCursorDeltaY();

    static qreal logicCursorX();
    static qreal logicCursorY();
    static QPointF logicCursor();

    [[maybe_unused]]
    static void resetUsersResize();

};

#endif // SCALING_H
