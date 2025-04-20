#ifndef OURPAINT_HEADERS_PAINTERS_PAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_PAINTER_H_

#include "GeometricObjects.h"
#include "ID.h"
#include "BoundBox.h"

class Painter {

public:
    virtual unsigned long long getWeight() = 0;
    virtual unsigned long long getHeight() = 0;
    virtual void changeSize(const BoundBox2D& allObjects) = 0;

    virtual void drawPoint(const std::unordered_map<ID, const Point*>& points) = 0;
    virtual void drawSection(const std::unordered_map<ID, const Section*>& sections) = 0;
    virtual void drawCircle(const std::unordered_map<ID, const Circle*>& circles) = 0;

    virtual ~Painter() = default;
};


#endif // ! OURPAINT_HEADERS_PAINTERS_PAINTER_H_
