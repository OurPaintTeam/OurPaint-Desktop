#include "EditorSession.h"

#include "Document.h"
#include "tools/CircleTool.h"
#include "tools/CursorTool.h"
#include "tools/LineTool.h"
#include "tools/PointTool.h"

EditorSession::EditorSession(Document& document, Camera2D& camera, OverlayModel& overlay)
    : document_(document),
      camera_(camera),
      overlay_(overlay),
      picker_(document_.scene(), camera_),
      cursorTool_(document_, camera_, picker_, overlay_),
      pointTool_(document_, camera_),
      lineTool_(document_, camera_, overlay_),
      circleTool_(document_, camera_, picker_, overlay_),
      arcTool_(document_, camera_, picker_, overlay_),
      bezierTool_(document_, camera_),
      dimensionTool_(document_, camera_, picker_, overlay_) {
    activeTool_ = &cursorTool_;
}

EditorSession::~EditorSession() {}

void EditorSession::select(ToolId id, double value) {
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


        case ToolId::ConstraintDimension:
            dimensionTool_.setDimension(value);
            activeTool_ = &dimensionTool_;
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
            circleTool_.setMode(CircleTool::Mode::DiameterTwoPoints);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleByThreePoints:
            circleTool_.setMode(CircleTool::Mode::ThreePoints);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleTangentTwoLines:
            circleTool_.setMode(CircleTool::Mode::TangentTwoObjectsRadius);
            activeTool_ = &circleTool_;
            break;
        case ToolId::CircleTangentThreeLines:
            circleTool_.setMode(CircleTool::Mode::TangentThreeObjects);
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


