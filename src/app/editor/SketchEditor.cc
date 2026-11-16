#include "SketchEditor.h"

#include <utility>

#include "../../core/Document.h"
#include "tool/CircleTool.h"
#include "tool/CursorTool.h"
#include "tool/LineTool.h"
#include "tool/PointTool.h"

using namespace app;

SketchEditor::SketchEditor(Document& document, render::Camera2D& camera, OverlayModel& overlay, const app::ConstraintLayout& constraintLayout,
                           app::ConstraintPresentationState& constraintPresentation)
    : document_(document),
      camera_(camera),
      overlay_(overlay),
      snap_(document_.sketch(), camera_),
      picker_(document_.sketch(), camera_),
      cursorTool_(document_, camera_, snap_, picker_, overlay_, constraintLayout, constraintPresentation),
      pointTool_(document_, camera_, snap_, overlay_, picker_),
      lineTool_(document_, camera_, snap_, overlay_, picker_),
      circleTool_(document_, camera_, snap_, picker_, overlay_),
      arcTool_(document_, camera_, snap_, picker_, overlay_),
      bezierTool_(document_, camera_, snap_, overlay_, picker_),
      constraintTool_(document_, camera_, snap_, overlay_, picker_) {
    activeTool_ = &cursorTool_;
}

SketchEditor::~SketchEditor() {}

void SketchEditor::selectTool(ToolId id, std::optional<double> value) {
    activeTool_->cancel();

    switch (id) {
        // First group
        case ToolId::Cursor:
            activeTool_ = &cursorTool_;
            break;


        // Second group
        case ToolId::Point:
            activeTool_ = &pointTool_;
            break;
        case ToolId::Line:
        case ToolId::Polyline:
        case ToolId::InfiniteLine:
            activeTool_ = &lineTool_;
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

        case ToolId::Nurbs:
            activeTool_ = &bezierTool_;
            break;


        // Third group: constraints
        default: {
            if (value.has_value()) {
                constraintTool_.setValue(value.value());
            }

            activeTool_ = &constraintTool_;
            constraintTool_.setSubToolType(id);
            break;
        }
    }

    if (activeToolId_ != id) {
        activeToolId_ = id;
        if (toolChangedCallback_) {
            toolChangedCallback_(id);
        }
    }
}

IInteractionTool* SketchEditor::activeTool() {
    return activeTool_;
}

ToolId SketchEditor::activeToolId() const noexcept { return activeToolId_; }

void SketchEditor::setToolChangedCallback(ToolChangedCallback callback) { toolChangedCallback_ = std::move(callback); }
