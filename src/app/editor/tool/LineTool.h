#ifndef OURPAINT_APPLICATION_LINE_TOOL_H_
#define OURPAINT_APPLICATION_LINE_TOOL_H_

#include <optional>

#include "DocumentManager.h"
#include "Camera2D.h"
#include "OverlayModel.h"
#include "IInteractionTool.h"
#include "ToolId.h"

class LineTool : public IInteractionTool {
public:
    explicit LineTool(core::Document& document, render::Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker);

    void onMouseMove(const input::MouseMoveEvent& e) override;
    void onMouseButton(const input::MouseButtonEvent& e) override;
    void onKey(const input::KeyEvent& e) override;
    bool cancel() override;

    void setSubToolType(ToolId subTool);

private:
    enum class State {
        WaitingFirstPoint,
        WaitingSecondPoint
    };

    void beginSegmentPreview(glm::dvec2 start);
    glm::dvec2 updateCursor(glm::dvec2 cursor, input::Modifiers modifiers);
    glm::dvec2 updatePreview(input::Modifiers modifiers);

    State state_ = State::WaitingFirstPoint;
    glm::dvec2 firstPoint_;
    glm::dvec2 cursorPoint_;
    snap::SnapResult firstSnap_;

    ToolId subTool_ = ToolId::Line;
    std::optional<core::sketch::GeometryRef> previousEnd_;
};

#endif // ! OURPAINT_APPLICATION_LINE_TOOL_H_
