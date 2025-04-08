#ifndef OURPAINT_HEADERS_OBJECTS_H_
#define OURPAINT_HEADERS_OBJECTS_H_

#include <fstream>

#include "Enums.h"
#include "ID.h"

struct rectangle {
    double x_1;
    double x_2;
    double y_1;
    double y_2;

    rectangle operator||(const rectangle &r);
};

struct IGeometricObject {
public:
    virtual rectangle rect() = 0;
    virtual inline Element type() = 0;
};



struct Point : public IGeometricObject {
    double x;
    double y;

    rectangle rect() override;
    inline Element type() override;
};

std::ifstream &operator>>(std::ifstream &in, Point &x);

struct Section : public IGeometricObject {
    Point *beg;
    Point *end;

    rectangle rect() override;
    inline Element type() override;
};

struct Circle : public IGeometricObject {
    Point *center;
    double R;

    rectangle rect() override;
    inline Element type() override;
};

#endif // ! OURPAINT_HEADERS_OBJECTS_H_
