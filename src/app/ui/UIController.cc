#include "UIController.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QScreen>
#include <QTranslator>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "../../core/Document.h"
#include "../../core/DocumentManager.h"
#include "../editor/CommandConsole.h"
#include "../platform/IViewportHost.h"
#include "../platform/QtViewportHost.h"
#include "../viewport/AxisTexts.h"
#include "../viewport/ViewportController.h"
#include "../viewport/render/RenderSceneBuilder.h"
#include "App/CustomConsole.h"
#include "App/RenderEngine.h"
#include "OpenGL2dRenderer.h"
#include "ParameterInputWidget.h"
#include "SettingsSystem.h"
#include "ToolsType.h"



UIController::UIController(std::vector<std::unique_ptr<DocumentView>>& views,
    DocumentManager& manager,
    IPlatformRuntime& platformRuntime )
    : views_(views),
      manager_(manager),
      platformRuntime_(platformRuntime),
      settings_(fs_.settingsPath()) {

    activeHost_ = static_cast<QtViewportHost*>(platformRuntime_.createViewportHost());

    auto ctx = createEditorContext(false);
    createManager(ctx);

    currentLanguage_ = settings_.language();
    applyLanguage(currentLanguage_);
}

UIController::~UIController() {
    for (auto& view : views_) {
        if (!view) continue;
        view->editorSession_.setToolChangedCallback({});
        if (activeHost_) activeHost_->removeController(&view->viewportController_);
    }
    views_.clear();
}


void UIController::createFile(UI::ProjectManager* currentManager,
                              const QString& path, bool newWindow) {
    UI::ProjectManager* targetManager = currentManager;
    if (newWindow || !targetManager) {
        targetManager = createManager(createEditorContext(true));
    }

    qint64 fileId = -1;
    const auto res = fs_.createProject(path, fileId);

    switch (res) {
        case UI::FileSystem::FsResult::Ok:
            break;
        case UI::FileSystem::FsResult::AlreadyExists:
            targetManager->addNotification("❌ File already exists: " + path);
            return;
        case UI::FileSystem::FsResult::InvalidArgument:
            targetManager->addNotification("❌ Invalid file name: " + path);
            return;
        default:
            targetManager->addNotification("❌ Failed to create file: " + path);
            return;
    }

    UI::FileSystem::ProjectData data = getProjectData(fileId);
    if (data.name.isEmpty()) return;

    const std::string fileName = data.name.toStdString();
    const std::string pathStr  = data.path.toStdString();
    const std::size_t key      = pathHash(pathStr);

    if (documentIds_.contains(key)) {
        setActiveFile(targetManager, fileId);
        return;
    }

    if (auto* previous = findViewByHash(activeTabHash_)) {
        previous->editorSession_.activeTool()->cancel();
    }

    const auto id = manager_.createNewDocument(fileName);
    try {
        if (auto* doc = manager_.at(id)) {
            doc->path() = pathStr;
        }

        documentIds_[key]  = id;

        attachDocument(*targetManager, id, data.id);
        projectModified_ = true;

        UI::FileData fd{data.id, data.name};
        targetManager->addTabInEditor(fd);

        if (newWindow) {
            targetManager->addNotification("✨ File created in new window: " + data.name);
        } else {
            targetManager->addNotification("✨ File created: " + data.name);
        }
    } catch (...) {
        documentIds_.erase(key);
        manager_.closeDocument(id);
        throw;
    }
}

void UIController::openFile(UI::ProjectManager* currentManager, const QString& path, bool newWindow) {
    if (path.trimmed().isEmpty()) return;
    if (!currentManager) return;

    qint64 fileId = -1;
    const auto res = fs_.registerProject(path, fileId);
    if (res != UI::FileSystem::FsResult::Ok && res != UI::FileSystem::FsResult::AlreadyExists) {
        currentManager->addNotification("❌ Failed to open file: " + QFileInfo(path).fileName());
        return;
    }

    openFileByID(currentManager, fileId, newWindow);
}

void UIController::openFileByID(UI::ProjectManager* currentManager, qint64 fileId, bool newWindow) {
    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) {
        if (currentManager) {
            currentManager->addNotification("❌ File not found: id " + QString::number(fileId));
        }
        return;
    }

    UI::ProjectManager* targetManager = currentManager;
    if (newWindow || !targetManager) {
        targetManager = createManager(createEditorContext(true));
    }

    const bool loaded = loadDocument(data.path, targetManager);
    if (!loaded) {
        targetManager->addNotification("❌ File not opened: " + data.name);
        return;
    }

    UI::FileData fd{data.id, data.name};
    targetManager->addTabInEditor(fd);

    const QString where = newWindow ? " in new window" : "";
    targetManager->addNotification("📄 File opened" + where + ": " + data.name);
}

void UIController::renameFile(UI::ProjectManager* currentManager, UI::FileData& data) {
    if (!currentManager) return;

    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(data.id, pd) != UI::FileSystem::FsResult::Ok) {
        currentManager->addNotification("❌ File not found: id " + QString::number(data.id));
        return;
    }

    const std::string newName = data.name.toStdString();
    if (newName.empty() || pd.name.toStdString() == newName) return;

    const std::string oldPathStr = pd.path.toStdString();
    const std::filesystem::path oldPath(oldPathStr);
    const std::filesystem::path newPath = oldPath.parent_path() / (newName + oldPath.extension().string());
    const std::string newPathStr = newPath.string();

    const std::size_t oldKey = pathHash(oldPathStr);
    const std::size_t newKey = pathHash(newPathStr);

    if (documentIds_.contains(newKey)) {
        currentManager->addNotification("❌ File with this name is already open: " + data.name);
        return;
    }

    if (fs_.renameProject(data.id, data.name) != UI::FileSystem::FsResult::Ok) {
        currentManager->addNotification("❌ Failed to rename: " + pd.name);
        return;
    }


        for (auto* m : projectManagers_) {
            if (m) {
                m->renameTabSlot(data);
            }
        }


    const auto it = documentIds_.find(oldKey);
    if (it != documentIds_.end()) {
        const DocumentId docId = it->second;
        documentIds_.erase(it);
        documentIds_[newKey] = docId;
    }

    if (auto* view = findView(oldPathStr)) {
        auto idxIt = indicesMap_.find(oldKey);
        if (idxIt != indicesMap_.end()) {
            const int index = idxIt->second;
            indicesMap_.erase(idxIt);
            indicesMap_[newKey] = index;
        }

        view->document().name() = newName;
        view->document().path() = newPathStr;
    }

    if (activeTabHash_ == oldKey) {
        activeTabHash_ = newKey;
    }

    projectModified_ = true;
}

void UIController::removeFile(UI::ProjectManager* currentManager, const qint64 fileId) {
    if (!currentManager) return;

    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) {
        currentManager->addNotification("❌ File not found: id " + QString::number(fileId));
        return;
    }

    const QString name = data.name;
    const std::string pathStr = data.path.toStdString();
    const std::size_t key = pathHash(pathStr);

    if (fs_.removeProject(fileId) != UI::FileSystem::FsResult::Ok) {
        currentManager->addNotification("❌ Failed to remove file: " + name);
        return;
    }

    const auto idIt = documentIds_.find(key);
    if (idIt != documentIds_.end()) {
        documentIds_.erase(idIt);
    }
    QFile::remove(data.path);

    for (auto* m : projectManagers_) {
        if (m) m->deleteTabSlot(fileId);
    }

    currentManager->addNotification("🗑️ File removed: " + name);

    const auto* target = findView(pathStr);
    if (target) {
        for (auto it = views_.begin(); it != views_.end(); ++it) {
            if (it->get() == target) {
                (*it)->editorSession_.setToolChangedCallback({});

                const auto removed = controllerMap_.find(&(*it)->viewportController_);
                const int removedIndex = (removed != controllerMap_.end()) ? removed->second : -1;

                DocumentId docId = idFromPath(pathStr);
                QtViewportHost* host = documentHosts_.contains(docId) ? documentHosts_[docId] : activeHost_;
                if (host) {
                    host->removeController(&(*it)->viewportController_);
                }

                controllerMap_.erase(&(*it)->viewportController_);
                views_.erase(it);
                indicesMap_.erase(key);

                if (removedIndex >= 0) {
                    for (auto& [n, index] : indicesMap_) {
                        if (index > removedIndex) --index;
                    }
                    for (auto& [ctrl, index] : controllerMap_) {
                        if (index > removedIndex) --index;
                    }
                }
                break;
            }
        }
    }

    if (activeTabHash_ == key) {
        if (indicesMap_.empty()) {
            activeTabHash_ = 0;
        } else {
            const std::size_t nextKey = indicesMap_.begin()->first;
            if (auto* nextView = findViewByHash(nextKey)) {
                const std::string nextPath = nextView->document().path();
                qint64 nextFileId = -1;

                if (fs_.getProjectIdByPath(QString::fromStdString(nextPath), nextFileId) == UI::FileSystem::FsResult::Ok) {
                    setActiveFile(currentManager, nextFileId);
                }
            }
        }
    }
}

void UIController::closeFile(UI::ProjectManager* currentManager, const qint64 fileId) {
    if (!currentManager) return;

    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) {
        currentManager->closeTabSlot(fileId);
        return;
    }

    const std::string pathStr = data.path.toStdString();
    const std::size_t key = pathHash(pathStr);

    const auto it = documentIds_.find(key);

    if (it != documentIds_.end()) {
        DocumentId docId = it->second;
        closeOneDocument(docId,currentManager);
        currentManager->addNotification("💾 File saved and closed: " + data.name);
    } else {
        currentManager->closeTabSlot(fileId);
    }
}

void UIController::setActiveFile(UI::ProjectManager* currentManager, const qint64 fileId) {
    if (!currentManager) return;

    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) return;

    const std::string pathStr = data.path.toStdString();
    const std::size_t newHash = pathHash(pathStr);

    auto* view = findView(pathStr);
    if (!view) return;

    if (activeTabHash_ != newHash) {
        if (auto* previous = findViewByHash(activeTabHash_)) {
            previous->editorSession_.activeTool()->cancel();
        }
    }

    activeTabHash_ = newHash;

    QtViewportHost* host = managerHosts_.contains(currentManager) ? managerHosts_[currentManager] : activeHost_;

    auto indexIt = indicesMap_.find(newHash);
    if (indexIt != indicesMap_.end() && host) {
        host->setActiveController(indexIt->second);
    }

    const auto idIt = documentIds_.find(newHash);
    if (idIt != documentIds_.end()) {
        presentActiveTool(currentManager, idIt->second, view->editorSession_.activeToolId());
    }
// <-- Вызываем у правильного хоста
    currentManager->setActiveTab(fileId);
}

void UIController::sentCommand(UI::ProjectManager* currentManager, const qint64 fileId, const QString command) {
    currentManager->addNotification(fileId, "Console command received: " + command);
    try {
        // const auto entry = documentIds_.find(activeTabName_);
        // Document* document = entry == documentIds_.end() ? nullptr : manager_.at(entry->second);
        // UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
        // CommandManager& cm = document->commandManager();
        // Transaction* txn = cm.invoke(str);
        // urm.push(std::move(*txn));

        // if (document) {
        //     viewportHost_.requestRedraw();
        // }
    } catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

void UIController::primitive(UI::ProjectManager* currentManager, const qint64 fileId, UI::PrimitiveType type) {
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
        case UI::PrimitiveType::LineSettings:
            // selectTool(ToolId::);
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

    currentManager->addNotification(fileId, "Primitive selected for file " + QString::number(fileId));
}

void UIController::constraint(UI::ProjectManager* currentManager, const qint64 fileId, UI::ConstraintType type) {
    const std::string pathStr = getFilePath(fileId).toStdString();

    switch (type) {
        case UI::ConstraintType::Distance:
        case UI::ConstraintType::VDistance:
        case UI::ConstraintType::HDistance:
        case UI::ConstraintType::Angle: {
            ToolId  constraintType;
            QString promptText;
            if (type == UI::ConstraintType::Angle) {
                constraintType = ToolId::Angle;
                promptText     = tr("Angle (degrees):");
            } else if (type == UI::ConstraintType::VDistance) {
                constraintType = ToolId::VerticalDistance;
                promptText     = tr("Vertical Distance:");
            } else if (type == UI::ConstraintType::HDistance) {
                constraintType = ToolId::HorizontalDistance;
                promptText     = tr("Horizontal Distance:");
            } else { // Distance
                constraintType = ToolId::Distance;
                promptText     = tr("Distance:");
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

    currentManager->addNotification(fileId, "Constraint selected for file " + QString::number(fileId));
}

void UIController::tools(UI::ProjectManager* currentManager, const qint64 fileId, UI::ToolsType type) {
    currentManager->addNotification(fileId, "Tool selected for file " + QString::number(fileId));
    switch (type) {
        case UI::ToolsType::None:
        case UI::ToolsType::Size:
            break;
            default:
            break;
    }
}

void UIController::script(UI::ProjectManager* currentManager, const QString& path) { currentManager->addNotification("Script: " + path); }

void UIController::settingsChanged(UI::ProjectManager* currentManager) {
    const auto d = settings_.loadChanges();
    if (d.isEmpty()) {
        return;
    }

    if (!d.sketch.isEmpty()) {
        qDebug() << "[settings] sketch changed:" << (d.sketch.gridEnabled ? *d.sketch.gridEnabled : -1) << (d.sketch.snapEnabled ? *d.sketch.snapEnabled : -1)
                 << (d.sketch.gridStep ? *d.sketch.gridStep : -1) << (d.sketch.snapStep ? *d.sketch.snapStep : -1);
    }

    if (d.language) {
        applyLanguage(*d.language);
    }
}

void UIController::saveFullProject(UI::ProjectManager* currentManager) {
    if (!currentManager) return;

    std::vector<DocumentId> ids;
    ids.reserve(documentIds_.size());
    for (const auto& [hash, id] : documentIds_) {
        ids.push_back(id);
    }

    if (ids.empty()) {
        currentManager->addNotification(QStringLiteral("💾 No open files to save."));
        return;
    }

    int savedCount = 0;
    for (DocumentId id : ids) {
        closeOneDocument(id,currentManager);
        savedCount++;
    }

    currentManager->addNotification(
        QStringLiteral("💾 Saved and closed %1 file(s)").arg(savedCount));
}

void UIController::lightEditorReturned(UI::ProjectManager* currentManager,const qint64 fileId) {
    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) {
        return;
    }

    UI::FileData fd{data.id, data.name};
    currentManager->addTabInEditor(fd);
    currentManager->addNotification("Returned tab from light editor: " + data.name);
}

void UIController::lightEditorRequested(UI::ProjectManager* currentManager,const qint64 id) {
    UI::FileSystem::ProjectData pd;
    if (fs_.getProject(id, pd) != UI::FileSystem::FsResult::Ok) {
        return;
    }
    UI::FileData fd{pd.id, pd.name};
    openLightEditor(currentManager,fd);
}

void UIController::openLightEditor(UI::ProjectManager* currentManager,UI::FileData& data) {
    auto ctx = createEditorContext(true);
    currentManager->openLightEditor(ctx, data);

    currentManager->addNotification("Opened light editor: " + data.name);
}

qint64 UIController::getFileID(const QString& filePath) {
    UI::FileSystem::ProjectData data;
    qint64 id = -1;
    if (fs_.getProjectIdByPath(filePath, id) != UI::FileSystem::FsResult::Ok) {
        return -1;
    }
    return id;
}

QString UIController::getFilePath(const qint64 fileId) {
    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) {
        return{};
    }
    return data.path;
}

QString UIController::getFileName(const qint64 fileId) {
    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) {
        return{};
    }
    return data.name;
}

UI::FileSystem::ProjectData UIController::getProjectData(const qint64 fileId) {
    UI::FileSystem::ProjectData data;
    if (fs_.getProject(fileId, data) != UI::FileSystem::FsResult::Ok) {
        return{};
    }
    return data;
}

UI::ProjectManager* UIController::createManager(const UI::EditorContext& context) {
    auto* manager = new UI::ProjectManager(context);
    projectManagers_.push_back(manager);

    managerHosts_[manager] = static_cast<QtViewportHost*>(context.renderEngine);

    initUISignals(*manager);
    return manager;
}

UI::EditorContext UIController::createEditorContext(bool isNewWindow) const {
    UI::EditorContext ctx;
    ctx.fileSystem = const_cast<UI::FileSystem*>(&fs_);
    ctx.settings = const_cast<UI::SettingsSystem*>(&settings_);

    if (isNewWindow) {
        auto* hostInterface = platformRuntime_.createViewportHost();
        auto* qtHost = static_cast<QtViewportHost*>(hostInterface);
        ctx.renderEngine = static_cast<QWindow*>(qtHost);
    } else {
        ctx.renderEngine = static_cast<QWindow*>(activeHost_);
    }

    ctx.consoleEngine = nullptr;
    return ctx;
}

void UIController::applyLanguage(UI::SettingsSystem::Language lang) {
    if (translator_) {
        qApp->removeTranslator(translator_);
        delete translator_;
        translator_ = nullptr;
    }

    currentLanguage_ = lang;

    if (lang == UI::SettingsSystem::Language::English) {
        qDebug() << "[UIManager] Language set to English (base)";
        return;
    }

    const QString path =
        (lang == UI::SettingsSystem::Language::Russian) ? QStringLiteral(":/translations/app_ru.qm") : QStringLiteral(":/translations/app_by.qm");

    auto* tr = new QTranslator();
    if (tr->load(path)) {
        qApp->installTranslator(tr);
        translator_ = tr;
        qDebug() << "[UIManager] Change translation:" << path;
    } else {
        qWarning() << "[UIManager] Failed to load:" << path;
        delete tr;
    }
}

DocumentView* UIController::findView(const std::string& path) const {
    for (const auto& view : views_) {
        if (view->document().path() == path) {
            return view.get();
        }
    }
    return nullptr;
}

DocumentView* UIController::findViewByName(const std::string& name) const {
    for (const auto& view : views_) {
        if (view->document().name() == name) {
            return view.get();
        }
    }
    return nullptr;
}

DocumentView* UIController::findViewByHash(std::size_t hash) const {
    if (hash == 0) return nullptr;
    for (const auto& view : views_) {
        if (pathHash(view->document().path()) == hash) {
            return view.get();
        }
    }
    return nullptr;
}


void UIController::selectTool(ToolId tool, std::optional<double> value, const std::string& path) {
    DocumentView* view = nullptr;

    if (path.empty()) {
        view = findViewByHash(activeTabHash_);
    } else {
        view = findView(path);
    }

    if (view) {
        view->editorSession_.selectTool(tool, value);
        view->viewportController_.requestRedraw();

        DocumentId id = idFromPath(view->document().path());
        QtViewportHost* host = documentHosts_.contains(id) ? documentHosts_[id] : activeHost_;
        if (host) host->requestRedraw();
    }
}

void UIController::deleteManager(UI::ProjectManager* manager) {
    projectManagers_.removeOne(manager);

    if (projectManagers_.isEmpty()) {
        std::vector<DocumentId> ids;
        ids.reserve(documentIds_.size());
        for (const auto& [hash, id] : documentIds_) {
            ids.push_back(id);
        }

        for (DocumentId id : ids) {
            closeOneDocument(id,manager);
        }
    }
}

void UIController::showConstraintPrompt(const QString& promptText,
                                        ToolId         constraintType,
                                        std::string    path)
{
    auto* prompt = new UI::ParameterInputWidget(promptText, nullptr);

    QSize size = prompt->sizeHint();
    if (size.isEmpty()) size = prompt->size();

    QPoint centerGlobal;
    if (QWidget* ref = QApplication::activeWindow()) {
        centerGlobal = ref->mapToGlobal(ref->rect().center());
    } else if (QWidget* ref = qobject_cast<QWidget*>(parent())) {
        centerGlobal = ref->mapToGlobal(ref->rect().center());
    } else {
        centerGlobal = QGuiApplication::primaryScreen()->availableGeometry().center();
    }

    QPoint target(centerGlobal.x() - size.width()  / 2,
                  centerGlobal.y() - size.height() / 2);

    QScreen* screen = QGuiApplication::screenAt(centerGlobal);
    if (!screen) screen = QGuiApplication::primaryScreen();
    const QRect avail = screen->availableGeometry();

    int x = target.x();
    int y = target.y();
    if (x + size.width()  > avail.right())  x = avail.right()  - size.width();
    if (y + size.height() > avail.bottom()) y = avail.bottom() - size.height();
    if (x < avail.left()) x = avail.left();
    if (y < avail.top())  y = avail.top();

    prompt->move(x, y);

    connect(prompt, &UI::ParameterInputWidget::inputEnteredTriggered,
             this,
             [this, path = std::move(path), constraintType]
             (const QString& parameter) {
         bool valid = false;
         const double value = parameter.toDouble(&valid);
         const double scale = (constraintType == ToolId::Angle)
                                  ? std::numbers::pi / 180.0
                                  : 1.0;
         selectTool(
             constraintType,
             valid ? value * scale
                   : std::numeric_limits<double>::quiet_NaN(),
             path);
     });

    prompt->show();
    prompt->raise();
    prompt->activateWindow();
}



void UIController::presentActiveTool(UI::ProjectManager* currentManager, DocumentId id, ToolId tool) {
    if (!currentManager) return;

    const auto entry = std::find_if(documentIds_.begin(), documentIds_.end(),
        [id](const auto& item) { return item.second == id; });

    if (entry == documentIds_.end()) {
        return;
    }

    const auto present = [currentManager](auto uiTool) {
        currentManager->setActiveToolEditorWindow(uiTool);
    };

    switch (tool) {
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
    }
}


void UIController::attachDocument(UI::ProjectManager& currentManager, DocumentId id, qint64 fileID) {
    QtViewportHost* host = managerHosts_.contains(&currentManager) ? managerHosts_[&currentManager] : activeHost_;
    documentHosts_[id] = host;

    auto view = std::make_unique<DocumentView>(*manager_.at(id), *host);
    view->editorSession_.setToolChangedCallback([this, id, &currentManager](ToolId tool) {
        presentActiveTool(&currentManager, id, tool);
    });

    const int index = host->addController(&view->viewportController_);
    if (index < 0) {
        throw std::runtime_error("Could not create a viewport for the document");
    }

    auto* controller = &view->viewportController_;
    const std::size_t key = pathHash(manager_.at(id)->path());
    const auto previousHash = activeTabHash_;

    try {
        indicesMap_[key] = index;
        controllerMap_[controller] = index;
        views_.push_back(std::move(view));

        setActiveFile(&currentManager, fileID);
    } catch (...) {
        host->removeController(controller);
        auto it = std::remove_if(views_.begin(), views_.end(),
            [controller](const auto& candidate) {
                return &candidate->viewportController_ == controller;
            });
        views_.erase(it, views_.end());

        indicesMap_.erase(key);
        controllerMap_.erase(controller);
        documentHosts_.erase(id);
        activeTabHash_ = previousHash;
        throw;
    }
}


void UIController::initUISignals(UI::ProjectManager& manager) {
    using PM = UI::ProjectManager;

    connect(&manager, &PM::createFileTriggered, this,
            [this, &manager](const QString& path, bool newWindow) { createFile(&manager, path, newWindow); });
    connect(&manager, &PM::openFileTriggered, this,
            [this, &manager](const QString& path, bool newWindow) { openFile( &manager, path,newWindow); });
    connect(&manager, &PM::openFileByIDTriggered, this,
            [this, &manager](qint64 fileId, bool newWindow) { openFileByID(&manager,fileId,  newWindow); });
    connect(&manager, &PM::renameFileTriggered, this,
            [this, &manager](UI::FileData& data) { renameFile(&manager,data); });
    connect(&manager, &PM::removeFileTriggered, this,
            [this, &manager](qint64 fileId) { removeFile(&manager, fileId); });
    connect(&manager, &PM::closeFileTriggered, this,
            [this, &manager](qint64 fileId) { closeFile(&manager,fileId); });
    connect(&manager, &PM::setActiveFileTriggered, this,
            [this, &manager](qint64 fileId) { setActiveFile(&manager, fileId); });
    connect(&manager, &PM::sentCommandTriggered, this,
            [this, &manager](qint64 fileId, const QString& command) { sentCommand(&manager, fileId, command); });
    connect(&manager, &PM::primitiveTriggered, this,
            [this, &manager](qint64 fileId, UI::PrimitiveType type) {
                primitive(&manager, fileId, type);
            });
    connect(&manager, &PM::constraintTriggered, this,
            [this, &manager](qint64 fileId, UI::ConstraintType type) { constraint(&manager, fileId, type); });
    connect(&manager, &PM::toolsTriggered, this,
            [this, &manager](qint64 fileId, UI::ToolsType type) { tools(&manager, fileId, type); });
    connect(&manager, &PM::scriptTriggered, this,
            [this, &manager](const QString& path) { script(&manager, path); });
    connect(&manager, &PM::settingsChanged, this,
            [this, &manager]() { settingsChanged(&manager); });
    connect(&manager, &PM::saveFullProjectTriggered, this,
            [this, &manager]() { saveFullProject(&manager); });
    connect(&manager, &PM::lightEditorReturned, this,
            [this, &manager](qint64 fileId) { lightEditorReturned(&manager, fileId); });
    connect(&manager, &PM::lightEditorRequested, this,
            [this, &manager](qint64 fileId) { lightEditorRequested(&manager, fileId); });

    connect(&manager, &QObject::destroyed, this,
            [this, &manager] {
                deleteManager(&manager);
            });
}


#include "NativeFile.h"
#include "QtPath.h"
#include "UIController.h"
#include <QFile>
#include <QFileInfo>

#include "DocumentSaveLoad.h"
#include <filesystem>
#include <limits>

namespace {

bool writeAllBytes(const std::filesystem::path& nativePath,
                   const core::persistence::DocumentBytes& bytes) {
    QFile file(app::ui::pathText(nativePath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    if (bytes.empty()) {
        return true;
    }
    const qint64 written = file.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<qint64>(bytes.size()));
    return written == static_cast<qint64>(bytes.size());
}

}  // namespace

bool UIController::loadDocument(const QString& path, UI::ProjectManager* currentManager) {
    if (path.isEmpty() || !currentManager) {
        return false;
    }

    auto bytes = app::detail::readFile(app::ui::nativePath(path));
    if (!bytes) {
        return false;
    }

    auto decoded = core::persistence::decodeDocument(bytes.value());
    if (!decoded) {
        return false;
    }

    std::string name = std::as_const(*decoded.value()).name();
    if (name.empty()) {
        name = QFileInfo(path).baseName().toStdString();
    }
    if (name.empty()) {
        name = "Sketch";
    }

    const auto id = manager_.openDocument(std::move(decoded.value()));

    qint64 fileID = -1;
    const auto res = fs_.registerProject(path, fileID);
    if (res != UI::FileSystem::FsResult::Ok &&
        res != UI::FileSystem::FsResult::AlreadyExists) {
        manager_.closeDocument(id);
        return false;
        }

    const std::string pathStr = path.toStdString();

    try {
        if (auto* doc = manager_.at(id)) {
            doc->path() = pathStr;
            doc->name() = name;
        }

        documentIds_[pathHash(pathStr)] = id;

        attachDocument(*currentManager, id, fileID);
    } catch (...) {
        manager_.closeDocument(id);
        documentIds_.erase(pathHash(pathStr));
        throw;
    }
    return true;
}


bool UIController::saveDocument(DocumentId id, const QString& path) {
    if (path.isEmpty()) {
        return false;
    }

    Document* document = manager_.at(id);
    if (!document) {
        return false;
    }

    const auto encoded = core::persistence::encodeDocument(*document);
    if (!encoded) {
        return false;
    }

    if (!writeAllBytes(app::ui::nativePath(path), encoded.value())) {
        return false;
    }

    const std::string pathStr = path.toStdString();
    document->path() = pathStr;


    qint64 fileID = -1;
    const auto res = fs_.registerProject(path, fileID);
    if (res != UI::FileSystem::FsResult::Ok &&
        res != UI::FileSystem::FsResult::AlreadyExists) {
        return false;
    }

    projectModified_ = false;
    return true;
}


void UIController::closeDocumentView(DocumentId id) {
    const Document* document = manager_.at(id);
    if (!document) return;

    const std::size_t key = pathHash(document->path());
    QtViewportHost* host = documentHosts_.contains(id) ? documentHosts_[id] : activeHost_;

    for (auto it = views_.begin(); it != views_.end(); ++it) {
        if (&(*it)->document() == document) {
            (*it)->editorSession_.setToolChangedCallback({});
            const auto removed = controllerMap_.find(&(*it)->viewportController_);
            const int removedIndex = removed == controllerMap_.end() ? -1 : removed->second;

            if (host) host->removeController(&(*it)->viewportController_);
            controllerMap_.erase(&(*it)->viewportController_);
            views_.erase(it);

            indicesMap_.erase(key);

            if (removedIndex >= 0) {
                for (auto& [n, idx] : indicesMap_)   if (idx > removedIndex) --idx;
                for (auto& [c, idx] : controllerMap_) if (idx > removedIndex) --idx;
            }
            break;
        }
    }
    documentHosts_.erase(id);
}


DocumentId UIController::idFromPath(const std::string& path) const {
    const auto it = documentIds_.find(pathHash(path));
    return (it != documentIds_.end())
               ? it->second
               : std::numeric_limits<DocumentId>::max();
}

std::size_t UIController::pathHash(const std::string& path) const {
    std::error_code ec;
    const auto canonical = std::filesystem::weakly_canonical(path, ec);
    const std::string normalized = ec
        ? path
        : canonical.generic_string();
    return std::hash<std::string>{}(normalized);
}


void UIController::openFilesFromCommandLine(const QStringList& filePaths) {
    if (filePaths.isEmpty()) return;

    if (projectManagers_.isEmpty()) {
        createManager(createEditorContext(true));
    }

    UI::ProjectManager* manager = projectManagers_.first();
    for (const QString& path : filePaths) {
        openFile(manager, path, false);
    }
}

void UIController::closeOneDocument(DocumentId docId, UI::ProjectManager* currentManager) {
    Document* document = manager_.at(docId);
    if (!document) return;

    const std::string pathStr = document->path();

    if (!pathStr.empty()) {
        saveDocument(docId, QString::fromStdString(pathStr));
    }

    qint64 fileId = -1;
    if (!pathStr.empty()) {
        fs_.getProjectIdByPath(QString::fromStdString(pathStr), fileId);
    }

    closeDocumentView(docId);
    manager_.closeDocument(docId);

    if (!pathStr.empty()) {
        const std::size_t key = pathHash(pathStr);
        documentIds_.erase(key);

        if (activeTabHash_ == key) {
            activeTabHash_ = 0;
        }
    }

   currentManager->closeTabSlot(fileId);
}