#ifndef OURPAINT_HEADERS_OBJECTS_H_
#define OURPAINT_HEADERS_OBJECTS_H_

#include "Enums.h"
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

struct RequirementData {
    Requirement req;
    std::vector<unsigned int> objects;
    std::vector<double> params;

    bool operator==(const RequirementData& other) const {
        return req == other.req
               && objects == other.objects
               && params == other.params;
    }

    bool operator!=(const RequirementData& other) const {
        return !(*this == other);
    }
};

#endif // ! OURPAINT_HEADERS_OBJECTS_H_