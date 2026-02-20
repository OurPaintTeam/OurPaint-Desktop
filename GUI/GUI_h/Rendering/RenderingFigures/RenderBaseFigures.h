#ifndef OURPAINT_DRAWINGFIGURES_H
#define OURPAINT_DRAWINGFIGURES_H

#include <QPainter>
#include <unordered_map>

#include "ID.h"



// Base interface
namespace render {
    inline bool isValidID(const ID& id) {
        return id != ID(0);
    }

}




#endif // OURPAINT_DRAWINGFIGURES_H
