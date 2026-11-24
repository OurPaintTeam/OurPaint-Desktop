#ifndef OURPAINT_APPLICATION_CONSTRUCTION_TOOL_H_
#define OURPAINT_APPLICATION_CONSTRUCTION_TOOL_H_

#include <optional>
#include <span>

#include "IInteractionTool.h"

namespace app {
class ConstraintLayout;

class ConstructionTool : public IInteractionTool {
public:
    ConstructionTool(core::Document& document,
                     render::Camera2D& camera,
                     snap::SnapSystem& snap,
                     OverlayModel& overlay,
                     Cpu2dPicker& picker,
                     const ConstraintLayout& constraintLayout);

    bool canToggle() const;
    void toggleSelection();

    void onMouseButton(const input::MouseButtonEvent& event) override;
    void onMouseMove(const input::MouseMoveEvent&) override {}
    void onKey(const input::KeyEvent&) override {}
    bool cancel() override { return false; }

private:
    std::optional<bool> curveConstruction(core::sketch::GeometryRef ref) const;
    void toggleCurves(std::span<const core::sketch::GeometryRef> refs);

    const ConstraintLayout& constraintLayout_;
};
}  // namespace app

#endif
