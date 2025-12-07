#ifndef OURPAINT_HEADERS_PAINTERS_PAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_PAINTER_H_

#include <unordered_map>
#include "GeometricObjects.h"
#include "ID.h"
#include "BoundBox.h"

class Painter {
protected:
    std::unordered_map<ID, Point*>* casePoints;
    std::unordered_map<ID, Section*>* caseSections;
    std::unordered_map<ID, Circle*>* caseCircles;
    std::unordered_map<ID, Arc*>* caseArcs;
    const BoundBox2D* rectangle;

public:
    Painter() : casePoints(nullptr),
                caseSections(nullptr),
                caseCircles(nullptr),
                caseArcs(nullptr),
                rectangle(nullptr) {}

    virtual void draw() = 0;
    virtual void clear() = 0;

    virtual unsigned long long getWeight() = 0;
    virtual unsigned long long getHeight() = 0;
    virtual void getBoundBox(const BoundBox2D& allObjects) = 0;

    virtual void initPointCase(std::unordered_map<ID, Point*>& points) = 0;
    virtual void initSectionCase(std::unordered_map<ID, Section*>& sections) = 0;
    virtual void initCircleCase(std::unordered_map<ID, Circle*>& circles) = 0;
    virtual void initArcCase(std::unordered_map<ID, Arc*>& Arcs) = 0;

    virtual ~Painter() = default;
};


#endif // ! OURPAINT_HEADERS_PAINTERS_PAINTER_H_
