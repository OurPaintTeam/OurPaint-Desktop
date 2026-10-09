#ifndef OURPAINT_APPLICATION_CONSTRAINT_TOOL_H_
#define OURPAINT_APPLICATION_CONSTRAINT_TOOL_H_

#include <span>
#include <vector>

#include "../../../core/sketch/SketchTypes.h"
#include "../ConstraintRequest.h"
#include "IInteractionTool.h"

class ConstraintActions;
class Cpu2dPicker;
class OverlayModel;

class ConstraintTool : public IInteractionTool {
public:
    ConstraintTool(ConstraintActions& actions, Cpu2dPicker& picker, OverlayModel& overlay);

    void begin(const ConstraintRequest& request, std::span<const core::sketch::GeometryRef> initialRefs = {});

    std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) override;
    std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) override;
    std::optional<ActionReport> onKey(const input::KeyEvent& e) override;
    ToolCancellation cancel() override;

private:
    std::optional<ActionReport> handleRef(const core::sketch::GeometryRef& ref);
    void resetInputs();

    ConstraintActions& actions_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;

    ConstraintRequest request_{ConstraintAction::Coincident, std::nullopt};
    std::vector<core::sketch::GeometryRef> refs_;
};

#endif  // ! OURPAINT_APPLICATION_CONSTRAINT_TOOL_H_
