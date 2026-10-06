#ifndef OURPAINT_APPLICATION_SKETCH_EDITOR_H_
#define OURPAINT_APPLICATION_SKETCH_EDITOR_H_

#include "../../core/DocumentManager.h"
#include "../viewport/OverlayModel.h"
#include "../viewport/picking/Cpu2dPicker.h"
#include "Camera2D.h"
#include "ConstraintActions.h"
#include "RenderScene.h"
#include "tools/ArcTool.h"
#include "tools/CircleTool.h"
#include "tools/ConstraintTool.h"
#include "tools/CubicBezierTool.h"
#include "tools/CursorTool.h"
#include "tools/DimensionTool.h"
#include "tools/IInteractionTool.h"
#include "tools/LineTool.h"
#include "tools/PointTool.h"
#include "tools/ToolId.h"

class SketchEditor {
public:
    SketchEditor(Document& document, Camera2D& camera, OverlayModel& overlay);
    ~SketchEditor();

    void select(ToolId id);
    void requestConstraint(const ConstraintRequest& request);
    void onKey(const input::KeyEvent& e);
    IInteractionTool* activeTool();

private:
    Document& document_;
    Camera2D& camera_;
    OverlayModel& overlay_;

    Cpu2dPicker picker_;
    ConstraintActions constraintActions_;

    IInteractionTool* activeTool_;

    // Tools
    CursorTool cursorTool_;
    PointTool pointTool_;
    LineTool lineTool_;
    CircleTool circleTool_;
    ArcTool arcTool_;
    CubicBezierTool bezierTool_;
    ConstraintTool constraintTool_;
    DimensionTool dimensionTool_;
};

#endif // ! OURPAINT_APPLICATION_SKETCH_EDITOR_H_
