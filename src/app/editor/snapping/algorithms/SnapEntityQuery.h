#ifndef OURPAINT_SNAP_ENTITY_QUERY_H
#define OURPAINT_SNAP_ENTITY_QUERY_H

#include <algorithm>
#include <vector>
#include <span>

#include "geometry/GeometryTypes.h"
#include "sketch/SketchTypes.h"

// Object filtering belongs to the snapping query layer: geometry algorithms
// operate on neutral geometry, while the snap layer decides which document
// entities participate in this particular query.
namespace snap::algorithms {

inline bool isForbiddenEntity(const core::sketch::EntityId& id, std::span<const geometry::GeometryRef> forbiddenObjects) {
    return std::any_of(forbiddenObjects.begin(), forbiddenObjects.end(),
        [&](const geometry::GeometryRef& forbidden) { return forbidden.entity == id; });
}

inline std::vector<geometry::SketchEntity> filterForbiddenEntities(
    const std::vector<geometry::SketchEntity>& entities, std::span<const geometry::GeometryRef> forbiddenObjects) {
    std::vector<geometry::SketchEntity> filtered;
    filtered.reserve(entities.size());
    for (const auto& entity : entities) {
        if (!isForbiddenEntity(entity.id, forbiddenObjects)) {
            filtered.push_back(entity);
        }
    }
    return filtered;
}

}  // namespace snap::algorithms

#endif  // OURPAINT_SNAP_ENTITY_QUERY_H
