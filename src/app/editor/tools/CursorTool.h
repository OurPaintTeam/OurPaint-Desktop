#ifndef OURPAINT_APPLICATION_CURSOR_TOOL_H_
#define OURPAINT_APPLICATION_CURSOR_TOOL_H_

#include "../../../core/Document.h"
#include "../../viewport/OverlayModel.h"
#include "../../viewport/constraints/ConstraintPresentationState.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "Camera2D.h"
#include "IInteractionTool.h"
#include "snapping/SnapSystem.h"

using namespace core;

class ConstraintActions;

class CursorTool : public IInteractionTool {
public:
    explicit CursorTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, Cpu2dPicker& picker, OverlayModel& overlay,
                        ConstraintActions& constraintActions, const app::ConstraintLayout& constraintLayout,
                        app::ConstraintPresentationState& constraintPresentation);

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
    std::optional<DimensionPickResult> pickDimension(glm::dvec2 position) const;
    void beginDimensionDrag(const DimensionPickResult& hit, glm::dvec2 position);
    void updateDimensionDrag(glm::dvec2 position);
    void endDimensionDrag(bool commit);

    ActionReport tryApplyPointOnPointNearCursor(double xLogic, double yLogic);

    enum class State { Idle, Pressed, PressedDimension, DraggingDimension, DraggingSelection, MarqueeSelection };

    State state_ = State::Idle;
    sketch::Sketch& sketch_;

    glm::dvec2 lastPos_{};
    glm::dvec2 pressWorldPos_{};
    glm::ivec2 lastScreenPos_{};
    ConstraintActions& constraintActions_;
    const app::ConstraintLayout& constraintLayout_;
    app::ConstraintPresentationState& constraintPresentation_;

    struct DimensionDrag {
        sketch::ConstraintId constraint;
        glm::dvec2 pressPosition;
        glm::dvec2 grabOffsetPx;
        std::optional<app::ConstraintLabelPlacement> initialPlacement;
    };
    std::optional<DimensionDrag> dimensionDrag_;

    std::vector<sketch::GeometryRef> marqueeBaseSelection_;
    std::vector<sketch::DragRequest> dragTargets_;
    std::optional<ActionReport> dragReport_;
    // Detached values recognize an unchanged gesture; they are never used for rollback.
    std::optional<std::vector<sketch::SketchEntity>> dragInitialEntities_;
    std::optional<std::vector<sketch::SketchConstraint>> dragInitialConstraints_;
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
