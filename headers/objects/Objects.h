#ifndef OURPAINT_HEADERS_OBJECTS_H_
#define OURPAINT_HEADERS_OBJECTS_H_

#include "objects/Enums.h"
#include <vector>
#include <limits>

struct ObjectData {
    Element et;
    std::vector<double> params;

    bool operator==(const ObjectData& other) const {
        return et == other.et && params == other.params;
    }

    bool operator!=(const ObjectData& other) const {
        return (this == &other);
    }
};

#endif // ! OURPAINT_HEADERS_OBJECTS_H_