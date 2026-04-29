#include "QtMainWindowBinder.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

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

    // --- Open project in new window ---
    QObject::connect(&window, &UI::ProjectManager::openNewWindowOpenProjectTriggered, this, [this]() { controller_.openProjectInNewWindow(); });

    // --- Create project in new window ---
    QObject::connect(&window, &UI::ProjectManager::openNewWindowCreateProjectTriggered, this, [this]() { controller_.createProjectInNewWindow(); });

    // --- Open project in this window ---
    QObject::connect(&window, &UI::ProjectManager::openProjectThisWindowTriggered, this, [this]() { controller_.openProjectInCurrentWindow(); });

    // --- Create project in this window ---
    QObject::connect(&window, &UI::ProjectManager::createProjectThisWindowTriggered, this, [this]() { controller_.createProjectInCurrentWindow(); });

    // --- Open file ---
    QObject::connect(&window, &UI::ProjectManager::openFileTriggered, this, [this](const QString& filePath) {
        const std::string documentName = controller_.openFile(filePath.toStdString());
        if (documentName.empty()) {
            window_.addNotification("Error: cannot open file");
            return;
        }

        const QString tabName = QString::fromStdString(documentName);
        window_.addTabSlot(tabName);
        window_.setActiveTabNameProject(tabName);
        window_.addNotification("File opened: " + QFileInfo(filePath).fileName());
    });

    // --- Rename tab ---
    QObject::connect(&window, &UI::ProjectManager::renameTabTriggered, this, [this]() { controller_.renameTab(); });

    // --- Remove tab ---
    QObject::connect(&window, &UI::ProjectManager::removeTabTriggered, this, [this]() { controller_.removeTab(); });

    // --- Create file ---
    QObject::connect(&window, &UI::ProjectManager::createFileTriggered, this, [this](const QString& fileName) {
        const std::string documentName = controller_.createFile(fileName.toStdString());
        if (documentName.empty()) {
            window_.addNotification("Error: cannot create file");
            return;
        }

        const QString tabName = QString::fromStdString(documentName);
        window_.addTabSlot(tabName);
        window_.setActiveTabNameProject(tabName);
    });

    // --- Save/export file ---
    QObject::connect(&window, &UI::ProjectManager::exportFileTriggered, this, [this](const QString& filePath) {
        const QString oldName = QString::fromStdString(controller_.activeDocumentName());
        const std::string savedName = controller_.saveActiveDocumentAs(filePath.toStdString());
        if (savedName.empty()) {
            window_.addNotification("Error: cannot save file");
            return;
        }

        const QString newName = QString::fromStdString(savedName);
        if (!oldName.isEmpty() && oldName != newName) {
            window_.renameTabSlot(oldName, newName);
        }
        window_.setActiveTabNameProject(newName);
        window_.addNotification("File saved: " + QFileInfo(filePath).fileName());
    });

    // --- Project ---

    QObject::connect(&window, &UI::ProjectManager::renameProjectTriggered, this, [this]() { controller_.renameProject(); });

    QObject::connect(&window, &UI::ProjectManager::setActiveTabTriggered, this, [this](const QString& name) {
        controller_.setActiveDocument(name.toStdString());
    });

    // --- Delete project ---
    QObject::connect(&window, &UI::ProjectManager::deleteProjectTriggered, this, [this]() { controller_.deleteProject(); });

    // Close
    /* QObject::connect(&window, &UI::ProjectManager::closeApplication,
                      this,[this]() {
                          controller_.closeApplication();
                      });*/

    // Tools - constrains
    QObject::connect(&window, &UI::ProjectManager::constraintTriggered, this, [this, &window](const QString& tabName, UI::ConstraintType _t1) {
        // auto* prompt = new UI::ParameterInputWidget("Input parament:",nullptr);
        //
        // connect(prompt, &UI::ParameterInputWidget::inputEnteredTriggered, this, [this, prompt](const QString& parametr) {
        //     if (parametr.isEmpty()) {
        //         return;
        //     }
        // qDebug()<<parametr;
        //     prompt->deleteLater();
        // });

        switch (_t1) {
            case UI::ConstraintType::PointPointDistance:
                controller_.selectTool(ToolId::ConstraintPointPointDistance);
               // controller_.selectTool(ToolId::ConstraintPointPointDistance);
               //controller_.selectTool(ToolId::ConstraintLineCircleDistance);
                break;
            case UI::ConstraintType::PointOnLine:
                controller_.selectTool(ToolId::ConstraintPointOnLine);
                break;
                break;
            case UI::ConstraintType::CoincidentPoints:
                controller_.selectTool(ToolId::ConstraintCoincidentPoints);
                break;
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
            case UI::PrimitiveType::EllipseThreePoints:
            case UI::PrimitiveType::CircleSettings:
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
