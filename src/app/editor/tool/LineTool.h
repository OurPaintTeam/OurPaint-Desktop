#ifndef OURPAINT_APPLICATION_LINE_TOOL_H_
#define OURPAINT_APPLICATION_LINE_TOOL_H_

#include "../../../core/DocumentManager.h"
#include "../../../render/Camera2D.h"
#include "../OverlayModel.h"
#include "IInteractionTool.h"

class LineTool : public IInteractionTool {
public:
    explicit LineTool(Document& document, render::Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    enum class State {
        WaitingFirstPoint,
        WaitingSecondPoint
    };

    State state_ = State::WaitingFirstPoint;
    glm::dvec2 firstPoint_;
    glm::dvec2 lastCursorWorldPos_;
    snap::SnapResult firstSnap_;
};

#endif // ! OURPAINT_APPLICATION_LINE_TOOL_H_
