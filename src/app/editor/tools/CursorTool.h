#ifndef OURPAINT_APPLICATION_CURSOR_TOOL_H_
#define OURPAINT_APPLICATION_CURSOR_TOOL_H_

#include "../../../core/Document.h"
#include "../../viewport/OverlayModel.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "Camera2D.h"
#include "IInteractionTool.h"

using namespace core;

class ConstraintActions;

class CursorTool : public IInteractionTool {
public:
    explicit CursorTool(Document& document, Camera2D& camera, Cpu2dPicker& picker, OverlayModel& overlay, ConstraintActions& constraintActions);

    std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) override;
    std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) override;
    std::optional<ActionReport> onKey(const input::KeyEvent& e) override;
    ToolCancellation cancel() override;

private:
    void copySelection();
    std::optional<ActionReport> pasteSelection();
    sketch::Status prepareDragSelection(std::span<const sketch::GeometryRef> refs);
    sketch::Result<sketch::SolveDiagnostics> moveSelection(sketch::Vec2 offset);
    std::optional<ActionReport> finishDrag();

    ActionReport tryApplyPointOnPointNearCursor(double xLogic, double yLogic);

    enum class State {
        Idle,
        Pressed,
        DraggingSelection,
        MarqueeSelection
    };

    State state_ = State::Idle;
    Document& document_;
    sketch::Sketch& sketch_;

    Camera2D& camera_;
    glm::dvec2 lastPos_{};
    glm::dvec2 pressWorldPos_{};
    glm::ivec2 lastScreenPos_{};
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;
    ConstraintActions& constraintActions_;

    std::vector<sketch::GeometryRef> marqueeBaseSelection_;
    std::vector<sketch::DragRequest> dragTargets_;
    std::optional<ActionReport> dragReport_;
    // Detached geometry is used only to recognize an unchanged gesture, never for rollback.
    std::optional<std::vector<sketch::SketchEntity>> dragInitialEntities_;
    bool dragAttempted_ = false;
    bool dragStopped_ = false;

    glm::dvec2 lastCursorWorldPos_{};
    glm::dvec2 copiedPos_{};

    struct ClipboardFragment {
        std::vector<sketch::SketchEntity> entities;
        std::vector<sketch::ConstraintDefinition> constraints;
        sketch::Vec2 center;
    };
    ClipboardFragment data_;
};

#endif // ! OURPAINT_APPLICATION_CURSOR_TOOL_H_
