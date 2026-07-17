#ifndef OURPAINT_APPLICATION_LINE_TOOL_H_
#define OURPAINT_APPLICATION_LINE_TOOL_H_

#include "../../viewport/OverlayModel.h"
#include "Camera2D.h"
#include "DocumentManager.h"
#include "IInteractionTool.h"

class LineTool : public IInteractionTool {
public:
    explicit LineTool(Document& document, Camera2D& camera, OverlayModel& overlay);

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

    Document& document_;
    Camera2D& camera_;
    OverlayModel& overlay_;
};

#endif // ! OURPAINT_APPLICATION_LINE_TOOL_H_