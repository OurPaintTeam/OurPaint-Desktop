#ifndef OURPAINT_HEADERS_PAINTERS_PAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_PAINTER_H_

#include "Objects.h"

class Painter {

public:
    virtual unsigned long long getWeight()=0;
    virtual unsigned long long getHeight()=0;
    virtual void changeSize(const rectangle& allObjects)=0;

    virtual void drawPoint(Point pt, bool isWhite = false)=0;
    virtual void drawSection(Section sec, bool isWhite = false)=0;
    virtual void drawCircle(Circle c, bool isWhite = false)=0;

    virtual ~Painter() = default;
};


#endif // ! OURPAINT_HEADERS_PAINTERS_PAINTER_H_
