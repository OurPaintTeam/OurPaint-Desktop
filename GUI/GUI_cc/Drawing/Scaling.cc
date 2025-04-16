#include <QPointF>
#include "Scaling.h"
#include "GeometricObjects.h"


// Static member initializations
double Scaling::scale = 1.0;
double Scaling::zoom = 20;
bool Scaling::usersResize = false;

double Scaling::Delta::X = 0;
double Scaling::Delta::Y = 0;

int Scaling::LastMousePos::x = 0;
int Scaling::LastMousePos::y = 0;

short int Scaling::StartMonitorSize::width_ = 1.0;
short int Scaling::StartMonitorSize::height_ = 1.0;

double Scaling::CenteredCoordinates::CenteredCoordinatesX = 1.0;
double Scaling::CenteredCoordinates::CenteredCoordinatesY = 1.0;

short int Scaling::Display::width = 1;
short int Scaling::Display::height = 1;

int Scaling::Cursor::x = 0;
int Scaling::Cursor::y = 0;

short int Scaling::getStartWidth() {
    return Scaling::StartMonitorSize::width_;
}

short int Scaling::getStartHeight() {
    return Scaling::StartMonitorSize::height_;
}

short int Scaling::getDisplayCoordinateX() {
    return Scaling::Display::width;
}

short int Scaling::getDisplayCoordinateY() {
    return Scaling::Display::height;
}


void Scaling::setDisplayCoordinateX(int x) {
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x;
}

void Scaling::setDisplayCoordinateY(int y) {
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y;
}

double Scaling::getCenteredCoordinatesX() {
    return Scaling::CenteredCoordinates::CenteredCoordinatesX;
}

double Scaling::getCenteredCoordinatesY() {
    return Scaling::CenteredCoordinates::CenteredCoordinatesY;
}

void Scaling::updateScaling() {
    scale = 1.0;
    zoom = 20;
    usersResize = false;
}

void Scaling::setStartSize(short int x, short int y) {
    Scaling::StartMonitorSize::width_ = x;
    Scaling::StartMonitorSize::height_ = y;
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x / 2.0;
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y / 2.0;
}

void Scaling::setActualSize(int x, int y) {
    Scaling::Display::width = (short) x;
    Scaling::Display::height = (short) y;
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x / 2.0;
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y / 2.0;
}

double Scaling::logicInt(int X) {
    return X / (scale * zoom);
}

double Scaling::logicXInt(int X) {
    return (X) / (scale * zoom);
    //+ Scaling::CenteredCoordinates::CenteredCoordinatesX + Scaling::Delta::X
}

double Scaling::logicYInt(int Y) {
    return (-Y) / (scale * zoom);
    //+ Scaling::CenteredCoordinates::CenteredCoordinatesY + Scaling::Delta::Y
}

double Scaling::logicCursorX() {
    return ((Scaling::getCursorX() - Scaling::CenteredCoordinates::CenteredCoordinatesX - Scaling::Delta::X) /
            (scale * zoom));
    //
}

double Scaling::logicCursorY() {
    // У оси у ось инвертирована
    return ((-Scaling::getCursorY() + Scaling::CenteredCoordinates::CenteredCoordinatesY + Scaling::Delta::Y) /
            (scale * zoom));
    //
}

double Scaling::logicDouble(double X) {
    return X / (scale * zoom);
}

double Scaling::logicXDouble(double X) {
    return (X - Scaling::CenteredCoordinates::CenteredCoordinatesX - Scaling::Delta::X) / (scale * zoom);
}

double Scaling::logicYDouble(double Y) {
    return (-Y + Scaling::CenteredCoordinates::CenteredCoordinatesY + Scaling::Delta::Y) / (scale * zoom);
}

double Scaling::scaleCoordinate(double X) {
    return X * scale * zoom;
}

double Scaling::scaleCoordinateX(double X) {
    return (X - Scaling::Delta::X - Scaling::CenteredCoordinates::CenteredCoordinatesX);//();
}

double Scaling::scaleCoordinateY(double Y) {
    return (Y - Scaling::Delta::Y - Scaling::CenteredCoordinates::CenteredCoordinatesY);//+ ();
}

int Scaling::centedX(double X) {
    return (X + Scaling::Delta::X + Scaling::CenteredCoordinates::CenteredCoordinatesX);
}

int Scaling::centedY(double Y) {
    return (-Y + Scaling::Delta::Y + Scaling::CenteredCoordinates::CenteredCoordinatesY);
}

void Scaling::scaling(int widgetWidth, int widgetHeight, const std::vector<double> &size) {
    double scaleX = static_cast<double>(widgetWidth) / Scaling::StartMonitorSize::width_;
    double scaleY = static_cast<double>(widgetHeight) / Scaling::StartMonitorSize::height_;
    scale = std::min(scaleX, scaleY);

    if (!usersResize && !size.empty()) {
        int widthX = static_cast<int>(widgetWidth / (2 * scale * zoom));
        int heightY = static_cast<int>(widgetHeight / (2 * scale * zoom));

        double maxX = size[0];
        double minX = size[1];
        double maxY = size[2];
        double minY = size[3];

        double scaleMaxX = (maxX / (scale * zoom));
        double scaleMaxY = (maxY / (scale * zoom));
        double scaleMinX = (minX / (scale * zoom));
        double scaleMinY = (minY / (scale * zoom));

        double ZoomMaxX = std::abs(widthX / scaleMaxX);
        double ZoomMinX = std::abs(widthX / scaleMinX);
        double ZoomMaxY = std::abs(heightY / scaleMaxY);
        double ZoomMinY = std::abs(heightY / scaleMinY);

        double newZoomX = std::min(ZoomMaxX, ZoomMinX);
        double newZoomY = std::min(ZoomMaxY, ZoomMinY);
        double newZoom = std::min(newZoomX, newZoomY);

        if (newZoom < zoom) {
            const double margin = 1.1;
            // zoom = newZoom / margin;
        }
    }
}

void Scaling::setZoomPlus() {
    usersResize = true;
    if (zoom < 125) {
        zoom *= 1.1;
    }
    else {
        zoom = 125;
    }
}

void Scaling::setZoomMinus() {
    usersResize = true;
    if (zoom > 9.80891e-08) {
        zoom /= 1.1;
    }
    else {
        zoom = 9.80891e-08;
    }
}

void Scaling::setZoomZero() {
    usersResize = true;
    zoom = 50;
    Scaling::Delta::X = 0;
    Scaling::Delta::Y = 0;
}

double Scaling::getScale() {
    return scale;
}

qreal Scaling::getZoom() {
    return zoom;
}

double Scaling::getDeltaX() {
    return Scaling::Delta::X;
}

double Scaling::getDeltaY() {
    return Scaling::Delta::Y;
}

int Scaling::getCursorX() {
    return Scaling::Cursor::x;
}

int Scaling::getCursorY() {
    return Scaling::Cursor::y;
}

void Scaling::setCursor(int x, int y) {
    Scaling::Cursor::x = x;
    Scaling::Cursor::y = y;
}

void Scaling::setDelta(double dx, double dy) {
    Scaling::Delta::X += dx;
    Scaling::Delta::Y += dy;
}

void Scaling::setDeltaX(double X) {
    Scaling::Delta::X += 10;
}

void Scaling::setDeltaY(double Y) {
    Scaling::Delta::Y += 10;
}

void Scaling::setLastMouse(int x, int y) {
    Scaling::LastMousePos::x = x;
    Scaling::LastMousePos::y = y;
}

void Scaling::startMousePress(int x, int y) {
    Scaling::LastMousePos::x = x;
    Scaling::LastMousePos::y = y;
}

void Scaling::mouseMove() {
    usersResize = true;
    int deltaX_ = Scaling::Cursor::x - Scaling::LastMousePos::x;
    int deltaY_ = Scaling::Cursor::y - Scaling::LastMousePos::y;

    setDelta(deltaX_, deltaY_);

    Scaling::LastMousePos::x = Scaling::Cursor::x;
    Scaling::LastMousePos::y = Scaling::Cursor::y;

}

int Scaling::getCursorDeltaX() {
    int temp = Scaling::Cursor::x - Scaling::LastMousePos::x;
    Scaling::LastMousePos::x = Scaling::Cursor::x;
    return temp;
}

int Scaling::getCursorDeltaY() {
    int temp = -Scaling::Cursor::y + Scaling::LastMousePos::y;
    Scaling::LastMousePos::y = Scaling::Cursor::y;
    return temp;
}

void Scaling::resetUsersResize() {
    usersResize = false;
}
