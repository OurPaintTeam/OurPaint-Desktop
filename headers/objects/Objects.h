#ifndef OURPAINT_HEADERS_OBJECTS_H_
#define OURPAINT_HEADERS_OBJECTS_H_

#include "Enums.h"
#include "ID.h"
#include <vector>
#include <limits>
#include <unordered_map>
#include "ErrorFunctions.h"

#define PARAM_ID double*

struct ObjectData {
    Element et;
    std::vector<ID> subObjects;
    std::vector<double> params;
    ID id;

    bool operator==(const ObjectData& other) const {
        return et == other.et && params == other.params;
    }

    bool operator!=(const ObjectData& other) const {
        return !(*this == other);
    }
};

struct RequirementData {
    Requirement req;
    std::vector<ID> objects;
    std::vector<double> params;
    ID id;

    bool operator==(const RequirementData& other) const {
        return req == other.req
                && objects == other.objects
                && params == other.params
                && id == other.id;
    }

    bool operator!=(const RequirementData& other) const {
        return !(*this == other);
    }
};

namespace std {
    template <>
    struct hash<RequirementData> {
        std::size_t operator()(const RequirementData& rd) const {
            std::size_t h1 = std::hash<Requirement>()(rd.req);
            std::size_t h2 = 0;
            for (const auto& obj : rd.objects) {
                h2 ^= std::hash<ID>()(obj) + 0x9e3779b9 + (h2 << 6) + (h2 >> 2);
            }
            std::size_t h3 = 0;
            for (const auto& param : rd.params) {
                h3 ^= std::hash<double>()(param) + 0x9e3779b9 + (h3 << 6) + (h3 >> 2);
            }
            std::size_t h4 = std::hash<ID>()(rd.id);

            return h1 ^ h2 ^ h3 ^ h4;
        }
    };
}

class VarsStorage {
private:
    std::unordered_map<PARAM_ID, Variable*> _vars;

public:
    std::vector<Variable*> getVars() {
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