#include "SketchEditor.h"

#include "../../core/Document.h"
#include "tools/CircleTool.h"
#include "tools/CursorTool.h"
#include "tools/LineTool.h"
#include "tools/PointTool.h"

SketchEditor::SketchEditor(Document& document, Camera2D& camera, OverlayModel& overlay)
    : document_(document),
      camera_(camera),
      overlay_(overlay),
      picker_(document_.sketch(), camera_),
      constraintActions_(document_.sketch()),
      cursorTool_(document_, camera_, picker_, overlay_, constraintActions_),
      pointTool_(document_, camera_),
      lineTool_(document_, camera_, overlay_),
      circleTool_(document_, camera_, picker_, overlay_),
      arcTool_(document_, camera_, picker_, overlay_),
      bezierTool_(document_, camera_),
      constraintTool_(constraintActions_, picker_, overlay_),
      dimensionTool_(constraintActions_, picker_, overlay_) {
    activeTool_ = &cursorTool_;
}

SketchEditor::~SketchEditor() {}

void SketchEditor::select(ToolId id) {
    cancelActiveTool();
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
            break;
    }
}

void SketchEditor::requestConstraint(const ConstraintRequest& request) {
    const auto refs = overlay_.selection_.model.items();
    const auto prepared = constraintActions_.prepare(request, refs);
    switch (prepared.state) {
        case ConstraintPreparation::State::Ready:
            if (prepared.definition) {
                if (activeTool_ == &cursorTool_) {
                    cancelActiveTool();
                }
                deliver(constraintActions_.apply(*prepared.definition));
            }
            return;
        case ConstraintPreparation::State::NeedsMoreInput:
            cancelActiveTool();
            if (request.action == ConstraintAction::Dimension || request.action == ConstraintAction::Angle) {
                dimensionTool_.begin(request, refs);
                activeTool_ = &dimensionTool_;
            } else {
                constraintTool_.begin(request, refs);
                activeTool_ = &constraintTool_;
            }
            return;
        case ConstraintPreparation::State::InvalidSelection:
        case ConstraintPreparation::State::Unsupported:
            deliver(ConstraintActions::rejectedReport(prepared));
            return;
    }
}

void SketchEditor::onKey(const input::KeyEvent& e) {
    if (e.action == input::KeyAction::Press && e.key == input::KeyCode::Escape) {
        if (e.isAutoRepeat) {
            return;
        }
        if (!cancelActiveTool().handled) {
            select(ToolId::Cursor);
        }
        return;
    }

    if (e.action == input::KeyAction::Press && e.modifiers == input::Modifiers::None && !e.isAutoRepeat) {
        std::optional<ConstraintAction> action;
        switch (e.key) {
            case input::KeyCode::Num4:
                action = ConstraintAction::Coincident;
                break;
            case input::KeyCode::Num5:
                action = ConstraintAction::Horizontal;
                break;
            case input::KeyCode::Num6:
                action = ConstraintAction::Vertical;
                break;
            case input::KeyCode::Num8:
                action = ConstraintAction::Parallel;
                break;
            case input::KeyCode::Num9:
                action = ConstraintAction::Perpendicular;
                break;
            default:
                break;
        }
        if (action) {
            requestConstraint({*action, std::nullopt});
            return;
        }
    }

    if (activeTool_ == &cursorTool_ && e.action == input::KeyAction::Press && !e.isAutoRepeat &&
        (e.key == input::KeyCode::Delete || (e.key == input::KeyCode::V && e.modifiers == input::Modifiers::Ctrl))) {
        cancelActiveTool();
    }
    deliver(activeTool_->onKey(e));
}

IInteractionTool* SketchEditor::activeTool() { return activeTool_; }

void SketchEditor::onMouseMove(const input::MouseMoveEvent& e) {
    deliver(activeTool_->onMouseMove(e));
}

void SketchEditor::onMouseButton(const input::MouseButtonEvent& e) {
    deliver(activeTool_->onMouseButton(e));
}

void SketchEditor::setReportCallback(ReportCallback callback) {
    reportCallback_ = std::move(callback);
}

void SketchEditor::deliver(const std::optional<ActionReport>& report) {
    if (report && reportCallback_) {
        reportCallback_(*report);
    }
}

ToolCancellation SketchEditor::cancelActiveTool() {
    auto cancelled = activeTool_->cancel();
    deliver(cancelled.report);
    return cancelled;
}

void SketchEditor::endInteraction() {
    if (activeTool_ == &cursorTool_) {
        cancelActiveTool();
    }
}

void SketchEditor::switchSolverBackend(core::sketch::BackendKind backend) {
    if (std::as_const(document_).sketch().backendKind() == backend) {
        return;
    }
    // End a gesture before migrating its model, retaining any edits made so far.
    cancelActiveTool();
    ActionReport report{ActionKind::SwitchBackend};
    report.backend = backend;
    const auto switched = document_.sketch().switchBackend(backend);
    if (switched) {
        report.change = ModelChange::Changed;
    } else {
        // Sketch stages the replacement and preserves the old backend on error.
        report.operationError = switched.error();
    }
    deliver(report);
}
