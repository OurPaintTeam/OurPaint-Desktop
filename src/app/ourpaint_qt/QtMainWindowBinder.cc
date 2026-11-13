#include "QtMainWindowBinder.h"

#include <QCloseEvent>
#include <QFile>
#include <QKeySequence>
#include <QStyle>
#include <QSignalBlocker>
#include <iostream>
#include <limits>
#include <numbers>
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>

#include "ProjectWindow.h"
#include "QtPath.h"

#include "InputWidget.h"
#include "ParameterInputWidget.h"
#include "ProjectMenu.h"
#include "ToolsType.h"
#include "QtViewportHost.h"
#include "UIObserver.h"

QtMainWindowBinder::QtMainWindowBinder(UI::ProjectManager& window, UIController& controller, QtViewportHost& host, QObject* parent)
    : QObject(parent), window_(window), controller_(controller), host_(&host) {
    connect(&observer_, &UIObserver::viewClosing, this, [this](ViewId view) {
        if (host_) {
            host_->removeController(controller_.findViewport(view));
        }
        const auto label = labels_.take(view);
        if (!tearingDown_ && !label.isEmpty()) {
            const QSignalBlocker blocker(window_);
            window_.deleteTabSlot(label);
        }
    }, Qt::DirectConnection);
    connect(&observer_, &UIObserver::activeChanged, this, [this](ViewId view) {
        if (host_) {
            host_->setEventSink(controller_.findViewport(view));
        }
        if (const auto tool = controller_.activeToolId(view); tool && labels_.contains(view)) {
            const QSignalBlocker blocker(window_);
            window_.setActiveTabNameProject(labels_[view]);
            UIObserver::presentActiveTool(window_, labels_[view].toStdString(), *tool, true);
        }
    });
    connect(&observer_, &UIObserver::selectedToolChanged, this, [this](ViewId view, ToolId tool) {
        if (labels_.contains(view)) {
            UIObserver::presentActiveTool(window_, labels_[view].toStdString(), tool, view == controller_.activeDocumentView());
        }
    });
    connect(&observer_, &UIObserver::redraw, this, [this](ViewId view) {
        if (host_ && view == controller_.activeDocumentView()) {
            host_->requestRedraw();
        }
    });
    installProjectActions();

    connect(&window, &UI::ProjectManager::saveFullProjectTriggered, this, [this]() { invoke([this]() { saveProject(); }); });
    // Console
    QObject::connect(&window, &UI::ProjectManager::sentCommandTriggered, this, [this](const QString& tabName, const QString& str) {
        (void)tabName;
        try {
            //controller_.executeConsoleCommand(str.toStdString());
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
                //controller_.executeConsoleCommand(line.toStdString());
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
        renameViewDocument(oldName, newName);
    });

    // --- Remove tab ---
    QObject::connect(&window, &UI::ProjectManager::removeTabTriggered, this, [this](const QString& fileName) {
        std::cout << "remove file triggered: " << fileName.toStdString() << '\n';
        controller_.closeDocumentView(viewForTab(fileName));
    });

    // --- Create file ---
    QObject::connect(&window, &UI::ProjectManager::createFileTriggered, this, [this](const QString& fileName) {
        std::cout << "create file triggered: " << fileName.toStdString() << '\n';
        invoke([this, fileName]() { showNewView(controller_.createDocument(fileName.toStdString())); });
    });

    // --- Set active file
    QObject::connect(&window, &UI::ProjectManager::setActiveTabTriggered, this, [this](const QString& fileName) {
        std::cout << "set active file: " << fileName.toStdString() << '\n';
        controller_.setActiveDocumentView(viewForTab(fileName));
    });



    // Tools - constraints
    QObject::connect(&window, &UI::ProjectManager::constraintTriggered, this, [this](const QString& tabName, UI::ConstraintType type) {
        const ViewId view = viewForTab(tabName);
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
                connect(prompt, &UI::ParameterInputWidget::inputEnteredTriggered, this, [this, view, constraintType](const QString& parameter) {
                    bool valid = false;
                    const double value = parameter.toDouble(&valid);
                    const double scale = constraintType == ToolId::Angle ? std::numbers::pi / 180.0 : 1.0;
                    controller_.selectTool(view, constraintType, valid ? value * scale : std::numeric_limits<double>::quiet_NaN());
                });

                prompt->move(200, 200);
                prompt->show();
                break;
            }
            case UI::ConstraintType::Tangent:
                controller_.selectTool(view, ToolId::Tangent, std::nullopt);
                break;
            case UI::ConstraintType::Coincident:
                controller_.selectTool(view, ToolId::Coincident, std::nullopt);
                break;
            case UI::ConstraintType::Horizontal:
                controller_.selectTool(view, ToolId::Horizontal, std::nullopt);
                break;
            case UI::ConstraintType::Vertical:
                controller_.selectTool(view, ToolId::Vertical, std::nullopt);
                break;
            case UI::ConstraintType::Parallel:
                controller_.selectTool(view, ToolId::Parallel, std::nullopt);
                break;
            case UI::ConstraintType::Perpendicular:
                controller_.selectTool(view, ToolId::Perpendicular, std::nullopt);
                break;
            case UI::ConstraintType::Equal:
                controller_.selectTool(view, ToolId::Equal, std::nullopt);
                break;
            case UI::ConstraintType::FixUnfix:
                controller_.selectTool(view, ToolId::FixUnfix, std::nullopt);
                break;
            case UI::ConstraintType::Concentric:
                controller_.selectTool(view, ToolId::Concentric, std::nullopt);
                break;
            case UI::ConstraintType::Collinear:
                controller_.selectTool(view, ToolId::Collinear, std::nullopt);
                break;
            case UI::ConstraintType::Midpoint:
                controller_.selectTool(view, ToolId::Midpoint, std::nullopt);
                break;
            case UI::ConstraintType::Symmetric:
                controller_.selectTool(view, ToolId::Symmetric, std::nullopt);
                break;
            case UI::ConstraintType::Curvature:
                controller_.selectTool(view, ToolId::Curvature, std::nullopt);
                break;
            default:
                return;
        }
    });

    // Tools - point/line
    QObject::connect(&window, &UI::ProjectManager::primitiveTriggered, this, [this](const QString tabName, UI::PrimitiveType _t1) {
        const ViewId view = viewForTab(tabName);
        switch (_t1) {
            case UI::PrimitiveType::Point:
                controller_.selectTool(view, ToolId::Point, std::nullopt);
                break;

            case UI::PrimitiveType::CubicBezier:
                controller_.selectTool(view, ToolId::Nurbs, std::nullopt);
                break;
            case UI::PrimitiveType::Line:
                controller_.selectTool(view, ToolId::Line, std::nullopt);
                break;
            case UI::PrimitiveType::Polyline:
                controller_.selectTool(view, ToolId::Polyline, std::nullopt);
                break;
            case UI::PrimitiveType::InfiniteLine:
                controller_.selectTool(view, ToolId::InfiniteLine, std::nullopt);
                break;
            case UI::PrimitiveType::LineSettings:
                // controller_.selectTool(ToolId::);
                break;


            case UI::PrimitiveType::CircleByRadius:
                controller_.selectTool(view, ToolId::CircleByRadius, std::nullopt);
                break;
            case UI::PrimitiveType::CircleByDiameter:
                controller_.selectTool(view, ToolId::CircleByDiameter, std::nullopt);
                break;
            case UI::PrimitiveType::CircleByTwoPoints:
                controller_.selectTool(view, ToolId::CircleByTwoPoints, std::nullopt);
                break;
            case UI::PrimitiveType::CircleByThreePoints:
                controller_.selectTool(view, ToolId::CircleByThreePoints, std::nullopt);
                break;
            case UI::PrimitiveType::CircleTangentTwoLines:
                controller_.selectTool(view, ToolId::CircleTangentTwoLines, std::nullopt);
                break;
            case UI::PrimitiveType::CircleTangentThreeLines:
                controller_.selectTool(view, ToolId::CircleTangentThreeLines, std::nullopt);
                break;


            case UI::PrimitiveType::ArcByRadius:
                controller_.selectTool(view, ToolId::ArcByRadius, std::nullopt);
                break;
            case UI::PrimitiveType::ArcByDiameter:
                controller_.selectTool(view, ToolId::ArcByDiameter, std::nullopt);
                break;
            case UI::PrimitiveType::ArcThreePoints:
                controller_.selectTool(view, ToolId::ArcByThreePoints, std::nullopt);
                break;
            case UI::PrimitiveType::ArcSettings:
                controller_.selectTool(view, ToolId::ArcByThreePoints, std::nullopt);
                break;
            default:
                break;
        }
    });

    // Tools - cursor/size
    QObject::connect(&window, &UI::ProjectManager::toolsTriggered, this, [this](const QString& tabName, UI::ToolsType _t1) {
        const ViewId view = viewForTab(tabName);
        if (_t1 == UI::ToolsType::Cursor) {
            controller_.selectTool(view, ToolId::Cursor, std::nullopt);
        }
    });
    controller_.setObserver(&observer_);
}

QtMainWindowBinder::~QtMainWindowBinder() {
    tearingDown_ = true;
    controller_.clearDocumentViews();
    controller_.setObserver(nullptr);
}

ViewId QtMainWindowBinder::viewForTab(const QString& label) const {
    return label.isEmpty() ? controller_.activeDocumentView() : labels_.key(label, InvalidViewId);
}

QString QtMainWindowBinder::availableLabel(const QString& base, ViewId except) const {
    QString name = base;
    for (int suffix = 2;; ++suffix) {
        const ViewId owner = labels_.key(name, InvalidViewId);
        if (owner == InvalidViewId || owner == except) {
            return name;
        }
        name = base + QStringLiteral(" (%1)").arg(suffix);
    }
}

void QtMainWindowBinder::showNewView(DocumentId document) {
    const ViewId view = controller_.createDocumentView(document);
    if (view == InvalidViewId) {
        return;
    }
    try {
        if (!host_ || host_->addController(controller_.findViewport(view)) < 0) {
            throw std::runtime_error("Could not create the document viewport");
        }
        auto base = QString::fromStdString(controller_.documentName(document));
        auto name = availableLabel(base.isEmpty() ? tr("Sketch") : base);
        labels_.insert(view, name);
        const QSignalBlocker blocker(window_);
        window_.addTabSlot(name);
        controller_.setActiveDocumentView(view);
    } catch (...) {
        controller_.closeDocumentView(view);
        throw;
    }
}

void QtMainWindowBinder::renameViewDocument(const QString& oldLabel, const QString& newName) {
    const auto view = viewForTab(oldLabel);
    const auto document = controller_.documentOf(view);
    if (newName.isEmpty() || availableLabel(newName, view) != newName || !controller_.renameDocument(document, newName.toStdString())) {
        return;
    }
    const QSignalBlocker blocker(window_);
    window_.renameTabSlot(oldLabel, newName);
    labels_[view] = newName;
    for (auto it = labels_.begin(); it != labels_.end(); ++it) {
        if (it.key() != view && controller_.documentOf(it.key()) == document) {
            const auto label = availableLabel(newName, it.key());
            window_.renameTabSlot(it.value(), label);
            it.value() = label;
        }
    }
    // ProjectManager selects the last renamed label, even for a background tab.
    window_.setActiveTabNameProject(labels_.value(controller_.activeDocumentView()));
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
            auto* newView = menu->addAction(tr("New view of current document"));
            connect(newView, &QAction::triggered, this, [this] {
                invoke([this] { showNewView(controller_.documentOf(controller_.activeDocumentView())); });
            });
            menu->addSeparator();
            menu->addAction(save);
            menu->addAction(saveAs);
        }
    }
}


namespace {

QWidget* dialogParent() {
    for (auto* widget : QApplication::topLevelWidgets()) {
        if (qobject_cast<UI::ProjectWindow*>(widget) && widget->isVisible()) {
            return widget;
        }
    }
    return QApplication::activeWindow();
}

QString chooseFolder(const char* title) {
    return QFileDialog::getExistingDirectory(dialogParent(), QCoreApplication::translate("ProjectPersistence", title), {},
                                             QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
}

QString fileErrorText(const app::FileError& error) {
    QString message;
    using app::FileStage;
    switch (error.stage) {
        case FileStage::ValidateProject:
            message = QCoreApplication::translate("ProjectPersistence", "The project data is invalid.");
            break;
        case FileStage::ValidatePath:
            message = QCoreApplication::translate("ProjectPersistence", "This file or folder cannot be used.");
            break;
        case FileStage::GenerateId:
            message = QCoreApplication::translate("ProjectPersistence", "Could not create a project identifier.");
            break;
        case FileStage::Encode:
            message = QCoreApplication::translate("ProjectPersistence", "Could not prepare a document for saving.");
            break;
        case FileStage::CreateDirectory:
            message = QCoreApplication::translate("ProjectPersistence", "Could not create the project folder.");
            break;
        case FileStage::Open:
        case FileStage::Read:
        case FileStage::Enumerate:
            message = QCoreApplication::translate("ProjectPersistence", "Could not read the file or project folder.");
            break;
        case FileStage::Decode:
            message = QCoreApplication::translate("ProjectPersistence", "The file is damaged or uses an unsupported format.");
            break;
        default:
            message = QCoreApplication::translate("ProjectPersistence", "Could not finish saving the file.");
            break;
    }
    message += QStringLiteral("\n") + app::ui::pathText(error.path);
    if (error.codecError) {
        message += QStringLiteral("\n") + QString::fromStdString(error.codecError->message);
    }
    if (error.systemError) {
        message += QStringLiteral("\n") + QString::fromLocal8Bit(error.systemError.message().c_str());
    }
    return message;
}

void present(UI::ProjectManager& manager, const app::SaveReport& report) {
    if (report) {
        manager.addNotification(QCoreApplication::translate("ProjectPersistence", "Project saved to %1.").arg(app::ui::pathText(report.root)));
        return;
    }
    QString text = fileErrorText(*report.error);
    text += QCoreApplication::translate("ProjectPersistence", "\nSaved files: %1. Not attempted: %2.")
                .arg(static_cast<qulonglong>(report.saved.size()))
                .arg(static_cast<qulonglong>(report.unprocessed.size()));
    if (report.failedFileReplaced) {
        text += QCoreApplication::translate("ProjectPersistence", "\nThe file already contains the new data, but saving could not be fully confirmed.");
    }
    manager.addNotification(text);
}

}  // namespace

void QtMainWindowBinder::updateProjectData() {
    const auto& metadata = controller_.projectMetadata();
    window_.setProjectData({app::ui::pathText(controller_.projectRoot()), QString::fromStdString(metadata.id)});
    if (auto* window = dialogParent()) {
        window->setWindowTitle(QString::fromStdString(metadata.name) + QStringLiteral(" - OurPaint"));
    }
}

bool QtMainWindowBinder::saveProject() {
    if (controller_.projectRoot().empty()) {
        return saveProjectAs();
    }
    auto report = controller_.saveProject();
    updateProjectData();
    present(window_, report);
    return static_cast<bool>(report);
}

bool QtMainWindowBinder::saveProjectAs() {
    const auto path = chooseFolder("Save project to a new or empty folder");
    if (path.isEmpty()) {
        return false;
    }
    auto report = controller_.saveProjectAs(app::ui::nativePath(path));
    if (report) {
        updateProjectData();
    }
    present(window_, report);
    return static_cast<bool>(report);
}

bool QtMainWindowBinder::confirmClose() {
    controller_.endInteractions();
    if (!controller_.needsSaveConfirmation()) {
        return true;
    }
    const auto answer = QMessageBox::question(dialogParent(), QCoreApplication::translate("ProjectPersistence", "Unsaved project"),
                                              QCoreApplication::translate("ProjectPersistence", "Save the current project before continuing?"),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    return answer == QMessageBox::Discard || (answer == QMessageBox::Save && saveProject());
}

bool QtMainWindowBinder::openProjectInCurrentWindow(const QString& requestedPath) {
    const auto selected = requestedPath.isEmpty() ? chooseFolder("Open project folder") : requestedPath;
    if (selected.isEmpty() || !confirmClose()) {
        return false;
    }
    if (auto error = controller_.openProject(app::ui::nativePath(selected))) {
        window_.addNotification(fileErrorText(*error));
        return false;
    }
    updateProjectData();
    for (const auto document : controller_.documents()) {
        showNewView(document);
    }
    window_.addNotification(QCoreApplication::translate("ProjectPersistence", "Project opened."));
    return true;
}

bool QtMainWindowBinder::createProjectInCurrentWindow(const QString& requestedPath) {
    const auto selected = requestedPath.isEmpty() ? chooseFolder("Create project in a new or empty folder") : requestedPath;
    if (selected.isEmpty() || !confirmClose()) {
        return false;
    }
    auto report = controller_.createProject(app::ui::nativePath(selected));
    if (report) {
        updateProjectData();
    }
    present(window_, report);
    return static_cast<bool>(report);
}

void QtMainWindowBinder::openProjectInNewWindow(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty() ? chooseFolder("Open project folder") : requestedPath;
    if (!path.isEmpty() && !QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--open-project"), path})) {
        window_.addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open another window."));
    }
}

void QtMainWindowBinder::createProjectInNewWindow(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty() ? chooseFolder("Create project in a new or empty folder") : requestedPath;
    if (!path.isEmpty() && !QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--create-project"), path})) {
        window_.addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open another window."));
    }
}

void QtMainWindowBinder::openFile(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty()
                          ? QFileDialog::getOpenFileName(dialogParent(), QCoreApplication::translate("ProjectPersistence", "Open sketch or project"), {},
                                                         QCoreApplication::translate("ProjectPersistence", "OurPaint files (*.ourp .ourpaint);;All files (*)"))
                          : requestedPath;
    if (path.isEmpty()) {
        return;
    }
    if (QFileInfo(path).isDir() || QFileInfo(path).fileName() == QStringLiteral(".ourpaint")) {
        openProjectInCurrentWindow(path);
        return;
    }
    auto result = controller_.openFile(app::ui::nativePath(path));
    if (!result) {
        window_.addNotification(fileErrorText(result.error()));
        return;
    }
    // Opening an already visible project file focuses its tab. Creating a second
    // independent view is an explicit menu action.
    for (auto it = labels_.cbegin(); it != labels_.cend(); ++it) {
        if (controller_.documentOf(it.key()) == result.value()) {
            controller_.setActiveDocumentView(it.key());
            return;
        }
    }
    showNewView(result.value());
}
