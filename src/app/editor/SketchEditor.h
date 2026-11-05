#ifndef OURPAINT_APPLICATION_SKETCH_EDITOR_H_
#define OURPAINT_APPLICATION_SKETCH_EDITOR_H_

#include <functional>
#include <optional>

#include "../../core/DocumentManager.h"
#include "../viewport/OverlayModel.h"
#include "../viewport/picking/Cpu2dPicker.h"
#include "Camera2D.h"
#include "RenderScene.h"
#include "tools/ArcTool.h"
#include "tools/CircleTool.h"
#include "tools/ConstraintTool.h"
#include "tools/CubicBezierTool.h"
#include "tools/CursorTool.h"
#include "tools/IInteractionTool.h"
#include "tools/LineTool.h"
#include "tools/PointTool.h"
#include "tools/ToolId.h"

namespace app {

class SketchEditor {
public:
    using ToolChangedCallback = std::function<void(ToolId)>;

    SketchEditor(Document& document, Camera2D& camera, OverlayModel& overlay, const app::ConstraintLayout& constraintLayout,
                 app::ConstraintPresentationState& constraintPresentation);
    ~SketchEditor();

    void selectTool(ToolId id, std::optional<double> value = std::nullopt);

    IInteractionTool* activeTool();
    ToolId activeToolId() const noexcept;
    // Called synchronously after a tool or mode change; does not replay the current state.
    void setToolChangedCallback(ToolChangedCallback callback);

private:
    Document& document_;
    Camera2D& camera_;
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
    CubicBezierTool bezierTool_;

    ConstraintTool constraintTool_;
};

}

#endif // ! OURPAINT_APPLICATION_SKETCH_EDITOR_H_
