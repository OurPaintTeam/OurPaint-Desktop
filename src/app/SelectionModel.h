#ifndef OURPAINT_APPLICATION_SELECTION_MODEL_H_
#define OURPAINT_APPLICATION_SELECTION_MODEL_H_

#include "objects/ID.h"
#include <vector>

using namespace core;

class SelectionModel {
public:
    bool empty() const;

    bool contains(ID id) const;

    void clear();

    void replace(ID id);
    void replace(const std::vector<ID>& ids);

    void add(ID id);
    void add(const std::vector<ID>& ids);

    void toggle(ID id);

    const std::vector<ID>& items() const;

private:
    std::vector<ID> items_;
    std::unordered_set<ID> set_;
};

#endif // ! OURPAINT_APPLICATION_SELECTION_MODEL_H_