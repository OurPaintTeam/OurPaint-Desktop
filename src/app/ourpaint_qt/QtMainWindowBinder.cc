#include "QtMainWindowBinder.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QKeySequence>
#include <QStyle>
#include <QSignalBlocker>
#include <iostream>
#include <limits>
#include <numbers>

#include "InputWidget.h"
#include "ParameterInputWidget.h"
#include "ProjectMenu.h"
#include "ProjectWindow.h"
#include "ToolsType.h"
#include "QtViewportHost.h"
#include "UIObserver.h"

QtMainWindowBinder::QtMainWindowBinder(UI::ProjectManager& window, UIController& controller, QtViewportHost& host, QObject* parent)
    : QObject(parent), window_(window), controller_(controller), host_(&host) {
    UIController::Callbacks callbacks;
    callbacks.attachViewport = [this](ViewportController& viewport) {
        return host_ && host_->addController(&viewport) >= 0;
    };
    callbacks.detachViewport = [this](ViewportController& viewport) {
        if (host_) {
            host_->removeController(&viewport);
        }
    };
    // The tab widget emits selection signals during programmatic changes. The
    // controller publishes the final active tab separately after updating its state.
    callbacks.tabAdded = [this](const std::string& name) {
        const QSignalBlocker blocker(window_);
        window_.addTabSlot(QString::fromStdString(name));
    };
    callbacks.tabRemoved = [this](const std::string& name) {
        const QSignalBlocker blocker(window_);
        window_.deleteTabSlot(QString::fromStdString(name));
    };
    callbacks.activeChanged = [this](const std::string& name, ViewportController& viewport) {
        if (host_) {
            host_->setEventSink(&viewport);
        }
        const QSignalBlocker blocker(window_);
        window_.setActiveTabNameProject(QString::fromStdString(name));
    };
    callbacks.toolChanged = [this](const std::string& name, ToolId tool, bool active) {
        UIObserver::presentActiveTool(window_, name, tool, active);
    };
    callbacks.redraw = [this]() {
        if (host_) {
            host_->requestRedraw();
        }
    };
    controller_.setCallbacks(std::move(callbacks));
    installProjectActions();

    connect(&window, &UI::ProjectManager::saveFullProjectTriggered, this, [this]() { invoke([this]() { saveProject(); }); });
    // Console
    QObject::connect(&window, &UI::ProjectManager::sentCommandTriggered, this, [this](const QString& tabName, const QString& str) {
        (void)tabName;
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
    QObject::connect(&window, &UI::ProjectManager::openNewWindowOpenProjectTriggered, this,
                     [this](const QString& path) { invoke([this, path]() { openProjectInNewWindow(path); }); });

    // --- Create project in new window ---
    QObject::connect(&window, &UI::ProjectManager::openNewWindowCreateProjectTriggered, this,
                     [this](const QString& path) { invoke([this, path]() { createProjectInNewWindow(path); }); });

    // --- Open project in this window ---
    QObject::connect(&window, &UI::ProjectManager::openProjectThisWindowTriggered, this,
                     [this](const QString& path) { invoke([this, path]() { openProjectInCurrentWindow(path); }); });

    // --- Create project in this window ---
    QObject::connect(&window, &UI::ProjectManager::createProjectThisWindowTriggered, this,
                     [this](const QString& path) { invoke([this, path]() { createProjectInCurrentWindow(path); }); });

    // --- Open file ---
    QObject::connect(&window, &UI::ProjectManager::openFileTriggered, this,
                     [this](const QString& path) { invoke([this, path]() { openFile(path); }); });

    // --- Rename tab ---
    QObject::connect(&window, &UI::ProjectManager::renameTabTriggered, this, [this](const QString& oldName, const QString& newName) {
        std::cout << "remove file triggered: " << oldName.toStdString() << " -> " << newName.toStdString() << '\n';
        const QSignalBlocker blocker(window_);
        if (controller_.renameTab(oldName.toStdString(), newName.toStdString())) {
            window_.renameTabSlot(oldName, newName);
        } else {
            window_.renameTabSlot(newName, oldName);
        }
    });

    // --- Remove tab ---
    QObject::connect(&window, &UI::ProjectManager::removeTabTriggered, this, [this](const QString& fileName) {
        std::cout << "remove file triggered: " << fileName.toStdString() << '\n';
        controller_.removeFile(fileName.toStdString());
    });

    // --- Create file ---
    QObject::connect(&window, &UI::ProjectManager::createFileTriggered, this, [this](const QString& fileName) {
        std::cout << "create file triggered: " << fileName.toStdString() << '\n';
        invoke([this, fileName]() { controller_.createFile(fileName.toStdString()); });
    });

    // --- Set active file
    QObject::connect(&window, &UI::ProjectManager::setActiveTabTriggered, this, [this](const QString& fileName) {
        std::cout << "set active file: " << fileName.toStdString() << '\n';
        controller_.setActiveFile(fileName.toStdString());
    });



    // Tools - constraints
    QObject::connect(&window, &UI::ProjectManager::constraintTriggered, this, [this](const QString& tabName, UI::ConstraintType type) {
        switch (type) {
            case UI::ConstraintType::Distance:
            case UI::ConstraintType::VDistance:
            case UI::ConstraintType::HDistance:
            case UI::ConstraintType::Angle: {
                ToolId constraintType;
                QString promptText;
                if (type == UI::ConstraintType::Angle) {
                    constraintType = ToolId::Angle;
                    promptText = tr("Angle (degrees):");
                } else if (type == UI::ConstraintType::VDistance) {
                    constraintType = ToolId::VerticalDistance;
                    promptText = tr("Vertical Distance:");
                } else if (type == UI::ConstraintType::HDistance) {
                    constraintType = ToolId::HorizontalDistance;
                    promptText = tr("Horizontal Distance:");
                } else if (type == UI::ConstraintType::Distance) {
                    constraintType = ToolId::Distance;
                    promptText = tr("Distance:");
                }

                auto* prompt = new UI::ParameterInputWidget(promptText, nullptr);
                connect(prompt, &UI::ParameterInputWidget::inputEnteredTriggered, this, [this, tabName, constraintType](const QString& parameter) {
                    bool valid = false;
                    const double value = parameter.toDouble(&valid);
                    const double scale = constraintType == ToolId::Angle ? std::numbers::pi / 180.0 : 1.0;
                    controller_.selectTool(constraintType, valid ? value * scale : std::numeric_limits<double>::quiet_NaN(), tabName.toStdString());
                });

                prompt->move(200, 200);
                prompt->show();
                break;
            }
            case UI::ConstraintType::Tangent:
                controller_.selectTool(ToolId::Tangent, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Coincident:
                controller_.selectTool(ToolId::Coincident, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Horizontal:
                controller_.selectTool(ToolId::Horizontal, std::nullopt,  tabName.toStdString());
                break;
            case UI::ConstraintType::Vertical:
                controller_.selectTool(ToolId::Vertical, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Parallel:
                controller_.selectTool(ToolId::Parallel, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Perpendicular:
                controller_.selectTool(ToolId::Perpendicular, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Equal:
                controller_.selectTool(ToolId::Equal, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::FixUnfix:
                controller_.selectTool(ToolId::FixUnfix, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Concentric:
                controller_.selectTool(ToolId::Concentric, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Collinear:
                controller_.selectTool(ToolId::Collinear, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Midpoint:
                controller_.selectTool(ToolId::Midpoint, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Symmetric:
                controller_.selectTool(ToolId::Symmetric, std::nullopt, tabName.toStdString());
                break;
            case UI::ConstraintType::Curvature:
                controller_.selectTool(ToolId::Curvature, std::nullopt, tabName.toStdString());
                break;
            default:
                return;
        }
    });

    // Tools - point/line
    QObject::connect(&window, &UI::ProjectManager::primitiveTriggered, this, [this](const QString tabName, UI::PrimitiveType _t1) {
        switch (_t1) {
            case UI::PrimitiveType::Point:
                controller_.selectTool(ToolId::Point, std::nullopt, tabName.toStdString());
                break;

            case UI::PrimitiveType::CubicBezier:
                controller_.selectTool(ToolId::Nurbs, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::Line:
                controller_.selectTool(ToolId::Line, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::Polyline:
                controller_.selectTool(ToolId::Polyline, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::InfiniteLine:
                controller_.selectTool(ToolId::InfiniteLine, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::LineSettings:
                // controller_.selectTool(ToolId::);
                break;


            case UI::PrimitiveType::CircleByRadius:
                controller_.selectTool(ToolId::CircleByRadius, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::CircleByDiameter:
                controller_.selectTool(ToolId::CircleByDiameter, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::CircleByTwoPoints:
                controller_.selectTool(ToolId::CircleByTwoPoints, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::CircleByThreePoints:
                controller_.selectTool(ToolId::CircleByThreePoints, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::CircleTangentTwoLines:
                controller_.selectTool(ToolId::CircleTangentTwoLines, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::CircleTangentThreeLines:
                controller_.selectTool(ToolId::CircleTangentThreeLines, std::nullopt, tabName.toStdString());
                break;


            case UI::PrimitiveType::ArcByRadius:
                controller_.selectTool(ToolId::ArcByRadius, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::ArcByDiameter:
                controller_.selectTool(ToolId::ArcByDiameter, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::ArcThreePoints:
                controller_.selectTool(ToolId::ArcByThreePoints, std::nullopt, tabName.toStdString());
                break;
            case UI::PrimitiveType::ArcSettings:
                controller_.selectTool(ToolId::ArcByThreePoints, std::nullopt, tabName.toStdString());
                break;
            default:
                break;
        }
    });

    // Tools - cursor/size
    QObject::connect(&window, &UI::ProjectManager::toolsTriggered, this, [this](const QString& tabName, UI::ToolsType _t1) {
        if (_t1 == UI::ToolsType::Cursor) {
            controller_.selectTool(ToolId::Cursor, std::nullopt, tabName.toStdString());
        }
    });
}

QtMainWindowBinder::~QtMainWindowBinder() {
    // The project window may already have closed. Detach surviving GL contexts
    // without sending any further presentation updates to the closed window.
    UIController::Callbacks teardown;
    teardown.detachViewport = [this](ViewportController& viewport) {
        if (host_) {
            host_->removeController(&viewport);
        }
    };
    controller_.setCallbacks(std::move(teardown));
    controller_.clearViews();
    controller_.setCallbacks({});
}

void QtMainWindowBinder::invoke(const std::function<void()>& action) {
    try {
        action();
    } catch (const std::exception& error) {
        window_.addNotification(tr("The operation failed: %1").arg(QString::fromUtf8(error.what())));
    }
}

bool QtMainWindowBinder::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Close) {
        try {
            if (confirmClose()) {
                return false;
            }
        } catch (const std::exception& error) {
            window_.addNotification(tr("The operation failed: %1").arg(QString::fromUtf8(error.what())));
        }
        static_cast<QCloseEvent*>(event)->ignore();
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void QtMainWindowBinder::installProjectActions() {
    for (auto* widget : QApplication::topLevelWidgets()) {
        auto* projectWindow = qobject_cast<UI::ProjectWindow*>(widget);
        if (!projectWindow) {
            continue;
        }
        projectWindow->installEventFilter(this);
        auto* save = new QAction(projectWindow->style()->standardIcon(QStyle::SP_DialogSaveButton), tr("Save project"), this);
        save->setShortcut(QKeySequence::Save);
        auto* saveAs = new QAction(tr("Save project as..."), this);
        saveAs->setShortcut(QKeySequence::SaveAs);
        projectWindow->addAction(save);
        projectWindow->addAction(saveAs);
        connect(save, &QAction::triggered, this, [this]() { invoke([this]() { saveProject(); }); });
        connect(saveAs, &QAction::triggered, this, [this]() { invoke([this]() { saveProjectAs(); }); });
        if (auto* menu = projectWindow->findChild<UI::ProjectMenu*>()) {
            menu->addSeparator();
            menu->addAction(save);
            menu->addAction(saveAs);
        }
    }
}
