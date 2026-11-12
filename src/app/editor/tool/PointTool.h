#ifndef OURPAINT_APPLICATION_POINT_TOOL_H_
#define OURPAINT_APPLICATION_POINT_TOOL_H_

#include "../../../core/DocumentManager.h"
#include "../../../render/Camera2D.h"
#include "IInteractionTool.h"
#include "snap/SnapSystem.h"

class OverlayModel;
class PointTool : public IInteractionTool {
public:
    explicit PointTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;
};

#endif // ! OURPAINT_APPLICATION_POINT_TOOL_H_
