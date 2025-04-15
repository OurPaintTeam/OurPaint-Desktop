#ifndef SCALING_H
#define SCALING_H


#include <algorithm>
#include <cmath>
#include <vector>
#include <QVector>
#include <immintrin.h>
#include "GUI/control/Modes.h"
#include "shell/objects.h"
#include "List.h"

class Scaling {
private:
    static double scale;      // Scale factor for window resizing
    static double zoom;       // Zoom level
    static bool usersResize;  // Flag for user resizing

    struct Delta {
        static double X;        // Movement along the X axis
        static double Y;        // Movement along the Y axis
    };
    static Delta delta;

    struct LastMousePos {
        static int x;
        static int y;
    };
    static LastMousePos lastMouse;

    struct StartMonitorSize {
        static short int width_;     // Initial width of the window
        static short int height_;    // Initial height of the window
    };
    static StartMonitorSize startDisplaySize;

    struct CenteredCoordinates {
        static double CenteredCoordinatesX;
        static double CenteredCoordinatesY;
    };
    static CenteredCoordinates centeredDisplay;

    struct Display {
        static short int width;
        static int short height;
    };
    static Display monitor;

    struct Cursor {
        static int x;
        static int y;
    };
    static Cursor cursor;


public:
    struct ScaledCircle {
        int x, y, radius;
    };

    // Methods to get various properties of the scaling and window size
    static short int getStartWidth();
    static short int getStartHeight();

    static short int getDisplayCoordinateX();
    static short int getDisplayCoordinateY();
    static double getCenteredCoordinatesX();
    static double getCenteredCoordinatesY();
    static void setDisplayCoordinateX(int x);
    static void  setDisplayCoordinateY(int y);

    static void updateScaling();
    static void setStartSize(short int x, short int y);
    static void setActualSize(int x, int y);

    static double scaleCoordinate(double X);
    static double scaleCoordinateX(double X);
    static double scaleCoordinateY(double Y);
    static double logicInt(int X);
    static double logicXInt(int X);
    static double logicYInt(int Y);
    static double logicDouble(double X);
    static double logicXDouble(double X);
    static double logicYDouble(double Y);

    static void scaling(int widgetWidth, int widgetHeight, const std::vector<double>& size);
    static void setZoomPlus();
    static void setZoomMinus();
    static void setZoomZero();
    static void setDelta(double dx, double dy);
    static void setDeltaX(double X);
    static void setDeltaY(double Y);
    static void setCursor(int x, int y);
    static void setLastMouse(int x,int y);

    static double getScale();
    static double getZoom();
    static double getDeltaX();
    static double getDeltaY();
    static int getCursorX();
    static int getCursorY();

    static void startMousePress(int x, int y);
    static void mouseMove();
    static void resetUsersResize();
    static int getCursorDeltaX();
    static int getCursorDeltaY();
    static int centedX(double X);
    static int centedY(double Y);
    static double logicCursorX();
    static double logicCursorY();

    // Метод для масштабирования круга
    static QVector<ScaledCircle> scaleVectorCircle(const List<circle>& c);
    static QVector<ScaledCircle> scaleVectorCircleSIMD(const List<circle>& c);
};

#endif // SCALING_H
