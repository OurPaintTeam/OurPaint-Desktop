#include "UIObserver.h"

#include "Lib/Core/ProjectManager.h"
#include "Lib/Core/ToolsType.h"

void UIObserver::presentActiveTool(UI::ProjectManager& window, const std::string& name, ToolId tool, bool active) {
    const QString tabName = QString::fromStdString(name);
    const auto present = [&](auto uiTool) {
        window.setActiveToolTabWindow(tabName, uiTool);
        if (active) {
            window.setActiveToolProjectWindow(uiTool);
        }
    };
    switch (tool) {
        case ToolId::Cursor:
            present(UI::ToolsType::Cursor);
            break;
        case ToolId::Point:
            present(UI::PrimitiveType::Point);
            break;
        case ToolId::Line:
            present(UI::PrimitiveType::Line);
            break;
        case ToolId::Polyline:
            present(UI::PrimitiveType::Polyline);
            break;
        case ToolId::InfiniteLine:
            present(UI::PrimitiveType::InfiniteLine);
            break;
        case ToolId::CircleByRadius:
            present(UI::PrimitiveType::CircleByRadius);
            break;
        case ToolId::CircleByDiameter:
            present(UI::PrimitiveType::CircleByDiameter);
            break;
        case ToolId::CircleByTwoPoints:
            present(UI::PrimitiveType::CircleByTwoPoints);
            break;
        case ToolId::CircleByThreePoints:
            present(UI::PrimitiveType::CircleByThreePoints);
            break;
        case ToolId::CircleTangentTwoLines:
            present(UI::PrimitiveType::CircleTangentTwoLines);
            break;
        case ToolId::CircleTangentThreeLines:
            present(UI::PrimitiveType::CircleTangentThreeLines);
            break;
        case ToolId::ArcByRadius:
            present(UI::PrimitiveType::ArcByRadius);
            break;
        case ToolId::ArcByDiameter:
            present(UI::PrimitiveType::ArcByDiameter);
            break;
        case ToolId::ArcByThreePoints:
            present(UI::PrimitiveType::ArcThreePoints);
            break;
        case ToolId::Nurbs:
            present(UI::PrimitiveType::CubicBezier);
            break;
        case ToolId::Distance:
            present(UI::ConstraintType::Distance);
            break;
        case ToolId::HorizontalDistance:
            present(UI::ConstraintType::HDistance);
            break;
        case ToolId::VerticalDistance:
            present(UI::ConstraintType::VDistance);
            break;
        case ToolId::Angle:
            present(UI::ConstraintType::Angle);
            break;
        case ToolId::Coincident:
            present(UI::ConstraintType::Coincident);
            break;
        case ToolId::Horizontal:
            present(UI::ConstraintType::Horizontal);
            break;
        case ToolId::Vertical:
            present(UI::ConstraintType::Vertical);
            break;
        case ToolId::Parallel:
            present(UI::ConstraintType::Parallel);
            break;
        case ToolId::Perpendicular:
            present(UI::ConstraintType::Perpendicular);
            break;
        case ToolId::Tangent:
            present(UI::ConstraintType::Tangent);
            break;
        case ToolId::Equal:
            present(UI::ConstraintType::Equal);
            break;
        case ToolId::FixUnfix:
            present(UI::ConstraintType::FixUnfix);
            break;
        case ToolId::Concentric:
            present(UI::ConstraintType::Concentric);
            break;
        case ToolId::Midpoint:
            present(UI::ConstraintType::Midpoint);
            break;
        case ToolId::Collinear:
            present(UI::ConstraintType::Collinear);
            break;
        case ToolId::Curvature:
            present(UI::ConstraintType::Curvature);
            break;
        case ToolId::Symmetric:
            present(UI::ConstraintType::Symmetric);
            break;
    }
}
