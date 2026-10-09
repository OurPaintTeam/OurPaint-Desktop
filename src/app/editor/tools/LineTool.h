#ifndef OURPAINT_APPLICATION_LINE_TOOL_H_
#define OURPAINT_APPLICATION_LINE_TOOL_H_

#include "../../../core/DocumentManager.h"
#include "../../viewport/OverlayModel.h"
#include "Camera2D.h"
#include "IInteractionTool.h"

class LineTool : public IInteractionTool {
public:
    explicit LineTool(Document& document, Camera2D& camera, OverlayModel& overlay);

    std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) override;
    std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) override;
    std::optional<ActionReport> onKey(const input::KeyEvent& e) override;
    ToolCancellation cancel() override;

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
