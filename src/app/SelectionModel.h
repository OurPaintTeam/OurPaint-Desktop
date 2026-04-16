#ifndef OURPAINT_APPLICATION_SELECTION_MODEL_H_
#define OURPAINT_APPLICATION_SELECTION_MODEL_H_

#include "objects/Scene_ID.h"
#include <vector>

class SelectionModel {
public:
    bool empty() const;

    bool contains(SceneObjects::ID id) const;

    void clear();

    void replace(SceneObjects::ID id);
    void replace(const std::vector<SceneObjects::ID>& ids);

    void add(SceneObjects::ID id);
    void add(const std::vector<SceneObjects::ID>& ids);

    void toggle(SceneObjects::ID id);

    const std::vector<SceneObjects::ID>& items() const;

private:
    std::vector<SceneObjects::ID> items_;
    std::unordered_set<SceneObjects::ID> set_;
};

#endif // ! OURPAINT_APPLICATION_SELECTION_MODEL_H_