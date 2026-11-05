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

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    unsigned int degree = 3;
    std::vector<glm::dvec2> points_;
};

#endif // ! OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_
