#include "QtMainWindowBinder.h"

QtMainWindowBinder::QtMainWindowBinder(UI::MainWindow& window,
                                       UIController& controller,
                                       QObject* parent)
    : QObject(parent)
    , window_(window)
    , controller_(controller)  {

    // Console
    QObject::connect(&window, &UI::MainWindow::sentCommandTriggered,
                     this, [this](const QString & str) {
                         controller_.executeConsoleCommand(str.toStdString());
                     });

    // --- Open project in new window ---
    QObject::connect(&window, &UI::MainWindow::openNewWindowOpenProjectTriggered,
                     this, [this]() {
                        controller_.openProjectInNewWindow();
                     });

    // --- Create project in new window ---
    QObject::connect(&window, &UI::MainWindow::openNewWindowCreateProjectTriggered,
                     this,[this]() {
                         controller_.createProjectInNewWindow();
                     });

    // --- Open project in this window ---
    QObject::connect(&window, &UI::MainWindow::openProjectThisWindowTriggered,
                     this,[this]() {
                         controller_.openProjectInCurrentWindow();
                     });

    // --- Create project in this window ---
    QObject::connect(&window, &UI::MainWindow::createProjectThisWindowTriggered,
                     this,[this]() {
                         controller_.createProjectInCurrentWindow();
                     });

    // --- Open file ---
    QObject::connect(&window, &UI::MainWindow::openFileTriggered,
                     this,[this]() {
                         controller_.openFile();
                     });

    // --- Rename tab ---
    QObject::connect(&window, &UI::MainWindow::renameTabTriggered,
                     this,[this]() {
                         controller_.renameTab();
                     });

    // --- Remove tab ---
    QObject::connect(&window, &UI::MainWindow::removeTabTriggered,
                     this,[this]() {
                         controller_.removeTab();
                     });

    // --- Create file ---
    QObject::connect(&window, &UI::MainWindow::createFileTriggered,
                     this,[this]() {
                         controller_.createFile();
                     });

    // --- Project ---

    QObject::connect(&window, &UI::MainWindow::renameProjectTriggered,
                     this,[this]() {
                         controller_.renameProject();
                     });

    // --- Delete project ---
    QObject::connect(&window, &UI::MainWindow::deleteProjectTriggered,
                     this,[this]() {
                            controller_.deleteProject();
                         });

    // Close
    QObject::connect(&window, &UI::MainWindow::closeApplication,
                     this,[this]() {
                         controller_.closeApplication();
                     });


    // Tools - constrains
    QObject::connect(&window, &UI::MainWindow::pointLineDistanceTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintPointLineDistance);
    });
    QObject::connect(&window, &UI::MainWindow::pointOnLineTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintPointOnLine);
    });
    QObject::connect(&window, &UI::MainWindow::pointPointDistanceTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintPointPointDistance);
    });
    QObject::connect(&window, &UI::MainWindow::coincidentPointsTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintCoincidentPoints);
    });
    QObject::connect(&window, &UI::MainWindow::lineCircleDistanceTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintLineCircleDistance);
    });
    QObject::connect(&window, &UI::MainWindow::lineOnCircleTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintLineOnCircle);
    });
    QObject::connect(&window, &UI::MainWindow::lineInCircleTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintLineInCircle);
    });
    QObject::connect(&window, &UI::MainWindow::parallelLinesTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintParallelLines);
    });
    QObject::connect(&window, &UI::MainWindow::perpendicularLinesTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintPerpendicularLines);
    });
    QObject::connect(&window, &UI::MainWindow::angleBetweenLinesTriggered, this,[this]() {
        controller_.selectTool(ToolId::ConstraintAngleBetweenLines);
    });

    // Tools - point/line
    QObject::connect(&window, &UI::MainWindow::pointTriggered, this,[this]() {
        controller_.selectTool(ToolId::Point);
    });
    QObject::connect(&window, &UI::MainWindow::lineTriggered, this,[this]() {
        controller_.selectTool(ToolId::Line);
    });
    QObject::connect(&window, &UI::MainWindow::polylineTriggered, this,[this]() {
        controller_.selectTool(ToolId::Polyline);
    });
    QObject::connect(&window, &UI::MainWindow::infiniteLineTriggered, this,[this]() {
        controller_.selectTool(ToolId::InfiniteLine);
    });

    // Tools - circle
    QObject::connect(&window, &UI::MainWindow::circleByDiameterTriggered, this,[this]() {
        controller_.selectTool(ToolId::CircleByDiameter);
    });
    QObject::connect(&window, &UI::MainWindow::circleTwoPointsTriggered, this,[this]() {
        controller_.selectTool(ToolId::CircleTwoPoints);
    });
    QObject::connect(&window, &UI::MainWindow::ellipseThreePointsTriggered, this,[this]() {
        controller_.selectTool(ToolId::EllipseThreePoints);
    });

    // Tools - arc
    QObject::connect(&window, &UI::MainWindow::arcByRadiusTriggered, this,[this]() {
        controller_.selectTool(ToolId::ArcByRadius);
    });
    QObject::connect(&window, &UI::MainWindow::arcByDiameterTriggered, this,[this]() {
        controller_.selectTool(ToolId::ArcByDiameter);
    });
    QObject::connect(&window, &UI::MainWindow::arcByThreePointsTriggered, this,[this]() {
        controller_.selectTool(ToolId::ArcByThreePoints);
    });

    // Tools - cursor/size
    QObject::connect(&window, &UI::MainWindow::cursorTriggered, this,[this]() {
        controller_.selectTool(ToolId::Cursor);
    });
    QObject::connect(&window, &UI::MainWindow::sizeTriggered, this,[this]() {
        controller_.selectTool(ToolId::Size);
    });
}

