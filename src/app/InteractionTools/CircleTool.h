#ifndef OURPAINT_APPLICATION_CIRCLE_TOOL_H_
#define OURPAINT_APPLICATION_CIRCLE_TOOL_H_

#include "DocumentManager.h"
#include "Camera2D.h"
#include "IInteractionTool.h"
#include "RenderData.h"

class CircleTool : public IInteractionTool {
public:
    explicit CircleTool(DocumentManager& documentManager_, Camera2D& camera, renderer::RenderData& renderData);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    void onCancel() override;

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
    renderer::RenderData& renderData_;
};

#endif // ! OURPAINT_APPLICATION_CIRCLE_TOOL_H_