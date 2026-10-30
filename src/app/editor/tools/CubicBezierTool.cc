#include "CubicBezierTool.h"

#include <iostream>

#include "../../../core/Document.h"
#include "../../../core/DocumentManager.h"
#include "Camera2D.h"
#include "objects/Objects.h"

CubicBezierTool::CubicBezierTool(Document& document, Camera2D& camera, snap::SnapSystem& snap, OverlayModel& overlay, Cpu2dPicker& picker)
    : IInteractionTool(document, camera, snap, overlay, picker) {}

std::optional<ActionReport> CubicBezierTool::onMouseMove(const input::MouseMoveEvent& e) { return std::nullopt; }

std::optional<ActionReport> CubicBezierTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button == input::MouseButton::Left && e.action == input::MouseButtonAction::Press) {
        glm::dvec2 v = camera_.screenLogicalToWorld({e.x, e.y});
        points_.push_back(v);
        overlay_.points_.push_back(OverlayModel::Point(v.x, v.y));
    }
    return std::nullopt;
}

std::optional<ActionReport> CubicBezierTool::onKey(const input::KeyEvent& e) {
    if (e.action != input::KeyAction::Press || e.key != input::KeyCode::Enter || e.isAutoRepeat) {
        return std::nullopt;
    }
    if (points_.size() < 4) {
        std::cerr << "A cubic NURBS curve needs at least four control points.\n";
        return std::nullopt;
    }

    core::sketch::NurbsCurve2 nc;
    nc.degree = 3;
    for (const auto& p : points_) {
        nc.controlPoints.push_back({{p.x, p.y}});
    }
    nc.weights.assign(points_.size(), 1.0);

    // Open uniform knots with degree + 1 copies at each end.
    nc.knots.assign(nc.degree + 1, 0.0);
    const size_t spans = points_.size() - nc.degree;
    for (size_t i = 1; i < spans; ++i) {
        nc.knots.push_back(static_cast<double>(i) / static_cast<double>(spans));
    }
    nc.knots.insert(nc.knots.end(), nc.degree + 1, 1.0);

    const auto added = document_.sketch().addEntity(nc);
    if (!added) {
        std::cerr << "Cannot add NURBS curve: " << added.error().message << '\n';
        return std::nullopt;
    }
    points_.clear();
    return std::nullopt;
}

ToolCancellation CubicBezierTool::cancel() {
    const bool handled = !points_.empty();
    points_.clear();
    overlay_.clear();
    return {handled, std::nullopt};
}
