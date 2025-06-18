#ifndef OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
#define OURPAINT_HEADERS_GEOMETRICOBJECTS_H_

#include <fstream>

#include "Enums.h"
#include "ID.h"
#include "Objects.h"
#include "BoundBox.h"

struct IGeometricObject {
    virtual ~IGeometricObject() = default;

    virtual BoundBox2D getBox() const = 0;
    virtual inline Element getType() const = 0;
};

struct Point : IGeometricObject {
    double x;
    double y;

    Point();
    Point(double x, double y);

    BoundBox2D getBox() const override;
    inline Element getType() const override;
};

std::ifstream &operator>>(std::ifstream &in, Point &x);

struct Section : IGeometricObject {
    Point *beg;
    Point *end;

    Section();
    Section(Point* p1, Point* p2);

    BoundBox2D getBox() const override;
    inline Element getType() const override;
};

struct Circle : IGeometricObject {
    Point *center;
    double r;

    Circle();
    Circle(Point* p, double r);

    BoundBox2D getBox() const override;
    inline Element getType() const override;
};

struct Arc : IGeometricObject {
    Point *beg;
    Point *end;
    Point *center;
    double r;

    Arc();
    Arc(Point* p1, Point* p2, Point* c, double r);

    BoundBox2D getBox() const override;
    inline Element getType() const override;
};

#endif // ! OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
