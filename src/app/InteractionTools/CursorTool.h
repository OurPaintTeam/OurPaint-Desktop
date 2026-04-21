#ifndef OURPAINT_APPLICATION_CURSOR_TOOL_H_
#define OURPAINT_APPLICATION_CURSOR_TOOL_H_

#include "Camera2D.h"
#include "DocumentManager.h"
#include "IInteractionTool.h"
#include "objects/Objects.h"
#include "../Cpu2dPicker.h"
#include "../OverlayModel.h"
#include <unordered_set>

class CursorTool : public IInteractionTool {
public:
    explicit CursorTool(DocumentManager& documentManager,
                        Camera2D& camera,
                        Cpu2dPicker& picker,
                        OverlayModel& overlay);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    enum class State {
        Idle,
        Pressed,
        DraggingSelection,
        MarqueeSelection
    };

    State state_ = State::Idle;
    DocumentManager& documentManager_;
    Camera2D& camera_;
    glm::dvec2 lastPos_{};
    glm::dvec2 pressWorldPos_{};
    glm::ivec2 lastScreenPos_{};
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;

    std::vector<SceneObjects::ID> marqueeBaseSelection_;

    std::vector<SceneObjects::ID> copiedObjects_;
    glm::dvec2 lastCursorPos_{};
    glm::dvec2 copiedPos_{};
    struct {
        glm::dvec2 minPos;
        glm::dvec2 maxPos;
    } copyBoundingBox_;
};

#endif // ! OURPAINT_APPLICATION_CURSOR_TOOL_H_