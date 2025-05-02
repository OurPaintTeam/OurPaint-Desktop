#ifndef OURPAINT_HEADERS_OBJECTS_H_
#define OURPAINT_HEADERS_OBJECTS_H_

#include "Enums.h"
#include <vector>
#include <limits>
#include <unordered_map>
#include "ErrorFunctions.h"

#define PARAM_ID double*

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

class VarsStorage {
private:
    std::unordered_map<PARAM_ID, Variable*> _vars;

public:
    std::vector<Variable*> getVars(PARAM_ID id) {
        std::vector<Variable*> vars;
        for (auto var : _vars) {
            vars.push_back(var.second);
        }
        return vars;
    };

    void addVar(PARAM_ID id) {
        if (_vars.find(id) == _vars.end()) {
            _vars[id] = new Variable(id);
        }
    };

    Variable* getVar(PARAM_ID id) {
        if (_vars.find(id) == _vars.end()) {
            throw std::runtime_error("Variable not found");
        }
        return _vars[id];
    };

    void clearVars() {
        for (auto var : _vars) {
            delete var.second;
        }
        _vars.clear();
    };

    bool hasVar(PARAM_ID id) const {
        return _vars.find(id) != _vars.end();
    };

    void removeVar(PARAM_ID id) {
        if (_vars.find(id) == _vars.end()) {
            throw std::runtime_error("Variable not found");
        }
        delete _vars[id];
        _vars.erase(id);
    };
};

#endif // ! OURPAINT_HEADERS_OBJECTS_H_