#include "QtUIController.h"

#include <QApplication>
#include <QFileInfo>
#include <QScreen>
#include <algorithm>

#include "CustomConsole.h"
#include "DocumentController.h"
#include "ParameterInputWidget.h"
#include "QtPlatformRuntime.h"
#include "QtViewportHost.h"

QtUIController::QtUIController(DocumentController& docController, QtPlatformRuntime& platformRuntime, Settings& settings)
    : docController_(docController), platformRuntime_(platformRuntime), ui_settings_(fs_.settingsPath()), app_settings_(settings) {
    activeHost_ = static_cast<QtViewportHost*>(platformRuntime_.createViewportHost());

    auto ctx = createEditorContext(false);
    auto m = createBaseManager(ctx);
    m->showStartWindow();

    currentLanguage_ = ui_settings_.language();
    applyLanguage(currentLanguage_);
}

QtUIController::~QtUIController() {
    for (auto& view : docController_.views()) {
        if (view && activeHost_) {
            activeHost_->removeController(&view->viewportController_);
        }
    }

    docController_.clearViews();

    qDeleteAll(projectManagers_);
    projectManagers_.clear();
}

UI::ProjectManager* QtUIController::createBaseManager(const UI::EditorContext& context) {
    QtViewportHost* host = static_cast<QtViewportHost*>(context.renderEngine);
    if (!host) {
        return nullptr;
    }

    auto manager = std::make_unique<UI::ProjectManager>(context);
    projectManagers_.push_back(manager.get());
    managerHosts_[manager.get()] = host;

    try {
        initBaseSignals(*manager);
    } catch (...) {
        managerHosts_.erase(manager.get());
        projectManagers_.removeOne(manager.get());
        throw;
    }

    return manager.release();
}

UI::ProjectManager* QtUIController::createLightManager(const UI::EditorContext& context) {
    QtViewportHost* host = static_cast<QtViewportHost*>(context.renderEngine);
    if (!host) {
        return nullptr;
    }

    auto manager = std::make_unique<UI::ProjectManager>(context);
    projectManagers_.push_back(manager.get());
    managerHosts_[manager.get()] = host;

    try {
        initLightManager(*manager);
    } catch (...) {
        managerHosts_.erase(manager.get());
        projectManagers_.removeOne(manager.get());
        throw;
    }

    return manager.release();
}

UI::EditorContext QtUIController::createEditorContext(bool isNewWindow) const {
    UI::EditorContext ctx;
    ctx.fileSystem = const_cast<UI::FileSystem*>(&fs_);
    ctx.settings = const_cast<UI::SettingsSystem*>(&ui_settings_);

    QtViewportHost* host = nullptr;
    if (isNewWindow) {
        host = platformRuntime_.createViewportHost();
    } else {
        host = activeHost_.data();
        if (!host) {
            host = platformRuntime_.createViewportHost();
        }
    }

    ctx.renderEngine = host;
    ctx.consoleEngine = nullptr;
    return ctx;
}

bool QtUIController::createFile(UI::ProjectManager* currentManager, const QString& path, bool newWindow) {
    if (path.trimmed().isEmpty()) {
        return false;
    }

    qint64 fileId = -1;
    if (fs_.createProject(path, fileId) != UI::FileSystem::FsResult::Ok) {
        if (currentManager) {
            currentManager->addNotification("❌ Failed to create file: " + path);
        }
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    DocumentId docId = docController_.createDocument(pd.path.toStdString());
    if (docId == DocumentController::InvalidId) {
        fs_.removeProject(fileId);
        if (currentManager) {
            currentManager->addNotification("❌ Failed to create document core");
        }
        return false;
    }

    UI::ProjectManager* targetManager = currentManager;
    bool isNew = newWindow || !targetManager;
    if (isNew) {
        targetManager = createBaseManager(createEditorContext(true));
    }

    if (!attachDocument(*targetManager, docId, fileId)) {
        docController_.closeDocument(docId);
        fs_.removeProject(fileId);
        return false;
    }

    UI::FileData fd{fileId, docController_.nameOf(docId).c_str()};
    if (isNew) {
        targetManager->showEditorWindow(fd);
    } else {
        targetManager->addTabSlot(fd);
    }

    targetManager->addNotification(isNew ? "✨ File created in new window: " + pd.name : "✨ File created: " + pd.name);
    return true;
}

bool QtUIController::openFile(UI::ProjectManager* currentManager, const QString& path, bool newWindow) {
    if (path.trimmed().isEmpty() || !currentManager) {
        return false;
    }

    qint64 fileId = -1;
    auto res = fs_.registerProject(path, fileId);
    if (res != UI::FileSystem::FsResult::Ok && res != UI::FileSystem::FsResult::AlreadyExists) {
        currentManager->addNotification("❌ Failed to open file: " + path);
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    DocumentId docId = docController_.loadDocument(pd.path.toStdString());
    if (docId == DocumentController::InvalidId) {
        currentManager->addNotification("❌ Failed to load document from disk");
        return false;
    }

    UI::ProjectManager* targetManager = currentManager;
    bool isNew = newWindow || !targetManager;
    if (isNew) {
        targetManager = createBaseManager(createEditorContext(true));
    }

    if (!attachDocument(*targetManager, docId, fileId)) {
        docController_.closeDocument(docId);
        return false;
    }

    UI::FileData fd{fileId, docController_.nameOf(docId).c_str()};
    if (isNew) {
        targetManager->showEditorWindow(fd);
    } else {
        targetManager->addTabSlot(fd);
    }

    targetManager->addNotification(isNew ? "📄 File opened in new window: " + pd.name : "📄 File opened: " + pd.name);
    return true;
}

bool QtUIController::openFileByID(UI::ProjectManager* currentManager, qint64 fileId, bool newWindow) {
    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        if (currentManager) {
            currentManager->addNotification("❌ File not found: " + QString::number(fileId));
        }
        return false;
    }
    return openFile(currentManager, pd.path, newWindow);
}

bool QtUIController::renameFile(UI::ProjectManager* currentManager, UI::FileData& data) {
    if (!currentManager) {
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(data.id, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    const std::string newName = data.name.toStdString();
    if (newName.empty() || pd.name.toStdString() == newName) {
        return false;
    }

    const DocumentId docId = docController_.idFromPath(pd.path.toStdString());
    if (docId == DocumentController::InvalidId) {
        return false;
    }

    if (!docController_.renameDocument(docId, newName)) {
        currentManager->addNotification("❌ Failed to rename file on disk.");
        return false;
    }

    if (fs_.renameProject(data.id, data.name) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    for (auto* m : projectManagers_) {
        if (m) {
            m->renameTabSlot(data);
        }
    }

    return true;
}

QString QtUIController::getFilePath(const qint64 fileId) {
    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) == UI::FileSystem::FsResult::Ok) {
        return pd.path;
    }
    return {};
}

bool QtUIController::removeFile(UI::ProjectManager* currentManager, const qint64 fileId) {
    if (!currentManager) {
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    const std::string path = pd.path.toStdString();
    DocumentId docId = docController_.idFromPath(path);

    if (docId != DocumentController::InvalidId) {
        closeDocumentView(docId);
        if (!docController_.closeDocument(docId)) {
            return false;
        }
    }

    std::filesystem::remove(path);
    fs_.removeProject(fileId);

    for (auto* m : projectManagers_) {
        if (m) {
            m->deleteTabSlot(fileId);
        }
    }
    currentManager->addNotification("🗑️ File removed: " + pd.name);
    return true;
}

bool QtUIController::closeFile(UI::ProjectManager* currentManager, const qint64 fileId) {
    if (!currentManager) {
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        currentManager->closeTabSlot(fileId);
        return false;
    }

    DocumentId docId = docController_.idFromPath(pd.path.toStdString());
    if (docId != DocumentController::InvalidId) {
        if (closeOneDocument(docId, currentManager)) {
            currentManager->addNotification("💾 File saved and closed: " + pd.name);
            return true;
        }
        return false;
    }
    currentManager->closeTabSlot(fileId);
    return true;
}

bool QtUIController::setActiveFile(UI::ProjectManager* currentManager, qint64 fileId) {
    if (!currentManager) {
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    const std::string pathStr = pd.path.toStdString();

    auto activated = docController_.activateByPath(pathStr);
    if (!activated) {
        return false;
    }

    QtViewportHost* host = managerHosts_.contains(currentManager) ? managerHosts_[currentManager].data() : nullptr;
    if (!host) {
        return false;
    }

    host->setActiveController(activated->controllerIndex);
    host->requestRedraw();

    presentActiveTool(currentManager, activated->id, activated->activeToolId);
    currentManager->setActiveTab(fileId);
    return true;
}

bool QtUIController::saveFullProject(UI::ProjectManager* currentManager, bool close) {
    if (!currentManager) {
        return false;
    }

    QtViewportHost* managerHost = managerHosts_.contains(currentManager) ? managerHosts_[currentManager].data() : nullptr;
    if (!managerHost) {
        return false;
    }

    std::vector<DocumentId> ids;
    for (const auto& [docId, host] : documentHosts_) {
        if (host == managerHost && docController_.isOpened(docId)) {
            ids.push_back(docId);
        }
    }

    if (ids.empty()) {
        currentManager->addNotification("💾 No open files to save.");
        return false;
    }

    int count = 0;
    bool allSuccess = true;

    for (DocumentId id : ids) {
        if (close) {
            if (!closeOneDocument(id, currentManager)) {
                allSuccess = false;
            } else {
                count++;
            }
        } else {
            if (docController_.saveDocument(id)) {
                count++;
            } else {
                allSuccess = false;
            }
        }
    }

    if (count > 0) {
        currentManager->addNotification(close ? QStringLiteral("💾 Saved and closed %1 file(s)").arg(count) : QStringLiteral("💾 Saved %1 file(s)").arg(count));
    }
    return allSuccess;
}

bool QtUIController::lightEditorRequested(UI::ProjectManager* currentManager, const qint64 id) {
    if (!currentManager) {
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(id, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    DocumentId docId = docController_.idFromPath(pd.path.toStdString());
    if (docId == DocumentController::InvalidId) {
        return false;
    }

    auto ctx = createEditorContext(true);
    auto* lightManager = createLightManager(ctx);

    currentManager->closeTabSlot(id);
    closeDocumentView(docId);

    if (!attachDocument(*lightManager, docId, id)) {
        return false;
    }

    UI::FileData fd{pd.id, pd.name};
    lightManager->showLightEditorWindow(fd);

    lightManager->addNotification("Opened light editor: " + pd.name);
    return true;
}

bool QtUIController::returnTabToEditorRequested(UI::ProjectManager* currentManager, qint64 fileId) {
    if (!currentManager) {
        return false;
    }
    bool flag = true;
    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    DocumentId docId = docController_.idFromPath(pd.path.toStdString());
    if (docId == DocumentController::InvalidId) {
        return false;
    }

    UI::ProjectManager* targetManager = nullptr;
    for (auto* pm : projectManagers_) {
        if (!pm->isLightEditor() && pm != currentManager) {
            targetManager = pm;
            break;
        }
    }

    bool isNew = (targetManager == nullptr);
    if (isNew) {
        targetManager = createBaseManager(createEditorContext(true));
    }

    flag = closeDocumentView(docId);

    if (!attachDocument(*targetManager, docId, fileId)) {
        return false;
    }

    UI::FileData fd{pd.id, pd.name};
    if (isNew) {
        targetManager->showEditorWindow(fd);
    } else {
        targetManager->addTabSlot(fd);
    }

    targetManager->addNotification("Returned tab from light editor: " + pd.name);
    return flag;
}

bool QtUIController::handleTabMove(qint64 fileId, quintptr sourceWindowPtr, quintptr targetWindowPtr) {
    UI::ProjectManager* sourceManager = nullptr;
    UI::ProjectManager* targetManager = nullptr;

    for (auto* pm : projectManagers_) {
        if (!pm) {
            continue;
        }
        QWidget* win = pm->windowWidget();
        if (!win) {
            continue;
        }
        const quintptr windowPtr = reinterpret_cast<quintptr>(win);
        if (windowPtr == sourceWindowPtr) {
            sourceManager = pm;
        }
        if (windowPtr == targetWindowPtr) {
            targetManager = pm;
        }
    }

    if (!sourceManager || !targetManager || sourceManager == targetManager) {
        return false;
    }

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(fileId, pd) != UI::FileSystem::FsResult::Ok) {
        return false;
    }

    DocumentId docId = docController_.idFromPath(pd.path.toStdString());
    if (docId == DocumentController::InvalidId) {
        return false;
    }

    closeDocumentView(docId);
    sourceManager->closeTabSlot(fileId);

    if (!attachDocument(*targetManager, docId, fileId)) {
        return false;
    }

    UI::FileData fd{pd.id, pd.name};
    targetManager->addTabSlot(fd);
    return true;
}

bool QtUIController::closeLightEditorWindowTriggered(UI::ProjectManager* currentManager) {
    if (!currentManager) {
        return false;
    }

    qint64 id = currentManager->getActiveTabId();
    if (id == -1) {
        return false;
    }

    QString path = getFilePath(id);
    if (path.isEmpty()) {
        return false;
    }

    DocumentId docId = docController_.idFromPath(path.toStdString());
    if (docId != DocumentController::InvalidId) {
        docController_.saveDocument(docId);
        closeDocumentView(docId);
        docController_.closeDocument(docId);
    }

    managerHosts_.erase(currentManager);
    return true;
}

bool QtUIController::attachDocument(UI::ProjectManager& currentManager, DocumentId id, qint64 fileID) {
    QtViewportHost* host = managerHosts_.contains(&currentManager) ? managerHosts_[&currentManager].data() : nullptr;
    if (!host) {
        return false;
    }

    Document* doc = docController_.manager().at(id);
    if (!doc) {
        return false;
    }

    auto view = std::make_unique<DocumentView>(*doc);
    view->editorSession_.setToolChangedCallback([this, id, m = &currentManager](ToolId tool) {
        if (m) {
            presentActiveTool(m, id, tool);
        }
    });

    const int index = host->addController(&view->viewportController_);
    if (index < 0) {
        return false;
    }

    docController_.registerView(std::move(view), index);
    documentHosts_[id] = host;

    if (!setActiveFile(&currentManager, fileID)) {
        closeDocumentView(id);
        return false;
    }
    return true;
}

bool QtUIController::closeDocumentView(DocumentId id) {
    QtViewportHost* host = documentHosts_.contains(id) ? documentHosts_[id].data() : nullptr;

    int removedIndex = -1;
    auto view = docController_.extractView(id, &removedIndex);
    if (!view) {
        documentHosts_.erase(id);
        return false;
    }

    if (host && removedIndex >= 0) {
        host->removeController(&view->viewportController_);
    }

    documentHosts_.erase(id);
    return true;
}

bool QtUIController::closeOneDocument(DocumentId docId, UI::ProjectManager* currentManager) {
    if (!docController_.isOpened(docId)) {
        return false;
    }
    if (!docController_.saveDocument(docId)) {
        return false;
    }

    const std::string path = docController_.pathOf(docId);
    const std::size_t key = docController_.pathHash(path);

    closeDocumentView(docId);
    if (!docController_.closeDocument(docId)) {
        return false;
    }

    docController_.clearActiveTabHashIf(key);

    qint64 fileId = -1;
    if (fs_.getProjectIdByPath(QString::fromStdString(path), fileId) == UI::FileSystem::FsResult::Ok) {
        currentManager->closeTabSlot(fileId);
    }
    return true;
}

void QtUIController::initBaseSignals(UI::ProjectManager& manager) {
    using PM = UI::ProjectManager;

    connect(&manager, &PM::createFileTriggered, this, [this, &manager](const QString& path, bool newWindow) { createFile(&manager, path, newWindow); });
    connect(&manager, &PM::openFileTriggered, this, [this, &manager](const QString& path, bool newWindow) { openFile(&manager, path, newWindow); });
    connect(&manager, &PM::openFileByIDTriggered, this, [this, &manager](qint64 fileId, bool newWindow) { openFileByID(&manager, fileId, newWindow); });
    connect(&manager, &PM::renameFileTriggered, this, [this, &manager](UI::FileData& data) { renameFile(&manager, data); });
    connect(&manager, &PM::removeFileTriggered, this, [this, &manager](qint64 fileId) { removeFile(&manager, fileId); });
    connect(&manager, &PM::closeFileTriggered, this, [this, &manager](qint64 fileId) { closeFile(&manager, fileId); });
    connect(&manager, &PM::setActiveFileTriggered, this, [this, &manager](qint64 fileId) { setActiveFile(&manager, fileId); });

    connect(&manager, &PM::lightEditorRequested, this, [this, &manager](qint64 fileId) { lightEditorRequested(&manager, fileId); });
    connect(&manager, &PM::tabMovedToAnotherWindow, this,
            [this](qint64 fileId, quintptr sourceWindowPtr, quintptr targetWindowPtr) { handleTabMove(fileId, sourceWindowPtr, targetWindowPtr); });

    connect(&manager, &PM::saveTriggered, this, [this, &manager]() { saveFullProject(&manager, false); });
    connect(&manager, &PM::windowClosed, this, [this, &manager]() { saveFullProject(&manager, true); });

    connect(&manager, &PM::sentCommandTriggered, this, [this, &manager](qint64 fileId, const QString& command) { sentCommand(&manager, fileId, command); });
    connect(&manager, &PM::primitiveTriggered, this, [this, &manager](qint64 fileId, UI::PrimitiveType type) { primitive(&manager, fileId, type); });
    connect(&manager, &PM::constraintTriggered, this, [this, &manager](qint64 fileId, UI::ConstraintType type) { constraint(&manager, fileId, type); });
    connect(&manager, &PM::toolsTriggered, this, [this, &manager](qint64 fileId, UI::ToolsType type) { tools(&manager, fileId, type); });
    connect(&manager, &PM::scriptTriggered, this, [this, &manager](const QString& path) { script(&manager, path); });
    connect(&manager, &PM::settingsChanged, this, [this, &manager]() { settingsChanged(&manager); });

    connect(&manager, &QObject::destroyed, this, [this, &manager] { deleteManager(&manager); });
}

void QtUIController::initLightManager(UI::ProjectManager& manager) {
    using PM = UI::ProjectManager;

    connect(&manager, &PM::returnTabToEditorRequested, this, [this, &manager](qint64 fileId) { returnTabToEditorRequested(&manager, fileId); });
    connect(&manager, &PM::windowClosed, this, [this, &manager]() { closeLightEditorWindowTriggered(&manager); });

    connect(&manager, &PM::saveTriggered, this, [this, &manager]() { saveFullProject(&manager, false); });
    connect(&manager, &PM::sentCommandTriggered, this, [this, &manager](qint64 fileId, const QString& command) { sentCommand(&manager, fileId, command); });
    connect(&manager, &PM::primitiveTriggered, this, [this, &manager](qint64 fileId, UI::PrimitiveType type) { primitive(&manager, fileId, type); });
    connect(&manager, &PM::constraintTriggered, this, [this, &manager](qint64 fileId, UI::ConstraintType type) { constraint(&manager, fileId, type); });
    connect(&manager, &PM::toolsTriggered, this, [this, &manager](qint64 fileId, UI::ToolsType type) { tools(&manager, fileId, type); });
    connect(&manager, &PM::scriptTriggered, this, [this, &manager](const QString& path) { script(&manager, path); });
    connect(&manager, &PM::settingsChanged, this, [this, &manager]() { settingsChanged(&manager); });

    connect(&manager, &QObject::destroyed, this, [this, &manager] { deleteManager(&manager); });
}

void QtUIController::deleteManager(UI::ProjectManager* manager) {
    auto it = managerHosts_.find(manager);
    if (it != managerHosts_.end()) {
        for (auto mapIt = documentHosts_.begin(); mapIt != documentHosts_.end();) {
            if (mapIt->second.isNull()) {
                mapIt = documentHosts_.erase(mapIt);
            } else {
                ++mapIt;
            }
        }
        managerHosts_.erase(it);
    }
    projectManagers_.removeOne(manager);
}

void QtUIController::openFilesFromCommandLine(const QStringList& filePaths) {
    if (filePaths.isEmpty()) {
        return;
    }

    UI::ProjectManager* targetManager = nullptr;
    for (auto* pm : projectManagers_) {
        if (pm && !pm->isLightEditor()) {
            targetManager = pm;
            break;
        }
    }

    if (!targetManager) {
        targetManager = createBaseManager(createEditorContext(true));
    }

    for (const QString& path : filePaths) {
        if (!path.trimmed().isEmpty()) {
            openFile(targetManager, path, false);
        }
    }
}

void QtUIController::sentCommand(UI::ProjectManager* currentManager, const qint64 id, const QString command) {
    if (currentManager) {
        currentManager->addNotification(id, "Console command received: " + command);
    }
}

void QtUIController::primitive(UI::ProjectManager* currentManager, const qint64 fileId, UI::PrimitiveType type) {
    const std::string pathStr = getFilePath(fileId).toStdString();
    switch (type) {
        case UI::PrimitiveType::Point:
            selectTool(ToolId::Point, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::CubicBezier:
            selectTool(ToolId::CubicBezier, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::Line:
            selectTool(ToolId::Line, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::Polyline:
            selectTool(ToolId::Polyline, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::InfiniteLine:
            selectTool(ToolId::InfiniteLine, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::CircleByRadius:
            selectTool(ToolId::CircleByRadius, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::CircleByDiameter:
            selectTool(ToolId::CircleByDiameter, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::CircleByTwoPoints:
            selectTool(ToolId::CircleByTwoPoints, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::CircleByThreePoints:
            selectTool(ToolId::CircleByThreePoints, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::CircleTangentTwoLines:
            selectTool(ToolId::CircleTangentTwoLines, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::CircleTangentThreeLines:
            selectTool(ToolId::CircleTangentThreeLines, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::ArcByRadius:
            selectTool(ToolId::ArcByRadius, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::ArcByDiameter:
            selectTool(ToolId::ArcByDiameter, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::ArcThreePoints:
            selectTool(ToolId::ArcByThreePoints, std::nullopt, pathStr);
            break;
        case UI::PrimitiveType::ArcSettings:
            selectTool(ToolId::ArcByThreePoints, std::nullopt, pathStr);
            break;
        default:
            break;
    }
    if (currentManager) {
        currentManager->addNotification(fileId, "Primitive selected");
    }
}

void QtUIController::constraint(UI::ProjectManager* currentManager, const qint64 fileId, UI::ConstraintType type) {
    const std::string pathStr = getFilePath(fileId).toStdString();
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
            } else {
                constraintType = ToolId::Distance;
                promptText = tr("Distance:");
            }
            showConstraintPrompt(promptText, constraintType, pathStr);
            break;
        }
        case UI::ConstraintType::Tangent:
            selectTool(ToolId::Tangent, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Coincident:
            selectTool(ToolId::Coincident, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Horizontal:
            selectTool(ToolId::Horizontal, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Vertical:
            selectTool(ToolId::Vertical, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Parallel:
            selectTool(ToolId::Parallel, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Perpendicular:
            selectTool(ToolId::Perpendicular, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Equal:
            selectTool(ToolId::Equal, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::FixUnfix:
            selectTool(ToolId::Fix, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Concentric:
            selectTool(ToolId::Concentric, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Collinear:
            selectTool(ToolId::Collinear, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Midpoint:
            selectTool(ToolId::Midpoint, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Symmetric:
            selectTool(ToolId::Symmetric, std::nullopt, pathStr);
            break;
        case UI::ConstraintType::Curvature:
            selectTool(ToolId::Curvature, std::nullopt, pathStr);
            break;
        default:
            return;
    }
    if (currentManager) {
        currentManager->addNotification(fileId, "Constraint selected");
    }
}

void QtUIController::tools(UI::ProjectManager* currentManager, const qint64 id, UI::ToolsType type) {
    if (currentManager) {
        currentManager->addNotification(id, "Tool selected");
    }
}

void QtUIController::script(UI::ProjectManager* currentManager, const QString& text) {
    if (currentManager) {
        currentManager->addNotification("Script: " + text);
    }
}

void QtUIController::settingsChanged(UI::ProjectManager* currentManager) {
    const auto d = ui_settings_.loadChanges();
    if (d.isEmpty()) return;

    bool anyApplied = false;

    if (d.sketch.gridEnabled && app_settings_.get_grid() != *d.sketch.gridEnabled) {
        app_settings_.set_grid(*d.sketch.gridEnabled);
        anyApplied = true;
    }

    if (d.sketch.snapEnabled && app_settings_.get_osnap() != *d.sketch.snapEnabled) {
        app_settings_.set_osnap(*d.sketch.snapEnabled);
        anyApplied = true;
    }

    if (d.sketch.gridStep && app_settings_.get_step_grid() != *d.sketch.gridStep) {
        app_settings_.set_step_grid(*d.sketch.gridStep);
        anyApplied = true;
    }

    if (d.sketch.snapStep && app_settings_.get_snap_snap() != *d.sketch.snapStep) {
        app_settings_.set_snap_snap(*d.sketch.snapStep);
        anyApplied = true;
    }

    if (!anyApplied) {
        return;
    }
}

void QtUIController::applyLanguage(UI::SettingsSystem::Language lang) {
    if (translator_) {
        qApp->removeTranslator(translator_);
        delete translator_;
        translator_ = nullptr;
    }
    currentLanguage_ = lang;
    if (lang == UI::SettingsSystem::Language::English) {
        return;
    }

    const QString path =
        (lang == UI::SettingsSystem::Language::Russian) ? QStringLiteral(":/translations/app_ru.qm") : QStringLiteral(":/translations/app_by.qm");

    auto* tr = new QTranslator();
    if (tr->load(path)) {
        qApp->installTranslator(tr);
        translator_ = tr;
    } else {
        delete tr;
    }
}

void QtUIController::presentActiveTool(UI::ProjectManager* currentManager, DocumentId id, ToolId tool) {
    if (!currentManager) {
        return;
    }

    const auto present = [currentManager](auto uiTool) { currentManager->setActiveTool(uiTool); };

    switch (tool) {
        case ToolId::Cursor:
            present(UI::ToolsType::None);
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
        case ToolId::CubicBezier:
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
        case ToolId::Fix:
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
        case ToolId::Symmetric:
            present(UI::ConstraintType::Symmetric);
            break;
        case ToolId::Curvature:
            present(UI::ConstraintType::Curvature);
            break;
        default:
            break;
    }
}

void QtUIController::selectTool(ToolId tool, std::optional<double> value, const std::string& path) {
    std::optional<DocumentId> id;
    if (!path.empty()) {
        id = docController_.idFromPath(path);
    }

    const DocumentId selected = docController_.selectTool(id, tool, value);
    if (selected == DocumentController::InvalidId) {
        return;
    }

    QtViewportHost* host = activeHost_.data();
    if (documentHosts_.contains(selected)) {
        host = documentHosts_[selected].data();
    }
    if (host) {
        host->requestRedraw();
    }
}

void QtUIController::showConstraintPrompt(const QString& promptText, ToolId constraintType, std::string path) {
    auto* prompt = new UI::ParameterInputWidget(promptText, nullptr);
    QSize size = prompt->sizeHint();
    if (size.isEmpty()) {
        size = prompt->size();
    }

    QPoint centerGlobal;
    if (QWidget* ref = QApplication::activeWindow()) {
        centerGlobal = ref->mapToGlobal(ref->rect().center());
    } else if (QWidget* ref = qobject_cast<QWidget*>(parent())) {
        centerGlobal = ref->mapToGlobal(ref->rect().center());
    } else {
        centerGlobal = QGuiApplication::primaryScreen()->availableGeometry().center();
    }

    QPoint target(centerGlobal.x() - size.width() / 2, centerGlobal.y() - size.height() / 2);
    QScreen* screen = QGuiApplication::screenAt(centerGlobal);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    const QRect avail = screen->availableGeometry();

    int x = target.x();
    int y = target.y();
    if (x + size.width() > avail.right()) {
        x = avail.right() - size.width();
    }
    if (y + size.height() > avail.bottom()) {
        y = avail.bottom() - size.height();
    }
    if (x < avail.left()) {
        x = avail.left();
    }
    if (y < avail.top()) {
        y = avail.top();
    }

    prompt->move(x, y);

    connect(prompt, &UI::ParameterInputWidget::inputEnteredTriggered, this, [this, path = std::move(path), constraintType](const QString& parameter) {
        bool valid = false;
        const double value = parameter.toDouble(&valid);
        const double scale = (constraintType == ToolId::Angle) ? std::numbers::pi / 180.0 : 1.0;
        selectTool(constraintType, valid ? value * scale : std::numeric_limits<double>::quiet_NaN(), path);
    });

    prompt->show();
    prompt->raise();
    prompt->activateWindow();
}