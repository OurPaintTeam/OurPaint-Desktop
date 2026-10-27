#include "PointTool.h"

#include <iostream>

#include "../../../core/Document.h"
#include "objects/Objects.h"

PointTool::PointTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker)
    : IInteractionTool(document, camera, snap, overlay, picker) {}

std::optional<ActionReport> PointTool::onMouseMove(const input::MouseMoveEvent& e) {
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    snap::SnapRequest request{core::sketch::Vec2{v.x, v.y}, snap::ActionType::Point, snap::DrawState::Start};
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

            using SK = snap::SnapKind;
            using CT = core::sketch::ConstraintType;
            using SE = core::sketch::SubElement;
            using Ref = core::sketch::GeometryRef;
            using ConstraintDef = core::sketch::ConstraintDefinition;

            const Ref pointRef{newPointId, SE::Whole};

            std::vector<ConstraintDef> appliedConstraints;
            bool needsSolve = false;
            bool constraintAttempted = false;

            auto addConstraint = [&](CT type, std::vector<Ref> refs) {
                ConstraintDef def;
                def.type = type;
                def.refs = std::move(refs);

                for (const auto& existing : appliedConstraints) {
                    if (existing.type != def.type || existing.refs.size() != def.refs.size()) {
                        continue;
                    }

                    bool sameRefs = true;

                    for (std::size_t i = 0; i < def.refs.size(); ++i) {
                        if (existing.refs[i].entity != def.refs[i].entity || existing.refs[i].sub != def.refs[i].sub) {
                            sameRefs = false;
                            break;
                        }
                    }

                    if (sameRefs) {
                        return;
                    }
                }

                constraintAttempted = true;

                if (!document_.sketch().supportsConstraint(def)) {
                    report.rejection = ActionRejection::Unsupported;
                    return;
                }

                auto c = document_.sketch().addConstraint(def);

                if (c) {
                    appliedConstraints.push_back(def);
                    report.constraintIds.push_back(c.value());
                    report.requestedConstraints++;
                    needsSolve = true;
                } else {
                    report.operationError = c.error();
                }
            };

            auto applySnapKind = [&](SK type, const std::vector<Ref>& objects) {
                switch (type) {
                    case SK::Coincident:
                    case SK::Intersection:
                    case SK::ExtendedIntersection:
                    case SK::VirtualIntersection:
                        for (const auto& object : objects) {
                            addConstraint(CT::Coincident, {pointRef, object});
                        }
                        break;

                    case SK::Midpoint:
                        for (const auto& object : objects) {
                            addConstraint(CT::Midpoint, {pointRef, object});
                        }
                        break;

                    default:

                        break;
                }
            };

            applySnapKind(snap_result_.type, snap_result_.objects);

            for (const auto& relation : snap_result_.relations) {
                applySnapKind(relation.type, relation.objects);
            }

            if (needsSolve) {
                auto solved = document_.sketch().solve();
                report.recordSolve(solved);

                if (!solved || solved.value().status != core::sketch::SolveStatus::Converged) {
                    std::cout << "solve err" << std::endl;
                }
            }

            return report;
        }

        return ActionReport::creation(ActionKind::CreatePoint, document_.sketch().addPoint({v.x, v.y}));
    }

    return std::nullopt;
}

std::optional<ActionReport> PointTool::onKey(const input::KeyEvent& e) { return std::nullopt; }

ToolCancellation PointTool::cancel() { return {}; }
