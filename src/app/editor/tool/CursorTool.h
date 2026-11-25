#ifndef OURPAINT_APPLICATION_CURSOR_TOOL_H_
#define OURPAINT_APPLICATION_CURSOR_TOOL_H_

#include "../../../core/Document.h"
#include "../../../render/Camera2D.h"
#include "../OverlayModel.h"
#include "../pick/Cpu2dPicker.h"
#include "../render/constraints/ConstraintPresentationState.h"
#include "IInteractionTool.h"
#include "geometry/GeometryTypes.h"
#include "snap/SnapSystem.h"

namespace app {

using namespace core;

class CursorTool : public IInteractionTool {
public:
    explicit CursorTool(Document& document,
                        render::Camera2D& camera,
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
    void updateMarqueeSelection(glm::dvec2 position, input::Modifiers modifiers);
    void deleteSelection();
    void copySelection();
    void pasteSelection();
    sketch::Status prepareDragSelection(std::span<const sketch::GeometryRef> refs);
    sketch::Result<sketch::SolveDiagnostics> moveSelection(sketch::Vec2 offset);
    void finishDrag();
    void beginDimensionDrag(const DimensionPickResult& hit, glm::dvec2 position);
    void updateDimensionDrag(glm::dvec2 position);
    void endDimensionDrag(bool commit);

    std::optional<std::pair<sketch::GeometryRef, sketch::GeometryRef>> findTwoPointsNearCursor(double xLogic, double yLogic);
    void applyPointOnPointConstraint(sketch::GeometryRef p1, sketch::GeometryRef p2);
    void previewTwoPoints(sketch::GeometryRef p1, sketch::GeometryRef p2);

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

    glm::dvec2 lastLogicalPos_{};

    glm::dvec2 lastPos_{};
    glm::dvec2 pressWorldPos_{};
    glm::dvec2 marqueeStartLogical_{};
    const ConstraintLayout& constraintLayout_;
    ConstraintPresentationState& constraintPresentation_;

    struct DimensionDrag {
        sketch::ConstraintId constraint;
        glm::dvec2 pressPosition;
        glm::dvec2 grabOffsetPx;
        std::optional<app::ConstraintLabelPlacement> initialPlacement;
    };
    std::optional<DimensionDrag> dimensionDrag_;

    std::vector<SelectionItem> marqueeBaseSelection_;
    std::vector<sketch::GeometryRef> dragGeometry_;
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
