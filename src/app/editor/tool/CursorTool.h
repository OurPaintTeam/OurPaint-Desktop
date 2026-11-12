#ifndef OURPAINT_APPLICATION_CURSOR_TOOL_H_
#define OURPAINT_APPLICATION_CURSOR_TOOL_H_

#include "../../../core/Document.h"
#include "../../../render/Camera2D.h"
#include "../render/constraints/ConstraintPresentationState.h"
#include "../pick/Cpu2dPicker.h"
#include "../OverlayModel.h"
#include "IInteractionTool.h"
#include "snap/SnapSystem.h"

namespace app {

using namespace core;

class CursorTool : public IInteractionTool {
public:
    explicit CursorTool(Document& document,
                        Camera2D& camera,
                        snap::SnapSystem& snap,
                        Cpu2dPicker& picker,
                        OverlayModel& overlay,
                        const ConstraintLayout& constraintLayout,
                        ConstraintPresentationState& constraintPresentation);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

private:
    void copySelection();
    void pasteSelection();
    sketch::Status prepareDragSelection(std::span<const sketch::GeometryRef> refs);
    sketch::Result<sketch::SolveDiagnostics> moveSelection(sketch::Vec2 offset);
    void finishDrag();
    std::optional<DimensionPickResult> pickDimension(glm::dvec2 position) const;
    void beginDimensionDrag(const DimensionPickResult& hit, glm::dvec2 position);
    void updateDimensionDrag(glm::dvec2 position);
    void endDimensionDrag(bool commit);

    void tryApplyPointOnPointConstraintNearCursor(double xLogic, double yLogic);

    enum class State {
        Idle,
        Pressed,
        PressedDimension,
        DraggingDimension,
        DraggingSelection,
        MarqueeSelection
    };

    State state_ = State::Idle;
    sketch::Sketch& sketch_;

    glm::dvec2 lastPos_{};
    glm::dvec2 pressWorldPos_{};
    glm::ivec2 lastScreenPos_{};
    const ConstraintLayout& constraintLayout_;
    ConstraintPresentationState& constraintPresentation_;

    struct DimensionDrag {
        sketch::ConstraintId constraint;
        glm::dvec2 pressPosition;
        glm::dvec2 grabOffsetPx;
        std::optional<app::ConstraintLabelPlacement> initialPlacement;
    };
    std::optional<DimensionDrag> dimensionDrag_;

    std::vector<sketch::GeometryRef> marqueeBaseSelection_;
    std::vector<sketch::DragRequest> dragTargets_;
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

}

#endif // ! OURPAINT_APPLICATION_CURSOR_TOOL_H_
