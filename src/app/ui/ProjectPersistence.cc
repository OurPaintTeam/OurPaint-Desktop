#include "UIController.h"

#include <QApplication>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QSignalBlocker>

#include <algorithm>
#include <stdexcept>

#include "../project/NativeFile.h"
#include "ProjectWindow.h"
#include "QtPath.h"

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

}  // namespace

std::string UIController::uniqueTabName(const std::string& preferred) const {
    const auto base = preferred.empty() ? std::string("Sketch") : preferred;
    auto name = base;
    for (size_t suffix = 2; documentIds_.contains(name); ++suffix) {
        name = base + " (" + std::to_string(suffix) + ")";
    }
    return name;
}

std::filesystem::path UIController::nextFilename(DocumentId id) const {
    for (uint64_t suffix = 0;; ++suffix) {
        const auto name = "sketch-" + std::to_string(id) + (suffix ? "-" + std::to_string(suffix) : std::string{}) + ".ourp";
        const auto candidate = std::filesystem::path(name);
        const bool used = std::any_of(project_.documents.begin(), project_.documents.end(), [&candidate](const auto& item) {
#ifdef _WIN32
            return app::ui::pathText(item.filename).compare(app::ui::pathText(candidate), Qt::CaseInsensitive) == 0;
#else
            return item.filename == candidate;
#endif
        });
        if (!used) {
            return candidate;
        }
    }
}

void UIController::endInteractions() {
    for (auto& view : views_) {
        view->editorSession_.endInteraction();
    }
}

void UIController::updateProjectData() {
    projectManager_.setProjectData({app::ui::pathText(project_.root), QString::fromStdString(project_.metadata.id)});
    if (auto* window = dialogParent()) {
        window->setWindowTitle(QString::fromStdString(project_.metadata.name) + QStringLiteral(" - OurPaint"));
    }
}

bool UIController::saveProject() {
    if (project_.root.empty()) {
        return saveProjectAs();
    }
    endInteractions();
    const auto report = app::saveProject(project_);
    updateProjectData();
    reportPresenter_.present(report);
    if (report) {
        projectModified_ = false;
    }
    return static_cast<bool>(report);
}

bool UIController::saveProjectAs() {
    endInteractions();
    const auto destination = chooseFolder("Save project to a new or empty folder");
    if (destination.isEmpty()) {
        return false;
    }
    auto description = project_;
    if (description.root.empty()) {
        description.metadata.name = QFileInfo(destination).fileName().toStdString();
    }
    const auto report = app::saveProjectAs(description, app::ui::nativePath(destination));
    reportPresenter_.present(report);
    if (!report) {
        return false;
    }
    project_.root = report.root;
    project_.metadata = report.metadata;
    projectModified_ = false;
    updateProjectData();
    return true;
}

bool UIController::confirmProjectChange() {
    endInteractions();
    if (!projectModified_ || (project_.root.empty() && project_.documents.empty())) {
        return true;
    }
    const auto answer = QMessageBox::question(dialogParent(), QCoreApplication::translate("ProjectPersistence", "Unsaved project"),
                                              QCoreApplication::translate("ProjectPersistence", "Save the current project before continuing?"),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);
    return answer == QMessageBox::Discard || (answer == QMessageBox::Save && saveProject());
}

bool UIController::confirmClose() { return confirmProjectChange(); }

bool UIController::openProjectInCurrentWindow(const QString& requestedPath) {
    const auto selected = requestedPath.isEmpty() ? chooseFolder("Open project folder") : requestedPath;
    if (selected.isEmpty() || !confirmProjectChange()) {
        return false;
    }
    const auto info = QFileInfo(selected);
    const auto root = info.fileName() == QStringLiteral(".ourpaint") ? info.absolutePath() : selected;
    auto loaded = app::loadProject(app::ui::nativePath(root));
    if (!loaded) {
        reportPresenter_.present(loaded.error());
        return false;
    }
    installProject(std::move(loaded.value()));
    projectManager_.addNotification(QCoreApplication::translate("ProjectPersistence", "Project opened."));
    return true;
}

bool UIController::createProjectInCurrentWindow(const QString& requestedPath) {
    const auto destination = requestedPath.isEmpty() ? chooseFolder("Create project in a new or empty folder") : requestedPath;
    if (destination.isEmpty() || !confirmProjectChange()) {
        return false;
    }
    app::Project draft{{}, {{}, QFileInfo(destination).fileName().toStdString()}, {}};
    const auto report = app::saveProjectAs(draft, app::ui::nativePath(destination));
    reportPresenter_.present(report);
    if (!report) {
        return false;
    }
    installProject({report.root, report.metadata, {}});
    return true;
}

void UIController::openProjectInNewWindow(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty() ? chooseFolder("Open project folder") : requestedPath;
    if (!path.isEmpty() && !QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--open-project"), path})) {
        projectManager_.addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open another window."));
    }
}

void UIController::createProjectInNewWindow(const QString& requestedPath) {
    const auto path = requestedPath.isEmpty() ? chooseFolder("Create project in a new or empty folder") : requestedPath;
    if (!path.isEmpty() && !QProcess::startDetached(QCoreApplication::applicationFilePath(), {QStringLiteral("--create-project"), path})) {
        projectManager_.addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open another window."));
    }
}

void UIController::openFile(const QString& requestedPath) {
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
    const auto canonical = QFileInfo(path).canonicalFilePath();
    if (!canonical.isEmpty() && !project_.root.empty()) {
        for (const auto& item : project_.documents) {
            if (QFileInfo(app::ui::pathText(project_.root / item.filename)).canonicalFilePath() == canonical) {
                for (const auto& [name, id] : documentIds_) {
                    if (manager_.at(id) == &item.document.get()) {
                        createFile(name);
                        return;
                    }
                }
            }
        }
    }
    auto bytes = app::detail::readFile(app::ui::nativePath(path));
    if (!bytes) {
        reportPresenter_.present(bytes.error());
        return;
    }
    auto decoded = core::persistence::decodeDocument(bytes.value());
    if (!decoded) {
        reportPresenter_.present(app::FileError{app::ui::nativePath(path), app::FileStage::Decode, {}, decoded.error()});
        return;
    }
    const auto name = uniqueTabName(std::as_const(*decoded.value()).name());
    const auto id = manager_.openDocument(std::move(decoded.value()));
    try {
        project_.documents.push_back({nextFilename(id), std::cref(*manager_.at(id))});
        attachDocument(id, name);
        projectModified_ = true;
    } catch (...) {
        std::erase_if(project_.documents, [this, id](const auto& item) { return &item.document.get() == manager_.at(id); });
        manager_.closeDocument(id);
        throw;
    }
}

void UIController::installProject(app::LoadedProject loaded) {
    auto description = loaded.view();
    std::vector<DocumentId> ids;
    std::vector<std::unique_ptr<DocumentView>> candidates;
    std::unordered_map<std::string, DocumentId> names;
    std::unordered_map<std::string, int> indices;
    std::unordered_map<ViewportController*, int> controllers;
    std::vector<std::string> labels;
    ids.reserve(loaded.documents.size());
    candidates.reserve(loaded.documents.size());
    labels.reserve(loaded.documents.size());
    try {
        for (auto& item : loaded.documents) {
            ids.push_back(manager_.openDocument(std::move(item.document)));
        }
        for (size_t i = 0; i < ids.size(); ++i) {
            auto* document = manager_.at(ids[i]);
            auto base = std::as_const(*document).name();
            if (base.empty()) {
                base = app::ui::pathText(loaded.documents[i].filename.stem()).toStdString();
            }
            if (base.empty()) {
                base = "Sketch";
            }
            auto name = base;
            for (size_t suffix = 2; names.contains(name); ++suffix) {
                name = base + " (" + std::to_string(suffix) + ")";
            }
            labels.push_back(name);
            auto view = std::make_unique<DocumentView>(*document, viewportHost_);
            const auto id = ids[i];
            view->editorSession_.setReportCallback([this, id](const ActionReport& report) { presentReport(id, report); });
            candidates.push_back(std::move(view));
            const int index = viewportHost_.addController(&candidates.back()->viewportController_);
            if (index < 0) {
                throw std::runtime_error("Could not create a viewport for a loaded document");
            }
            names.emplace(name, id);
            indices.emplace(name, index);
            controllers.emplace(&candidates.back()->viewportController_, index);
        }
    } catch (...) {
        for (auto& view : candidates) {
            view->editorSession_.setReportCallback({});
            viewportHost_.removeController(&view->viewportController_);
        }
        candidates.clear();
        for (const auto id : ids) {
            manager_.closeDocument(id);
        }
        throw;
    }

    // Prevent tab selection callbacks while replacing the complete UI binding.
    const QSignalBlocker blocker(projectManager_);
    for (const auto& [name, id] : documentIds_) {
        projectManager_.deleteTabSlot(QString::fromStdString(name));
    }
    for (auto& view : views_) {
        view->editorSession_.setReportCallback({});
        viewportHost_.removeController(&view->viewportController_);
    }
    views_.clear();
    project_.documents.clear();
    for (const auto& [name, id] : documentIds_) {
        manager_.closeDocument(id);
    }
    documentIds_ = std::move(names);
    indicesMap_ = std::move(indices);
    controllerMap_ = std::move(controllers);
    views_ = std::move(candidates);
    project_ = std::move(description);
    activeTabName_.clear();
    // Removing old host entries shifts the indices of the staged controllers.
    for (const auto& name : labels) {
        auto* view = findView(name);
        const int index = viewportHost_.addController(&view->viewportController_);
        indicesMap_.at(name) = index;
        controllerMap_.at(&view->viewportController_) = index;
        projectManager_.addTabSlot(QString::fromStdString(name));
    }
    if (!labels.empty()) {
        setActiveFile(labels.back());
    }
    projectModified_ = false;
    updateProjectData();
}
