#include "GeometricObjects.h"

Point::Point() : x(), y() {}
Point::Point(double x, double y) : x(x), y(y) {}
BoundBox2D Point::getBox() const {
    return {x, x, y, y};
}
inline Element Point::getType() const {
    return Element::ET_POINT;
}
std::ifstream& operator>> (std::ifstream& in, Point& x) {
    in >> x.x;
    in >> x.y;
    return in;
}

Section::Section() : beg(nullptr), end(nullptr) {}
Section::Section(Point* p1, Point* p2) : beg(p1), end(p2) {}
BoundBox2D Section::getBox() const {
    return {beg->x,end->x, beg->y, end->y};
}
inline Element Section::getType() const {
    return Element::ET_SECTION;
}

Circle::Circle() : center(nullptr), r() {}
Circle::Circle(Point* p, double r) : center(p), r(r) {}
BoundBox2D Circle::getBox() const {
    return {center->x - r, center->x + r,center->y - r, center->y + r};
}
inline Element Circle::getType() const {
    return Element::ET_CIRCLE;
}

Arc::Arc() : beg(nullptr),end(nullptr), center(nullptr), r() {}
Arc::Arc(Point *p1, Point *p2, Point *c, double r): beg(p1), end(p2), center(c), r(r) {}
BoundBox2D Arc::getBox() const {
    return {center->x - r, center->x + r,center->y - r, center->y + r};
}
inline Element Arc::getType() const {
    return Element::ET_ARC;
}

