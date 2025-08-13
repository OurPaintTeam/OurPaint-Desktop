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
    double x1 = s.beg->x, y1 = s.beg->y;
    double x2 = s.end->x, y2 = s.end->y;
    BoundBox2D bb;
    bb.min_x = std::min(x1, x2);
    bb.max_x = std::max(x1, x2);
    bb.min_y = std::min(y1, y2);
    bb.max_y = std::max(y1, y2);
    return bb;
}

inline BoundBox2D getBox(const Circle& c) {
    return {c.center->x - c.r, c.center->x + c.r, c.center->y - c.r, c.center->y + c.r};
}

inline BoundBox2D getBox(const Arc&) {
    return BoundBox2D{}; // TODO
}

#endif // ! OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
