#ifndef OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
#define OURPAINT_HEADERS_GEOMETRICOBJECTS_H_

#include <fstream>

#include "objects/Enums.h"
#include "objects/ID.h"
#include "objects/Objects.h"
#include "objects/BoundingBox.h"
#include "Painter.h"

struct IGeometricObject {
public:
    virtual ~IGeometricObject() = default;

    virtual BoundingBox getBox() = 0;
    virtual inline Element getElementType() = 0;
};

struct Point : public IGeometricObject {
    double x;
    double y;

    Point();
    Point(double x, double y);

    BoundingBox getBox() override;
    inline Element getElementType() override;
};

std::ifstream &operator>>(std::ifstream &in, Point &x);

struct Section : public IGeometricObject {
    Point *beg;
    Point *end;

    Section();
    Section(Point* p1, Point* p2);

    BoundingBox getBox() override;
    inline Element getElementType() override;
};

struct Circle : public IGeometricObject {
    Point *center;
    double r;

    Circle();
    Circle(Point* p, double r);

    BoundingBox getBox() override;
    inline Element getElementType() override;
};

#endif // ! OURPAINT_HEADERS_GEOMETRICOBJECTS_H_
