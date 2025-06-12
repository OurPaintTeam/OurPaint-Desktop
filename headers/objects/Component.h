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

class Component {
public:
    Component();

    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    Component(Component&&) noexcept;
    Component& operator=(Component&&) noexcept;

    ~Component();

    bool contains(ID id) const;

    std::vector<Function*>& errorFunctions();
    const std::vector<Function*> errorFunctions() const;

    std::vector<Variable*>& vars();
    const std::vector<Variable*> vars() const;

    std::unordered_set<ID>& objectIDs();
    const std::unordered_set<ID>& objectIDs() const;

    LSMFORLMTask*& task();
    const LSMFORLMTask* task() const;

    bool noErrorFunction() const;

private:
    std::vector<Function*> _errorFunctions;
    std::vector<Variable*> _vars;
    LSMFORLMTask* _task = nullptr;
    std::unordered_set<ID> _objectIDs;

    void cleanup();
};

#endif // ! OURPAINT_HEADERS_OBJECTS_COMPONENT_H_