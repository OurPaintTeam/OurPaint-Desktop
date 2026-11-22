#include "SelectionModel.h"

#include <algorithm>

bool SelectionModel::empty() const { return items_.empty(); }

bool SelectionModel::contains(SelectionItem item) const { return set_.contains(item); }

void SelectionModel::clear() {
    items_.clear();
    set_.clear();
}

void SelectionModel::replace(SelectionItem item) {
    clear();
    add(item);
}

void SelectionModel::replace(const std::vector<SelectionItem>& items) {
    if (&items != &items_) {
        clear();
        for (const auto& item : items) {
            add(item);
        }
    }
}

void SelectionModel::replace(const std::vector<GeometryRef>& refs) {
    clear();
    add(refs);
}

void SelectionModel::add(SelectionItem item) {
    if (set_.insert(item).second) {
        items_.push_back(item);
    }
}

void SelectionModel::add(const std::vector<GeometryRef>& refs) {
    for (auto ref : refs) {
        add(ref);
    }
}

void SelectionModel::toggle(SelectionItem item) {
    if (set_.erase(item)) {
        std::erase(items_, item);
    } else {
        add(item);
    }
}

std::vector<core::sketch::GeometryRef> SelectionModel::geometryItems() const {
    std::vector<GeometryRef> refs;
    refs.reserve(items_.size());
    for (const auto& item : items_) {
        if (const auto* ref = std::get_if<GeometryRef>(&item)) {
            refs.push_back(*ref);
        }
    }
    return refs;
}
