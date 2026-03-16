#ifndef HEADERS_REQUIREMENTS_COMPONENTS_H
#define HEADERS_REQUIREMENTS_COMPONENTS_H
#include <vector>
#include <set>
//forward declaration
class ErrorFunction;
class Variable;
namespace OurPaintDCM::Utils {
struct ID;
}



namespace OurPaintDCM::Requirements {
/**
 * @brief struct which describe one item of component
 */
struct ComponentElement {
    Utils::ID id; ///< ID of requirement(in graph it is edge)
    ErrorFunction* func; ///< Function of Error that used in solving
    ~ComponentElement() {
        delete func;
    }
};

/**
 * @brief class which describe all components
 *
 * This class is used to store all components
 * and to delete them
 *
 */
class Components {
    std::vector<ComponentElement> _elems; ///< vector of components
    std::set<Variable*> _vars; ///< set of variables
    public:
    /**
     * @brief default constructor
     */
    Components(): _elems(), _vars() {}

    /**
     * @brief add element to components
     * @param element - element to add
     */
    void addElement(ComponentElement& element) {
        _elems.push_back(element);
        for (auto& var : element.func->getVariables()) {
            _vars.insert(var);
        }
    }
    /**
     *@brief delete element from components
     * @param id - ID of element
     */
    void deleteElement(const Utils::ID& id) {
        for (auto it = _elems.begin(); it != _elems.end(); ++it) {
            if (it->id == id) {
                _elems.erase(it);
                break;
            }
        }
    }
    size_t varsSize() const {
        return _vars.size();
    }
    size_t elemsSize() const {
        return _elems.size();
    }

    void clear() {
        _vars.clear();
        _elems.clear();
    }

    const std::vector<ComponentElement>& getElements() const {
        return _elems;
    }

    const std::set<Variable*>& getVariables() const {
        return _vars;
    }
};
}
#endif //HEADERS_REQUIREMENTS_COMPONENTS_H
