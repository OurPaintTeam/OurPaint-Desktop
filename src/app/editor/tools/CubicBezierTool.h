#ifndef OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_
#define OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_

#include <vector>

#include "../../../core/DocumentManager.h"
#include "Camera2D.h"
#include "IInteractionTool.h"
#include "OverlayModel.h"
#include "RenderScene.h"

class CubicBezierTool : public IInteractionTool {
public:
    explicit CubicBezierTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker);

    std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) override;
    std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) override;
    std::optional<ActionReport> onKey(const input::KeyEvent& e) override;
    ToolCancellation cancel() override;

private:
    std::vector<glm::dvec2> points_;

};

#endif // ! OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_
