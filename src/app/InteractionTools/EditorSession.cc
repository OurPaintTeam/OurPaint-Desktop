#include "EditorSession.h"

#include "LineTool.h"
#include "PointTool.h"
#include "CircleTool.h"
#include "CursorTool.h"

EditorSession::EditorSession(DocumentManager& manager,
                            Camera2D& camera,
                            renderer::RenderData& renderData,
                            Cpu2dPicker& picker,
                            OverlayModel& overlay)
    : documentManager_(manager),
      camera_(camera),
      renderData_(renderData),
      picker_(picker),
      overlay_(overlay),
      cursorTool_(documentManager_, camera_, picker_, overlay_),
      pointTool_(documentManager_, camera_),
      lineTool_(documentManager_, camera_, renderData_),
      circleTool_(documentManager_, camera_, renderData_) {
    activeTool_ = new PointTool(documentManager_, camera_);
}

EditorSession::~EditorSession() {
    delete activeTool_;
}

void EditorSession::select(ToolId id) {
    activeTool_->cancel();
    switch (id) {
        case ToolId::Cursor:
            activeTool_ = &cursorTool_;
            break;
        case ToolId::Size:

            break;
        case ToolId::Point:
            activeTool_ = &pointTool_;
            break;
        case ToolId::Line:
            activeTool_ = &lineTool_;
            break;
        case ToolId::Polyline:

            break;
        case ToolId::InfiniteLine:

            break;

        case ToolId::CircleByDiameter:
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleTwoPoints:
            activeTool_ = &circleTool_;
            break;
        case ToolId::EllipseThreePoints:
            activeTool_ = &circleTool_;
            break;

        case ToolId::ArcByRadius:

            break;
        case ToolId::ArcByDiameter:

            break;
        case ToolId::ArcByThreePoints:

            break;

        case ToolId::ConstraintPointLineDistance:

            break;
        case ToolId::ConstraintPointOnLine:

            break;
        case ToolId::ConstraintPointPointDistance:

            break;
        case ToolId::ConstraintCoincidentPoints:

            break;
        case ToolId::ConstraintLineCircleDistance:

            break;
        case ToolId::ConstraintLineOnCircle:

            break;
        case ToolId::ConstraintLineInCircle:

            break;
        case ToolId::ConstraintParallelLines:

            break;
        case ToolId::ConstraintPerpendicularLines:

            break;
        case ToolId::ConstraintAngleBetweenLines:

            break;
    }
}

IInteractionTool* EditorSession::activeTool() {
    return activeTool_;
}


