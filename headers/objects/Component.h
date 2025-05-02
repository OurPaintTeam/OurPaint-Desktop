#ifndef OURPAINT_HEADERS_OBJECTS_COMPONENT_H_
#define OURPAINT_HEADERS_OBJECTS_COMPONENT_H_

#include <vector>
#include <unordered_set>
#include "ID.h"
#include "Function.h"


struct Component {
    Component() = default;
    Component(const Component&) = default;
    Component(Component&&) = default;

    Component& operator=(const Component&) = default;
    Component& operator=(Component&&) = default;

    ~Component() {
        for (auto& f : _errorRequirementFunctions) {
            delete f;
        }
        _errorRequirementFunctions.clear();
        for (auto& c : _componentVars) {
            delete c;
        }
        _componentVars.clear();
        _objectIDs.clear();
    }

    std::vector<Function*> _errorRequirementFunctions;
    std::vector<Variable*> _componentVars;
    std::unordered_set<ID> _objectIDs;

    bool contains(ID id) const {
        return _objectIDs.contains(id);
    }
};

#endif // ! OURPAINT_HEADERS_OBJECTS_COMPONENT_H_