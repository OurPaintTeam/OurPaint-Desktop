#ifndef OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_
#define OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_

#include "DocumentManager.h"
#include "Camera2D.h"
#include "IInteractionTool.h"
#include "RenderScene.h"

class CubicBezierTool : public IInteractionTool {
public:
    explicit CubicBezierTool(DocumentManager& documentManager, Camera2D& camera, render::RenderScene& renderData);

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
    DocumentManager& documentManager_;
    Camera2D& camera_;
    render::RenderScene& renderData_;
};

#endif // ! OURPAINT_APPLICATION_CUBIC_BEZIER_TOOL_H_