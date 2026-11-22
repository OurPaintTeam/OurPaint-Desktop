#ifndef OURPAINT_APPLICATION_SELECTION_MODEL_H_
#define OURPAINT_APPLICATION_SELECTION_MODEL_H_

#include <functional>
#include <unordered_set>
#include <variant>
#include <vector>

#include "SketchTypes.h"

using SelectionItem = std::variant<core::sketch::GeometryRef, core::sketch::ConstraintId>;

class SelectionModel {
    using GeometryRef = core::sketch::GeometryRef;

public:
    bool empty() const;

    bool contains(SelectionItem item) const;

    void clear();

    void replace(SelectionItem item);
    void replace(const std::vector<SelectionItem>& items);
    void replace(const std::vector<GeometryRef>& refs);

    void add(SelectionItem item);
    void add(const std::vector<GeometryRef>& refs);

    void toggle(SelectionItem item);

    const std::vector<SelectionItem>& items() const { return items_; }
    std::vector<GeometryRef> geometryItems() const;

private:
    struct ItemHash {
        std::size_t operator()(const SelectionItem& item) const noexcept {
            if (const auto* ref = std::get_if<GeometryRef>(&item)) {
                const auto ownerAndRole = std::hash<int64_t>{}(ref->entity.get()) * 31 + static_cast<std::size_t>(ref->sub);
                return ownerAndRole * 31 + std::hash<size_t>{}(ref->index);
            }
            return std::hash<int64_t>{}(std::get<core::sketch::ConstraintId>(item).get());
        }
    };
    // Whole and point sub-elements remain independently selectable.
    std::vector<SelectionItem> items_;
    std::unordered_set<SelectionItem, ItemHash> set_;
};

#endif  // ! OURPAINT_APPLICATION_SELECTION_MODEL_H_
