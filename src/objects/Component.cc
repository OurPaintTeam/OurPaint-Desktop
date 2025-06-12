#include "Component.h"

Component::Component() : _task(nullptr) {}

Component::Component(Component&& other) noexcept
        : _errorFunctions(std::move(other._errorFunctions)),
          _vars(std::move(other._vars)),
          _task(other._task),
          _objectIDs(std::move(other._objectIDs)) {
    other._task = nullptr;
}

Component& Component::operator=(Component&& other) noexcept {
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

Component::~Component() {
    cleanup();
}

bool Component::contains(ID id) const {
    return _objectIDs.contains(id);
}

void Component::cleanup() {
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

std::vector<Function*>& Component::errorFunctions() {
    return _errorFunctions;
}

const std::vector<Function*> Component::errorFunctions() const {
    return _errorFunctions;
}

std::vector<Variable*>& Component::vars() {
    return _vars;
}

const std::vector<Variable*> Component::vars() const {
    return _vars;
}

std::unordered_set<ID>& Component::objectIDs() {
    return _objectIDs;
}

const std::unordered_set<ID>& Component::objectIDs() const {
    return _objectIDs;
}

LSMFORLMTask*& Component::task() {
    return _task;
}

const LSMFORLMTask* Component::task() const {
    return _task;
}

bool Component::noErrorFunction() const {
    return _errorFunctions.empty();
}
