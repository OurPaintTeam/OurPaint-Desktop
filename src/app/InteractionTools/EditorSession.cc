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
      lineTool_(documentManager_, camera_, overlay),
      circleTool_(documentManager_, camera_, picker, overlay),
      arcTool_(documentManager_, camera_, picker, overlay),
      bezierTool_(documentManager_, camera_, renderData_) {
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


        case ToolId::CircleByRadius:
            circleTool_.setMode(CircleTool::Mode::CenterRadius);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleByDiameter:
            circleTool_.setMode(CircleTool::Mode::CenterDiameter);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleByTwoPoints:
            circleTool_.setMode(CircleTool::Mode::TwoPoints);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleByThreePoints:
            circleTool_.setMode(CircleTool::Mode::ThreePoints);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleTangentTwoLines:
            circleTool_.setMode(CircleTool::Mode::TangentTwoLines);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleTangentThreeLines:
            circleTool_.setMode(CircleTool::Mode::TangentThreeLines);
            activeTool_ = &circleTool_;
            break;


        case ToolId::ArcByRadius:
            arcTool_.setMode(ArcTool::Mode::ThreePoints);
            activeTool_ = &arcTool_;
            break;
        case ToolId::ArcByDiameter:
            arcTool_.setMode(ArcTool::Mode::ThreePoints);
            activeTool_ = &arcTool_;
            break;
        case ToolId::ArcByThreePoints:
            arcTool_.setMode(ArcTool::Mode::ThreePoints);
            activeTool_ = &arcTool_;
            break;

        case ToolId::CubicBezier:
            activeTool_ = &bezierTool_;

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


