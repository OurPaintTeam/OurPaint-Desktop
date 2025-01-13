#ifndef OURPAINT_HEADERS_PAINTERS_PAINTER_H_
#define OURPAINT_HEADERS_PAINTERS_PAINTER_H_

#include "objects.h"

class Painter {

public:
    virtual unsigned long long getWeight()=0;
    virtual unsigned long long getHeight()=0;
    virtual void changeSize(const rectangle& allObjects)=0;

    virtual void drawPoint(point pt, bool isWhite = false)=0;
    virtual void drawSection(section sec, bool isWhite = false)=0;
    virtual void drawCircle(circle c, bool isWhite = false)=0;
};


#endif // ! OURPAINT_HEADERS_PAINTERS_PAINTER_H_
