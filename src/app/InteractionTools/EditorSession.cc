#include "EditorSession.h"

#include "LineTool.h"
#include "PointTool.h"
#include "CircleTool.h"
#include "CursorTool.h"

EditorSession::EditorSession(DocumentManager& manager, Camera2D& camera, renderer::RenderData& renderData)
    : documentManager_(manager), camera_(camera), renderData_(renderData) {
    activeTool_ = new PointTool(documentManager_, camera_);
}

EditorSession::~EditorSession() {
    delete activeTool_;
}

void EditorSession::select(ToolId id) {
    switch (id) {
        case ToolId::Cursor:
            delete activeTool_;
            activeTool_ = new CursorTool(documentManager_, camera_, renderData_);
            break;
        case ToolId::Size:

            break;
        case ToolId::Point:
            delete activeTool_;
            activeTool_ = new PointTool(documentManager_, camera_);
            break;
        case ToolId::Line:
            delete activeTool_;
            activeTool_ = new LineTool(documentManager_, camera_, renderData_);
            break;
        case ToolId::Polyline:

            break;
        case ToolId::InfiniteLine:

            break;

        case ToolId::CircleByDiameter:
            delete activeTool_;
            activeTool_ = new CircleTool(documentManager_, camera_, renderData_);
            break;
        case ToolId::CircleTwoPoints:
            delete activeTool_;
            activeTool_ = new CircleTool(documentManager_, camera_, renderData_);
            break;
        case ToolId::EllipseThreePoints:
            delete activeTool_;
            activeTool_ = new CircleTool(documentManager_, camera_, renderData_);
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


