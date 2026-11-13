#ifndef OURPAINT_APPLICATION_SKETCH_EDITOR_H_
#define OURPAINT_APPLICATION_SKETCH_EDITOR_H_

#include <functional>
#include <optional>

#include "../../core/DocumentManager.h"
#include "../../render/Camera2D.h"
#include "../../render/RenderScene.h"
#include "pick/Cpu2dPicker.h"
#include "OverlayModel.h"
#include "tool/ArcTool.h"
#include "tool/CircleTool.h"
#include "tool/ConstraintTool.h"
#include "tool/CursorTool.h"
#include "tool/IInteractionTool.h"
#include "tool/LineTool.h"
#include "tool/NurbsTool.h"
#include "tool/PointTool.h"
#include "tool/ToolId.h"

namespace app {

class SketchEditor {
public:
    using ToolChangedCallback = std::function<void(ToolId)>;

    SketchEditor(Document& document, render::Camera2D& camera, OverlayModel& overlay, const app::ConstraintLayout& constraintLayout,
                 app::ConstraintPresentationState& constraintPresentation);
    ~SketchEditor();

    void selectTool(ToolId id, std::optional<double> value = std::nullopt);

    IInteractionTool* activeTool();
    ToolId activeToolId() const noexcept;
    // Called synchronously after a tool or mode change; does not replay the current state.
    void setToolChangedCallback(ToolChangedCallback callback);

private:
    Document& document_;
    render::Camera2D& camera_;
    OverlayModel& overlay_;
    snap::SnapSystem snap_;

    Cpu2dPicker picker_;

    IInteractionTool* activeTool_;
    ToolId activeToolId_ = ToolId::Cursor;
    ToolChangedCallback toolChangedCallback_;

    // Tools
    CursorTool cursorTool_;

    PointTool pointTool_;
    LineTool lineTool_;
    CircleTool circleTool_;
    ArcTool arcTool_;
    NurbsTool bezierTool_;

    ConstraintTool constraintTool_;
};

}

#endif // ! OURPAINT_APPLICATION_SKETCH_EDITOR_H_
