#include "Scaling.h"


// Static member initializations
const short int Scaling::userUnitSize = 20;
double Scaling::zoom = userUnitSize;
double Scaling::scale =1.0;
bool Scaling::usersResize = false;


double Scaling::Delta::X = 0;
double Scaling::Delta::Y = 0;

short int Scaling::LastMousePos::x = 0;
short int Scaling::LastMousePos::y = 0;

short int Scaling::StartMonitorSize::StartMonitorWidth = 1;
short int Scaling::StartMonitorSize::StartMonitorHeight = 1;

double Scaling::CenteredCoordinates::CenteredCoordinatesX = 1.0;
double Scaling::CenteredCoordinates::CenteredCoordinatesY = 1.0;

short int Scaling::ActualMonitorSize::ActualMonitorWidth = 1;
short int Scaling::ActualMonitorSize::ActualMonitorHeight = 1;

short int Scaling::Cursor::x = 0;
short int Scaling::Cursor::y = 0;

short int Scaling::getStartWidth() {
    return Scaling::StartMonitorSize::StartMonitorWidth;
}

short int Scaling::getStartHeight() {
    return Scaling::StartMonitorSize::StartMonitorHeight;
}

short int Scaling::getActualMonitorWidth() {
    return Scaling::ActualMonitorSize::ActualMonitorWidth;
}

short int Scaling::getActualMonitorHeight() {
    return Scaling::ActualMonitorSize::ActualMonitorHeight;
}

double Scaling::getCenteredCoordinatesX() {
    return Scaling::CenteredCoordinates::CenteredCoordinatesX;
}

double Scaling::getCenteredCoordinatesY() {
    return Scaling::CenteredCoordinates::CenteredCoordinatesY;
}

void Scaling::setCenteredCoordinatesX(double x) {
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x;
}

void Scaling::setCenteredCoordinatesY(double y) {
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y;
}



void Scaling::updateScaling() {
    scale = 1.0;
    zoom = userUnitSize;
    usersResize = false;
}

void Scaling::setStartMonitorSize(short int x, short int y) {
    Scaling::StartMonitorSize::StartMonitorWidth = x;
    Scaling::StartMonitorSize::StartMonitorHeight = y;
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x / 2.0;
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y / 2.0;
}

void Scaling::setActualMonitorSize(short int x,short int y) {
    Scaling::ActualMonitorSize::ActualMonitorWidth =  x;
    Scaling::ActualMonitorSize::ActualMonitorHeight =  y;
    Scaling::CenteredCoordinates::CenteredCoordinatesX = x / 2.0;
    Scaling::CenteredCoordinates::CenteredCoordinatesY = y / 2.0;
}






double Scaling::logic(double X) {
    return X / (scale * zoom);
}

double Scaling::logicCursorX() {
    return ((Scaling::getCursorX() - Scaling::CenteredCoordinates::CenteredCoordinatesX - Scaling::Delta::X) /
            (scale * zoom));
}

double Scaling::logicCursorY() {
    // У оси у ось инвертирована
    return ((-Scaling::getCursorY() + Scaling::CenteredCoordinates::CenteredCoordinatesY + Scaling::Delta::Y) /
            (scale * zoom));
}






double Scaling::scaleCoordinate(double X) {
    return (X * scale * zoom);
}

double Scaling::scaleCoordinateX(double X) {
    return (X - Scaling::Delta::X - Scaling::CenteredCoordinates::CenteredCoordinatesX);//();
}

double Scaling::scaleCoordinateY(double Y) {
    return (Y - Scaling::Delta::Y - Scaling::CenteredCoordinates::CenteredCoordinatesY);//+ ();
}



void Scaling::scaling(/* int widgetWidth, int widgetHeight, const std::vector<double> &size */) {
}

void Scaling::setZoom(double z){
    zoom=z;
}

void Scaling::setZoomPlus() {
    usersResize = true;

    const short int MAXSIZE =50;
    if (zoom < MAXSIZE) {
        zoom *= 1.1;
    }
    else {
        zoom = MAXSIZE;
    }
    scale = 1.0;
}

void Scaling::setZoomMinus() {
    usersResize = true;
    const double MINSIZE=9e-07;
    if (zoom > MINSIZE) {
        zoom /= 1.1;
    }
    else {
        zoom = MINSIZE;
    }
    scale = 1.0;
}

void Scaling::setZoomZero() {
    usersResize = true;
    zoom = userUnitSize;
    scale=1.0;
    Scaling::Delta::X = 0;
    Scaling::Delta::Y = 0;
}

short int Scaling::getUserUnitSize() {
    return userUnitSize;
}

double Scaling::getScale() {
    return scale;
}

double Scaling::getZoom() {
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

void Scaling::setCursor(short int x,short int y) {
    Scaling::Cursor::x = x;
    Scaling::Cursor::y = y;
}

void Scaling::setDelta(double dx, double dy) {
    Scaling::Delta::X += dx;
    Scaling::Delta::Y += dy;
}

void Scaling::setDeltaX() {
    const short int step=10;
    Scaling::Delta::X += step;
}

void Scaling::setDeltaY() {
    const short int step=10;
    Scaling::Delta::Y += step;
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
