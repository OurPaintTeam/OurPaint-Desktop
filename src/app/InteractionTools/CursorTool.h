#ifndef OURPAINT_APPLICATION_CURSOR_TOOL_H_
#define OURPAINT_APPLICATION_CURSOR_TOOL_H_

#include "Camera2D.h"
#include "DocumentManager.h"
#include "IInteractionTool.h"
#include "Objects.h"
#include "RenderData.h"

class CursorTool : public IInteractionTool {
public:
    explicit CursorTool(DocumentManager& documentManager, Camera2D& camera, renderer::RenderData& renderData);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    enum class State {
        WaitingStartMoving,
        Moving
    };

    State state_ = State::WaitingStartMoving;
    DocumentManager& documentManager_;
    Camera2D& camera_;
    renderer::RenderData& renderData_;
    ObjectData obj_;
    glm::dvec2 lastPos_{};
};

#endif // ! OURPAINT_APPLICATION_CURSOR_TOOL_H_