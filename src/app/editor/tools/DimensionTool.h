#ifndef OURPAINT_APPLICATION_DIMENSION_TOOL_H_
#define OURPAINT_APPLICATION_DIMENSION_TOOL_H_

#include <span>
#include <vector>

#include "../../../core/sketch/SketchTypes.h"
#include "../ConstraintRequest.h"
#include "IInteractionTool.h"

class ConstraintActions;
class Cpu2dPicker;
class OverlayModel;

class DimensionTool : public IInteractionTool {
public:
    DimensionTool(ConstraintActions& actions, Cpu2dPicker& picker, OverlayModel& overlay);

    void begin(const ConstraintRequest& request, std::span<const core::sketch::GeometryRef> initialRefs = {});

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    void handleRef(const core::sketch::GeometryRef& ref);
    void resetInputs();

    ConstraintActions& actions_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;

    ConstraintRequest request_{ConstraintAction::Dimension, std::nullopt};
    std::vector<core::sketch::GeometryRef> refs_;
};

#endif  // ! OURPAINT_APPLICATION_DIMENSION_TOOL_H_
