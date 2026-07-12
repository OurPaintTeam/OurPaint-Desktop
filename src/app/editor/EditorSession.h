#ifndef OURPAINT_APPLICATION_EDITORSESSION_H_
#define OURPAINT_APPLICATION_EDITORSESSION_H_

#include "../viewport/OverlayModel.h"
#include "../viewport/picking/Cpu2dPicker.h"
#include "tools/ArcTool.h"
#include "Camera2D.h"
#include "tools/DimensionTool.h"
#include "DocumentManager.h"
#include "tools/PointTool.h"
#include "RenderData.h"
#include "tools/CircleTool.h"
#include "tools/CubicBezierTool.h"
#include "tools/CursorTool.h"
#include "tools/IInteractionTool.h"
#include "tools/LineTool.h"
#include "tools/ToolId.h"

class EditorSession {
public:
    EditorSession(Document& document, Camera2D& camera, OverlayModel& overlay);
    ~EditorSession();

    void select(ToolId id, double value = 0.0);
    IInteractionTool* activeTool();

private:
    Document& document_;
    Camera2D& camera_;
    OverlayModel& overlay_;

    Cpu2dPicker picker_;


    IInteractionTool* activeTool_;

    // Tools
    CursorTool cursorTool_;
    PointTool pointTool_;
    LineTool lineTool_;
    CircleTool circleTool_;
    ArcTool arcTool_;
    CubicBezierTool bezierTool_;
    DimensionTool dimensionTool_;
};

#endif // ! OURPAINT_APPLICATION_EDITORSESSION_