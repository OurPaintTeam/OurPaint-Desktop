#ifndef OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_
#define OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_

#include "../../../core/DocumentManager.h"
#include "Camera2D.h"
#include "IInteractionTool.h"
#include "RenderScene.h"

class CubicBezierTool : public IInteractionTool {
public:
    explicit CubicBezierTool(Document& document, Camera2D& camera);

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
    double firstPoint_X;
    double firstPoint_Y;
    Document& document_;
    Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_
