#include "QtMainWindowBinder.h"

#include "ToolsType.h"

QtMainWindowBinder::QtMainWindowBinder(UI::ProjectManager& window,
                                       UIController& controller,
                                       QObject* parent)
    : QObject(parent)
    , window_(window)
    , controller_(controller)  {

    // Console
    QObject::connect(&window, &UI::ProjectManager::sentCommandTriggered,
                     this, [this](const QString & str) {
                         controller_.executeConsoleCommand(str.toStdString());
                     });

    // --- Open project in new window ---
    QObject::connect(&window, &UI::ProjectManager::openNewWindowOpenProjectTriggered,
                     this, [this]() {
                        controller_.openProjectInNewWindow();
                     });

    // --- Create project in new window ---
    QObject::connect(&window, &UI::ProjectManager::openNewWindowCreateProjectTriggered,
                     this,[this]() {
                         controller_.createProjectInNewWindow();
                     });

    // --- Open project in this window ---
    QObject::connect(&window, &UI::ProjectManager::openProjectThisWindowTriggered,
                     this,[this]() {
                         controller_.openProjectInCurrentWindow();
                     });

    // --- Create project in this window ---
    QObject::connect(&window, &UI::ProjectManager::createProjectThisWindowTriggered,
                     this,[this]() {
                         controller_.createProjectInCurrentWindow();
                     });

    // --- Open file ---
    QObject::connect(&window, &UI::ProjectManager::openFileTriggered,
                     this,[this]() {
                         controller_.openFile();
                     });

    // --- Rename tab ---
    QObject::connect(&window, &UI::ProjectManager::renameTabTriggered,
                     this,[this]() {
                         controller_.renameTab();
                     });

    // --- Remove tab ---
    QObject::connect(&window, &UI::ProjectManager::removeTabTriggered,
                     this,[this]() {
                         controller_.removeTab();
                     });

    // --- Create file ---
    QObject::connect(&window, &UI::ProjectManager::createFileTriggered,
                     this,[this]() {
                         controller_.createFile();
                     });

    // --- Project ---

    QObject::connect(&window, &UI::ProjectManager::renameProjectTriggered,
                     this,[this]() {
                         controller_.renameProject();
                     });

    // --- Delete project ---
    QObject::connect(&window, &UI::ProjectManager::deleteProjectTriggered,
                     this,[this]() {
                            controller_.deleteProject();
                         });

    // Close
   /* QObject::connect(&window, &UI::ProjectManager::closeApplication,
                     this,[this]() {
                         controller_.closeApplication();
                     });*/


    // Tools - constrains
    QObject::connect(&window, &UI::ProjectManager::constraintTriggered, this,[this](UI::ConstraintType & _t1) {
        switch (_t1) {
            case UI::ConstraintType::PointLineDistance:
                controller_.selectTool(ToolId::ConstraintPointPointDistance);
                break;
            case UI::ConstraintType::PointOnLine:
                controller_.selectTool(ToolId::ConstraintPointOnLine);
                break;
            case UI::ConstraintType::PointPointDistance:
                controller_.selectTool(ToolId::ConstraintPointPointDistance);
                break;
            case UI::ConstraintType::CoincidentPoints:
                controller_.selectTool(ToolId::ConstraintCoincidentPoints);
                break;
            case UI::ConstraintType::LineCircleDistance:
                controller_.selectTool(ToolId::ConstraintLineCircleDistance);
                break;
            case UI::ConstraintType::LineOnCircle:
                controller_.selectTool(ToolId::ConstraintLineOnCircle);
                break;
            case UI::ConstraintType::LineInCircle:
                controller_.selectTool(ToolId::ConstraintLineInCircle);
                break;
            case UI::ConstraintType::ParallelLines:
                controller_.selectTool(ToolId::ConstraintParallelLines);
                break;
            case UI::ConstraintType::PerpendicularLines:
                controller_.selectTool(ToolId::ConstraintPerpendicularLines);
                break;
            case UI::ConstraintType::AngleBetweenLines:
                controller_.selectTool(ToolId::ConstraintAngleBetweenLines);
                break;
            default:
                break;
        }
    });

    // Tools - point/line
    QObject::connect(&window, &UI::ProjectManager::primitiveTriggered, this,[this](UI::PrimitiveType & _t1) {
        switch (_t1) {
            case UI::PrimitiveType::Point:
                controller_.selectTool(ToolId::Point);
                break;

            case UI::PrimitiveType::Line:
                controller_.selectTool(ToolId::Line);
                break;
            case UI::PrimitiveType::Polyline:
                controller_.selectTool(ToolId::Polyline);
                break;
            case UI::PrimitiveType::InfiniteLine:
                controller_.selectTool(ToolId::InfiniteLine);
                break;
            case UI::PrimitiveType::LineSettings:
                //controller_.selectTool(ToolId::);
                break;

            case UI::PrimitiveType::CircleByDiameter:
                controller_.selectTool(ToolId::CircleByDiameter);
                break;
            case UI::PrimitiveType::CircleTwoPoints:
                controller_.selectTool(ToolId::CircleTwoPoints);
                break;
            case UI::PrimitiveType::EllipseThreePoints:
                //controller_.selectTool(ToolId::);
                break;
            case UI::PrimitiveType::CircleSettings:
                //controller_.selectTool(ToolId::);
                break;

            case UI::PrimitiveType::ArcByRadius:
                //controller_.selectTool(ToolId::);
                break;
            case UI::PrimitiveType::ArcByDiameter:
                //controller_.selectTool(ToolId::);
                break;
            case UI::PrimitiveType::ArcThreePoints:
                //controller_.selectTool(ToolId::);
                break;
            case UI::PrimitiveType::ArcSettings:
                //controller_.selectTool(ToolId::);
                break;
            default: break;
        }
    });

    // Tools - cursor/size
    QObject::connect(&window, &UI::ProjectManager::toolsTriggered, this,[this](UI::ToolsType & _t1) {
        if (_t1 == UI::ToolsType::Cursor) {
            controller_.selectTool(ToolId::Cursor);
        }
    });
}

