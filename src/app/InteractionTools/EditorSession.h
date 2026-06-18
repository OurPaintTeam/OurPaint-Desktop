#ifndef OURPAINT_APPLICATION_EDITORSESSION_H_
#define OURPAINT_APPLICATION_EDITORSESSION_H_

#include "DocumentManager.h"
#include "IInteractionTool.h"
#include "ToolId.h"
#include "Camera2D.h"
#include "RenderScene.h"
#include "../Cpu2dPicker.h"
#include "../OverlayModel.h"

#include "LineTool.h"
#include "PointTool.h"
#include "CircleTool.h"
#include "ArcTool.h"
#include "CursorTool.h"
#include "CubicBezierTool.h"
#include "DimensionTool.h"

class EditorSession {
public:
    EditorSession(DocumentManager& manager, Camera2D& camera, render::RenderScene& renderScene, Cpu2dPicker& picker, OverlayModel& overlay);
    ~EditorSession();

    void select(ToolId id, double value = 0.0);
    IInteractionTool* activeTool();

private:
    IInteractionTool* activeTool_ = nullptr;
    DocumentManager& documentManager_;
    Camera2D& camera_;
    render::RenderScene& renderScene_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;

    CursorTool cursorTool_;
    PointTool pointTool_;
    LineTool lineTool_;
    CircleTool circleTool_;
    ArcTool arcTool_;
    CubicBezierTool bezierTool_;
    DimensionTool dimensionTool_;
};

#endif // ! OURPAINT_APPLICATION_EDITORSESSION_H_

