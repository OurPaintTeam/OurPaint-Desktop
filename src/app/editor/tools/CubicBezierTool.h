#ifndef OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_
#define OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_

#include "Camera2D.h"
#include "DocumentManager.h"
#include "IInteractionTool.h"
#include "RenderScene.h"

class CubicBezierTool : public IInteractionTool {
public:
    explicit CubicBezierTool(Document& document, Camera2D& camera);

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
    double firstPoint_X;
    double firstPoint_Y;
    Document& document_;
    Camera2D& camera_;
};

#endif // ! OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_