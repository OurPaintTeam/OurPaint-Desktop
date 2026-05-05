#include "QtMainWindowBinder.h"

#include <QDir>
#include <QFile>

#include "InputWidget.h"
#include "PainterWidget.h"
#include "ParameterInputWidget.h"
#include "ToolsType.h"

QtMainWindowBinder::QtMainWindowBinder(UI::ProjectManager& window, UIController& controller, QObject* parent)
    : QObject(parent), window_(window), controller_(controller) {
    // Console
    QObject::connect(&window, &UI::ProjectManager::sentCommandTriggered, this, [this](const QString& tabName, const QString& str) {
        try {
            controller_.executeConsoleCommand(str.toStdString());
        } catch (const std::exception& e) {
            QString error = "Error: " + QString::fromStdString(e.what());
            window_.addNotification(error);
        }
    });

    // Scripts
    QObject::connect(&window, &UI::ProjectManager::scriptTriggered, this, [this](const QString& path) {
        qDebug() << "Script start:" << path;

        QFile file(path);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "OPEN FAILED:" << file.errorString();
            window_.addNotification("Error: cannot open file");
            return;
        }

        qDebug() << "File opened OK";

        QTextStream in(&file);

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();

            if (line.isEmpty()) {
                continue;
            }

            try {
                controller_.executeConsoleCommand(line.toStdString());
            } catch (const std::exception& e) {
                window_.addNotification("Error: " + QString::fromStdString(e.what()));
            }
        }

        file.close();
    });

    // --- Open project in new window ---
    QObject::connect(&window, &UI::ProjectManager::openNewWindowOpenProjectTriggered, this, [this]() { controller_.openProjectInNewWindow(); });

    // --- Create project in new window ---
    QObject::connect(&window, &UI::ProjectManager::openNewWindowCreateProjectTriggered, this, [this]() { controller_.createProjectInNewWindow(); });

    // --- Open project in this window ---
    QObject::connect(&window, &UI::ProjectManager::openProjectThisWindowTriggered, this, [this]() { controller_.openProjectInCurrentWindow(); });

    // --- Create project in this window ---
    QObject::connect(&window, &UI::ProjectManager::createProjectThisWindowTriggered, this, [this]() { controller_.createProjectInCurrentWindow(); });

    // --- Open file ---
    QObject::connect(&window, &UI::ProjectManager::openFileTriggered, this, [this]() { controller_.openFile(); });

    // --- Rename tab ---
    QObject::connect(&window, &UI::ProjectManager::renameTabTriggered, this, [this]() { controller_.renameTab(); });

    // --- Remove tab ---
    QObject::connect(&window, &UI::ProjectManager::removeTabTriggered, this, [this]() { controller_.removeTab(); });

    // --- Create file ---
    QObject::connect(&window, &UI::ProjectManager::createFileTriggered, this, [this]() { controller_.createFile(); });

    // --- Project ---

    QObject::connect(&window, &UI::ProjectManager::renameProjectTriggered, this, [this]() { controller_.renameProject(); });

    // --- Delete project ---
    QObject::connect(&window, &UI::ProjectManager::deleteProjectTriggered, this, [this]() { controller_.deleteProject(); });

    // Close
    /* QObject::connect(&window, &UI::ProjectManager::closeApplication,
                      this,[this]() {
                          controller_.closeApplication();
                      });*/

    // Tools - constrains
    QObject::connect(&window, &UI::ProjectManager::constraintTriggered, this, [this, &window](const QString& tabName, UI::ConstraintType _t1) {
        switch (_t1) {
            case UI::ConstraintType::Distance: {
                auto* prompt = new UI::ParameterInputWidget("Input:",nullptr);
                connect(prompt, &UI::ParameterInputWidget::inputEnteredTriggered, this, [this, prompt](const QString& parameter) {
                    if (parameter.isEmpty()) {
                        return;
                    }
                    controller_.selectTool(ToolId::ConstraintDimension, parameter.toDouble());
                    prompt->deleteLater();
                });

                prompt->move(200, 200);
                prompt->show();
                break;
            }
            case UI::ConstraintType::Tangent:
                controller_.selectTool(ToolId::ConstraintPointOnLine);
                break;
            case UI::ConstraintType::Coincident:
                controller_.selectTool(ToolId::ConstraintCoincidentPoints);
                break;
            case UI::ConstraintType::Parallel:
                controller_.selectTool(ToolId::ConstraintParallelLines);
                break;
            case UI::ConstraintType::Perpendicular:
                controller_.selectTool(ToolId::ConstraintPerpendicularLines);
                break;
            case UI::ConstraintType::Collinear:
                controller_.selectTool(ToolId::ConstraintAngleBetweenLines);
                break;
            default:
                break;
        }
    });

    // Tools - point/line
    QObject::connect(&window, &UI::ProjectManager::primitiveTriggered, this, [this](const QString tabName, UI::PrimitiveType _t1) {
        switch (_t1) {
            case UI::PrimitiveType::Point:
                controller_.selectTool(ToolId::Point);
                break;

            case UI::PrimitiveType::CubicBezier:
                controller_.selectTool(ToolId::CubicBezier);
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
                // controller_.selectTool(ToolId::);
                break;


            case UI::PrimitiveType::CircleByRadius:
                controller_.selectTool(ToolId::CircleByRadius);
                break;
            case UI::PrimitiveType::CircleByDiameter:
                controller_.selectTool(ToolId::CircleByDiameter);
                break;
            case UI::PrimitiveType::CircleByTwoPoints:
                controller_.selectTool(ToolId::CircleByTwoPoints);
                break;
            case UI::PrimitiveType::CircleByThreePoints:
                controller_.selectTool(ToolId::CircleByThreePoints);
                break;
            case UI::PrimitiveType::CircleTangentTwoLines:
                controller_.selectTool(ToolId::CircleTangentTwoLines);
                break;
            case UI::PrimitiveType::CircleTangentThreeLines:
                controller_.selectTool(ToolId::CircleTangentThreeLines);
                break;


            case UI::PrimitiveType::ArcByRadius:
                controller_.selectTool(ToolId::ArcByThreePoints);
                break;
            case UI::PrimitiveType::ArcByDiameter:
                controller_.selectTool(ToolId::ArcByThreePoints);
                break;
            case UI::PrimitiveType::ArcThreePoints:
                controller_.selectTool(ToolId::ArcByThreePoints);
                break;
            case UI::PrimitiveType::ArcSettings:
                controller_.selectTool(ToolId::ArcByThreePoints);
                break;
            default:
                break;
        }
    });

    // Tools - cursor/size
    QObject::connect(&window, &UI::ProjectManager::toolsTriggered, this, [this](const QString& tabName, UI::ToolsType _t1) {
        if (_t1 == UI::ToolsType::Cursor) {
            controller_.selectTool(ToolId::Cursor);
        }
    });
}
