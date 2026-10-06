#ifndef OURPAINT_APPLICATION_SELECTION_MODEL_H_
#define OURPAINT_APPLICATION_SELECTION_MODEL_H_

#include <functional>
#include <unordered_set>
#include <vector>

#include "SketchTypes.h"

class SelectionModel {
    using GeometryRef = core::sketch::GeometryRef;

public:
    bool empty() const;

    bool contains(GeometryRef ref) const;

    void clear();

    void replace(GeometryRef ref);
    void replace(const std::vector<GeometryRef>& refs);

    void add(GeometryRef ref);
    void add(const std::vector<GeometryRef>& refs);

    void toggle(GeometryRef ref);

    const std::vector<GeometryRef>& items() const;

private:
    struct RefHash {
        std::size_t operator()(GeometryRef ref) const noexcept { return std::hash<int64_t>{}(ref.entity.get()) * 4 + static_cast<std::size_t>(ref.sub); }
    };
    // Whole and point sub-elements remain independently selectable.
    std::vector<GeometryRef> items_;
    std::unordered_set<GeometryRef, RefHash> set_;
};

#endif  // ! OURPAINT_APPLICATION_SELECTION_MODEL_H_
