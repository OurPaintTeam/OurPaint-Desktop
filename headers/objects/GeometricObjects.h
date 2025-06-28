#ifndef OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
#define OURPAINT_HEADERS_GEOMETRICOBJECTS_H_

#include <fstream>
#include "BoundBox.h"

struct Point {
    double x;
    double y;
};

struct Section {
    Point* beg;
    Point* end;
};

struct Circle {
    Point* center;
    double r;
};

struct Arc {
    Point* beg;
    Point* end;
    Point* center;
};

inline std::ifstream& operator>>(std::ifstream& in, Point& x) {
    in >> x.x;
    in >> x.y;
    return in;
}

inline BoundBox2D getBox(const Point& p) {
    return {p.x, p.x, p.y, p.y};
}

inline BoundBox2D getBox(const Section& s) {
    return {s.beg->x, s.end->x, s.beg->y, s.beg->y};
}

inline BoundBox2D getBox(const Circle& c) {
    return {c.center->x - c.r, c.center->x + c.r, c.center->y - c.r, c.center->y + c.r};
}

inline BoundBox2D getBox(const Arc&) {
    return BoundBox2D{}; // TODO
}

#endif // ! OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
