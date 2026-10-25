#include "PointTool.h"

#include <iostream>

#include "../../../core/Document.h"
#include "objects/Objects.h"

PointTool::PointTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker) :
IInteractionTool(document,camera,snap,overlay, picker) {}

std::optional<ActionReport> PointTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    snap::SnapRequest request{ core::sketch::Vec2{v.x, v.y}, snap::ActionType::Point, snap::DrawState::Start};
    snapVisual(request);
    return std::nullopt;
}


std::optional<ActionReport> PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});

        if (snap_result_.snapped) {
            v.x = snap_result_.point.position.x;
            v.y = snap_result_.point.position.y;

            core::sketch::Result<core::sketch::EntityId> addResult = document_.sketch().addPoint({v.x, v.y});
            if (!addResult) {
                return ActionReport::creation(ActionKind::CreatePoint, addResult);
            }
            const auto newPointId = addResult.value();

            ActionReport report = ActionReport::creation(ActionKind::CreatePoint, addResult);

            core::sketch::ConstraintDefinition def;
            bool shouldAddConstraint = false;

            switch(snap_result_.type) {
                case snap::SnapKind::Coincident:
                case snap::SnapKind::Intersection:
                case snap::SnapKind::ExtendedIntersection:
                    def.type = core::sketch::ConstraintType::Coincident;
                    def.refs = {
                        { newPointId, core::sketch::SubElement::Whole },
                        snap_result_.objects[0]
                    };
                    shouldAddConstraint = true;
                    break;


                case snap::SnapKind::Midpoint :
                    def.type = core::sketch::ConstraintType::Midpoint;
                    def.refs = {
                        { newPointId, core::sketch::SubElement::Whole },
                        snap_result_.objects[0]
                    };
                    shouldAddConstraint = true;
                    break;



                default:
                    // Оставляем shouldAddConstraint = false
                    break;
            }

            if (shouldAddConstraint && document_.sketch().supportsConstraint(def)) {
                auto c = document_.sketch().addConstraint(def);
                if (c) {
                    report.constraintIds.push_back(c.value());
                    report.requestedConstraints++;
                } else {
                    report.operationError = c.error();
                }

                auto solved = document_.sketch().solve();
                report.recordSolve(solved);

                if (!solved || solved.value().status != core::sketch::SolveStatus::Converged) {
                    std::cout << "solve err" << std::endl;
                }
            } else if (shouldAddConstraint) {
                report.rejection = ActionRejection::Unsupported;
            }

            return report;
        }

        //  если snap_result_.snapped == false
        return ActionReport::creation(ActionKind::CreatePoint, document_.sketch().addPoint({v.x, v.y}));
    }
    return std::nullopt;
}

std::optional<ActionReport> PointTool::onKey(const input::KeyEvent& e) {
    return std::nullopt;
}

ToolCancellation PointTool::cancel() {
    return {};
}


