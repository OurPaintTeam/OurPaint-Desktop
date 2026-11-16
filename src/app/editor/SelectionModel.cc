#include "SelectionModel.h"

#include <algorithm>

bool SelectionModel::empty() const { return items_.empty(); }

bool SelectionModel::contains(GeometryRef ref) const { return set_.contains(ref); }

void SelectionModel::clear() {
    items_.clear();
    set_.clear();
}

void SelectionModel::replace(GeometryRef ref) {
    items_ = {ref};
    set_.clear();
    set_.insert(ref);
}

void SelectionModel::replace(const std::vector<GeometryRef>& refs) {
    if (&refs != &items_) {
        clear();
        add(refs);
    }
}

void SelectionModel::add(GeometryRef ref) {
    if (set_.insert(ref).second) {
        items_.push_back(ref);
    }
}

void SelectionModel::add(const std::vector<GeometryRef>& refs) {
    for (auto ref : refs) {
        add(ref);
    }
}

void SelectionModel::toggle(GeometryRef ref) {
    if (set_.erase(ref)) {
        std::erase(items_, ref);
    } else {
        add(ref);
    }
}

const std::vector<core::sketch::GeometryRef>& SelectionModel::items() const { return items_; }
