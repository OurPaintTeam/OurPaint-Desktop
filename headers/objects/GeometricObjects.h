#ifndef OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
#define OURPAINT_HEADERS_GEOMETRICOBJECTS_H_

#include <fstream>

#include "objects/Enums.h"
#include "objects/ID.h"
#include "objects/Objects.h"
#include "objects/BoundBox.h"
#include "Painter.h"

struct IGeometricObject {
public:
    virtual ~IGeometricObject() = default;

    virtual BoundBox2D getBox() const = 0;
    virtual inline Element getElementType() const = 0;
};

struct Point : public IGeometricObject {
    double x;
    double y;

    Point();
    Point(double x, double y);

    BoundBox2D getBox() const override;
    inline Element getElementType() const override;
};

std::ifstream &operator>>(std::ifstream &in, Point &x);

struct Section : public IGeometricObject {
    Point *beg;
    Point *end;

    Section();
    Section(Point* p1, Point* p2);

    BoundBox2D getBox() const override;
    inline Element getElementType() const override;
};

struct Circle : public IGeometricObject {
    Point *center;
    double r;

    Circle();
    Circle(Point* p, double r);

    BoundBox2D getBox() const override;
    inline Element getElementType() const override;
};

#endif // ! OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
