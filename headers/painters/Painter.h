#ifndef OURPAINT_HEADERS_PAINTERS_PAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_PAINTER_H_

#include <unordered_map>
#include "GeometricObjects.h"
#include "ID.h"
#include "BoundBox.h"

class Painter {

public:
    virtual unsigned long long getWeight() = 0;
    virtual unsigned long long getHeight() = 0;
    virtual void changeSize(const BoundBox2D& allObjects) = 0;

    virtual void drawPoint(std::unordered_map<ID, Point*>& points) = 0;
    virtual void drawSection(std::unordered_map<ID, Section*>& sections) = 0;
    virtual void drawCircle(std::unordered_map<ID, Circle*>& circles) = 0;

    virtual ~Painter() = default;
};


#endif // ! OURPAINT_HEADERS_PAINTERS_PAINTER_H_
