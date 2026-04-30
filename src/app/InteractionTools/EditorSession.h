#ifndef OURPAINT_APPLICATION_EDITORSESSION_H_
#define OURPAINT_APPLICATION_EDITORSESSION_H_

#include "../Cpu2dPicker.h"
#include "../OverlayModel.h"
#include "ArcTool.h"
#include "Camera2D.h"
#include "CircleTool.h"
#include "CubicBezierTool.h"
#include "CursorTool.h"
#include "DocumentManager.h"
#include "IInteractionTool.h"
#include "LineTool.h"
#include "PointTool.h"
#include "RenderData.h"
#include "ToolId.h"

class SnapEngine;
class EditorSession {
public:
    EditorSession(DocumentManager& manager, Camera2D& camera, renderer::RenderData& renderData, Cpu2dPicker& picker, OverlayModel& overlay, SnapEngine& snapEngine);
    ~EditorSession();

    void select(ToolId id);
    IInteractionTool* activeTool();

private:
    IInteractionTool* activeTool_ = nullptr;
    DocumentManager& documentManager_;
    Camera2D& camera_;
    renderer::RenderData& renderData_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;

    CursorTool cursorTool_;
    PointTool pointTool_;
    LineTool lineTool_;
    CircleTool circleTool_;
    ArcTool arcTool_;
    CubicBezierTool bezierTool_;
};

#endif // ! OURPAINT_APPLICATION_EDITORSESSION_H_

