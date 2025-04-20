#ifndef SCALING_H
#define SCALING_H
#include <vector>

class Scaling {
private:
    static double scale;      // Scale factor for window resizing
    static double zoom;       // Zoom level
    static bool usersResize;  // Flag for user resizing
    const static short int userUnitSize; // Пользовательский размер 20пикселей=1единице

    struct Delta {
        static double X;        // Movement along the X axis
        static double Y;        // Movement along the Y axis
    };

    struct LastMousePos {
        static short int x;
        static short int y;
    };

    struct Cursor {
        static short int x;
        static short int y;
    };

    struct StartMonitorSize {
        static short int StartMonitorWidth;     // Initial width of the window
        static short int StartMonitorHeight;    // Initial height of the window
    };

    struct CenteredCoordinates {
        static double CenteredCoordinatesX;
        static double CenteredCoordinatesY;
    };

    struct ActualMonitorSize {
        static short int ActualMonitorWidth;
        static int short ActualMonitorHeight;
    };


public:

    // Methods to get various properties of the scaling and window size
    static short int getStartWidth();
    static short int getStartHeight();

    static short int getActualMonitorWidth();
    static short int getActualMonitorHeight();
    static double getCenteredCoordinatesX();
    static double getCenteredCoordinatesY();
    static void setCenteredCoordinatesX(double x);
    static void  setCenteredCoordinatesY(double y);

    static void updateScaling();
    static void setStartMonitorSize(short int x, short int y);
    static void setActualMonitorSize(short int x,short int y);

    static double scaleCoordinate(double X);
    static double scaleCoordinateX(double X);
    static double scaleCoordinateY(double Y);
    static double logic(double X);

    static void scaling(int widgetWidth, int widgetHeight, const std::vector<double>& size);
    static void setZoomPlus();
    static void setZoomMinus();
    static void setZoomZero();
    static void setZoom(double z);
    static void setDelta(double dx, double dy);
    static void setDeltaX();
    static void setDeltaY();
    static void setCursor(short int x,short int y);

    static double getScale();
    static short int getUserUnitSize();
    static double getZoom();
    static double getDeltaX();
    static double getDeltaY();
    static int getCursorX();
    static int getCursorY();

    static void startMousePress(int x, int y);
    static void mouseMove();
    static int getCursorDeltaX();
    static int getCursorDeltaY();
    static double logicCursorX();
    static double logicCursorY();

    static void resetUsersResize();

};

#endif // SCALING_H
