#include "SelectionModel.h"

bool SelectionModel::empty() const {
    return items_.empty();
}

bool SelectionModel::contains(SceneObjects::ID id) const {
    return set_.contains(id);
}

void SelectionModel::clear() {
    items_.clear();
    set_.clear();
}

void SelectionModel::replace(SceneObjects::ID id) {
    items_ = {id};
    set_.clear();
    set_.insert(id);
}

void SelectionModel::replace(const std::vector<SceneObjects::ID>& ids) {
    items_ = ids;
    set_.clear();
    set_.insert(ids.begin(), ids.end());
}

void SelectionModel::add(SceneObjects::ID id) {
    if (!set_.contains(id)) {
        set_.insert(id);
        items_.push_back(id);
    }
}

void SelectionModel::add(const std::vector<SceneObjects::ID>& ids) {
    set_.insert(ids.begin(), ids.end());
    items_.insert(items_.end(), ids.begin(), ids.end());
}

void SelectionModel::toggle(SceneObjects::ID id) {
    if (set_.contains(id)) {
        set_.erase(id);
        std::erase(items_, id);
    } else {
        add(id);
    }
}

const std::vector<SceneObjects::ID>& SelectionModel::items() const {
    return items_;
}




