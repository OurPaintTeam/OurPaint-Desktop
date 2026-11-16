#ifndef OURPAINT_APPLICATION_CONSTRAINT_TOOL_H_
#define OURPAINT_APPLICATION_CONSTRAINT_TOOL_H_

#include <span>
#include <vector>

#include "../../../core/sketch/SketchTypes.h"
#include "../ConstraintRequest.h"
#include "IInteractionTool.h"
#include "ToolId.h"

class ConstraintActions;
class Cpu2dPicker;
class OverlayModel;

namespace {
    using namespace core;
}

class ConstraintTool : public IInteractionTool {
public:
    ConstraintTool(Document& document,
                   render::Camera2D& camera,
                   snap::SnapSystem& snap,
                   OverlayModel& overlay,
                   Cpu2dPicker& picker);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;

    bool cancel() override;

    void setValue(double value);
    void setSubToolType(ToolId type);

private:
    void pushConstraint();
    void pushEntityToOverlay(const sketch::GeometryRef& ref) const;

    void pushPointToOverlay(double x, double y) const;
    void pushLineToOverlay(double x1, double y1, double x2, double y2) const;
    void pushCircleToOverlay(double x, double y, double r) const;
    void pushArcToOverlay(double x, double y, double r, double startAngle, double endAngle) const;

private:
    double value_{};
    bool isValueSet_ = false;
    ToolId subTool_;
    bool isConstraintTypeSet_ = false;

    std::optional<sketch::ConstraintType> constraintType_ = std::nullopt;
    std::vector<sketch::GeometryRef> refs_;
    bool coincidentMidpoint_ = false;

    sketch::Sketch& sketch_;
};

#endif  // ! OURPAINT_APPLICATION_CONSTRAINT_TOOL_H_
