#ifndef OURPAINT_HEADERS_OBJECTS_COMPONENT_H_
#define OURPAINT_HEADERS_OBJECTS_COMPONENT_H_

#include <vector>
#include <unordered_set>
#include "ID.h"
#include "Task.h"
#include "Function.h"
#include "ErrorFunction.h"
#include "LSMFORLMTask.h"

/**
 * Definition of the Component class — a technical scene component
 * used within Scene to manage connectivity.
 * It stores and manages the lifecycle of error functions,
 * variables, and associated IDs.
 */

struct Component {
    std::vector<Function*> _errorFunctions;
    std::vector<Variable*> _vars;
    LSMFORLMTask* _task = nullptr;
    std::unordered_set<ID> _objectIDs;

    Component() : _task(nullptr) {}

    ~Component() {
        cleanup();
    }

    Component(Component&& other) noexcept
            : _errorFunctions(std::move(other._errorFunctions)),
              _vars(std::move(other._vars)),
              _task(other._task),
              _objectIDs(std::move(other._objectIDs)) {
        other._task = nullptr;
    }

    Component& operator=(Component&& other) noexcept {
        if (this != &other) {
            cleanup();

            _errorFunctions = std::move(other._errorFunctions);
            _vars = std::move(other._vars);
            _task = other._task;
            _objectIDs = std::move(other._objectIDs);

            other._task = nullptr;
        }
        return *this;
    }

    void cleanup() {
        for (auto& f: _errorFunctions) {
            delete f;
        }
        _errorFunctions.clear();

        delete _task;
        _task = nullptr;

        for (auto& c: _vars) {
            delete c;
        }
        _vars.clear();

        _objectIDs.clear();
    }

    inline bool contains(ID id) const {
        return _objectIDs.contains(id);
    }

    inline bool noErrorFunction() const {
        return _errorFunctions.empty();
    }
};

#endif // ! OURPAINT_HEADERS_OBJECTS_COMPONENT_H_