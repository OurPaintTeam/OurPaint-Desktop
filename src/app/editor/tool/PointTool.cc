#include "PointTool.h"

#include <iostream>

#include "../../../core/Document.h"
#include "objects/Objects.h"

PointTool::PointTool(core::Document& document, render::Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker) :
IInteractionTool(document,camera,snap,overlay, picker) {}

void PointTool::onMouseMove(const input::MouseMoveEvent& e) {
    // SNAP
    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
    snap::SnapRequest request{core::sketch::Vec2{v.x, v.y}, snap::ActionType::Draw, snap::PrimitiveType::Point, snap::DrawState::Start};
    snapVisual(request);
}

void PointTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button != input::MouseButton::Left || e.action != input::MouseButtonAction::Press) {
        return;
    }

    glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});

    if (!snap_results_.empty()) {
        v.x = snap_results_[0].point.position.x;
        v.y = snap_results_[0].point.position.y;
    }

    auto addResult = document_.sketch().addPoint({v.x, v.y});
    if (!addResult) {
        std::cout << "addPoint failed\n";
        return;
    }
    const auto newPointId = addResult.value();

    const core::sketch::GeometryRef pointRef{newPointId,
                                             core::sketch::SubElement::Whole};

    bool anyAdded = false;

    for (const auto& snap_result : snap_results_) {
        if (applySnapConstraint(snap_result, pointRef)) {
            anyAdded = true;
        }
    }

    if (anyAdded) {
        auto solved = document_.sketch().solve();
        if (!solved ||
            solved.value().solveStatus != core::sketch::SolveStatus::Converged) {
            std::cout << "solve err in PointTool\n";
            }
    }

    document_.sketch().pushState(); // FOR UNDO/REDO
}

void PointTool::onKey(const input::KeyEvent& e) { (void)e; }

bool PointTool::cancel() { return false; }
