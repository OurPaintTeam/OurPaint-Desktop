#include "objects/GeometricObjects.h"

Point::Point() : x(), y() {}
Point::Point(double x, double y) : x(x), y(y) {}
BoundingBox Point::getBox() {
    return {x, x, y, y};
}
inline Element Point::getElementType() {
    return ET_POINT;
}
std::ifstream& operator>> (std::ifstream& in, Point& x) {
    in >> x.x;
    in >> x.y;
    return in;
}

Section::Section() : beg(nullptr), end(nullptr) {}
Section::Section(Point* p1, Point* p2) : beg(p1), end(p2) {}
BoundingBox Section::getBox() {
    return {beg->x,end->x, beg->y, end->y};
}
inline Element Section::getElementType() {
    return ET_SECTION;
}

Circle::Circle() : center(nullptr), r() {}
Circle::Circle(Point* p, double r) : center(p), r(r) {}
BoundingBox Circle::getBox() {
    return {center->x - r, center->x + r,center->y - r, center->y + r};
}
inline Element Circle::getElementType() {
    return ET_CIRCLE;
}
