#ifndef OURPAINT_APPLICATION_POINT_TOOL_H_
#define OURPAINT_APPLICATION_POINT_TOOL_H_

#include "../../../core/DocumentManager.h"
#include "Camera2D.h"
#include "IInteractionTool.h"
#include "snapping/SnapSystem.h"

class OverlayModel;
class PointTool : public IInteractionTool {
public:
    explicit PointTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker);

    std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) override;
    std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) override;
    std::optional<ActionReport> onKey(const input::KeyEvent& e) override;
    ToolCancellation cancel() override;

};

#endif // ! OURPAINT_APPLICATION_POINT_TOOL_H_
