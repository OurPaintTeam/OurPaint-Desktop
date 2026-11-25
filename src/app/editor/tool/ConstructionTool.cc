#include "ConstructionTool.h"

#include <algorithm>
#include <vector>

#include "Cpu2dPicker.h"

namespace app {
using namespace core::sketch;

ConstructionTool::ConstructionTool(core::Document& document,
                                   render::Camera2D& camera,
                                   snap::SnapSystem& snap,
                                   OverlayModel& overlay,
                                   Cpu2dPicker& picker,
                                   const ConstraintLayout& constraintLayout)
    : IInteractionTool(document, camera, snap, overlay, picker), constraintLayout_(constraintLayout) {}

std::optional<bool> ConstructionTool::curveConstruction(GeometryRef ref) const {
    if (ref.sub != SubElement::Whole) {
        return std::nullopt;
    }
    const auto entity = document_.sketch().entity(ref.entity);
    if (!entity || std::holds_alternative<Point2>(entity.value().geometry)) {
        return std::nullopt;
    }
    return entity.value().construction;
}

bool ConstructionTool::canToggle() const {
    const auto& items = overlay_.selection_.model.items();
    return items.empty() || std::ranges::any_of(items, [this](const SelectionItem& item) {
        const auto* ref = std::get_if<GeometryRef>(&item);
        return ref && curveConstruction(*ref).has_value();
    });
}

void ConstructionTool::toggleSelection() {
    toggleCurves(overlay_.selection_.model.geometryItems());
}

void ConstructionTool::toggleCurves(std::span<const GeometryRef> refs) {
    std::vector<EntityId> regular;
    std::vector<EntityId> construction;
    for (const auto ref : refs) {
        if (const auto flag = curveConstruction(ref)) {
            (*flag ? construction : regular).push_back(ref.entity);
        }
    }

    const bool makeConstruction = !regular.empty();
    const auto& changed = makeConstruction ? regular : construction;
    if (changed.empty()) {
        return;
    }
    auto& sketch = document_.sketch();
    for (auto id : changed) {
        // Each ID was resolved above; setConstruction only changes its metadata.
        sketch.setConstruction(id, makeConstruction);
    }
    sketch.pushState();
}

void ConstructionTool::onMouseButton(const input::MouseButtonEvent& event) {
    if (event.button != input::MouseButton::Left || event.action != input::MouseButtonAction::Press) {
        return;
    }
    if (const auto hit = picker_.pickAtScreenLogical(event.x, event.y, constraintLayout_)) {
        if (const auto* geometry = std::get_if<GeometryPickResult>(&*hit)) {
            toggleCurves({&geometry->ref, 1});
        }
    }
}
}  // namespace app
