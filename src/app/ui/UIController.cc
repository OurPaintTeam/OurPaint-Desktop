#include "UIController.h"

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
#include "OpenGL2dRenderer.h"

UIController::UIController(std::vector<std::unique_ptr<DocumentView>>& views,
                           DocumentManager& manager,
                           IPlatformRuntime& platformRuntime,
                           UI::ProjectManager& projectManager,
                           QtViewportHost& host)
    : views_(views),
      manager_(manager),
      platformRuntime_(platformRuntime),
      viewportHost_(host),
      projectManager_(projectManager),
      reportPresenter_(projectManager),
      activeTabName_({}) {}

UIController::~UIController() {
    for (auto& view : views_) {
        view->editorSession_.setReportCallback({});
        viewportHost_.removeController(&view->viewportController_);
    }
}

DocumentView* UIController::findView(const std::string& name) const {
    const auto entry = documentIds_.find(name);
    if (entry == documentIds_.end()) {
        return nullptr;
    }
    const Document* document = manager_.at(entry->second);
    for (const auto& view : views_) {
        if (&view->document() == document) {
            return view.get();
        }
    }
    return nullptr;
}

void UIController::selectTool(ToolId tool, const std::string& tabName) {
    if (auto* view = findView(tabName.empty() ? activeTabName_ : tabName)) {
        view->editorSession_.select(tool);
        view->viewportController_.requestRedraw();
        viewportHost_.requestRedraw();
    }
}

void UIController::requestConstraint(const ConstraintRequest& request, const std::string& tabName) {
    const auto& name = tabName.empty() ? activeTabName_ : tabName;
    if (auto* view = findView(name)) {
        view->editorSession_.requestConstraint(request);
        view->viewportController_.requestRedraw();
        viewportHost_.requestRedraw();
    }
}

std::function<void(const ConstraintRequest&)> UIController::constraintRequestHandler(const std::string& tabName) {
    const auto entry = documentIds_.find(tabName.empty() ? activeTabName_ : tabName);
    const DocumentId origin = entry == documentIds_.end() ? InvalidDocumentId : entry->second;
    return [this, origin](const ConstraintRequest& request) {
        for (const auto& [name, id] : documentIds_) {
            if (id == origin && findView(name)) {
                requestConstraint(request, name);
                return;
            }
        }
    };
}

void UIController::executeConsoleCommand(std::string str) {
    try {
        const auto entry = documentIds_.find(activeTabName_);
        Document* document = entry == documentIds_.end() ? nullptr : manager_.at(entry->second);
        // UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
        // CommandManager& cm = document->commandManager();
        // Transaction* txn = cm.invoke(str);
        // urm.push(std::move(*txn));

        if (document) {
            viewportHost_.requestRedraw();
        }
    }catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

void UIController::createFile(const std::string& fileName) {
    if (fileName.empty()) {
        return;
    }
    if (findView(fileName)) {
        setActiveFile(fileName);
        return;
    }
    if (auto* previous = findView(activeTabName_)) {
        previous->editorSession_.endInteraction();
    }
    const auto existing = documentIds_.find(fileName);
    if (existing != documentIds_.end()) {
        attachDocument(existing->second, fileName);
        return;
    }
    const auto id = manager_.createNewDocument(fileName);
    try {
        const auto filename = nextFilename(id);
        project_.documents.push_back({filename, std::cref(*manager_.at(id))});
        attachDocument(id, fileName);
        projectModified_ = true;
    } catch (...) {
        std::erase_if(project_.documents, [this, id](const auto& item) { return &item.document.get() == manager_.at(id); });
        manager_.closeDocument(id);
        throw;
    }
}

void UIController::attachDocument(DocumentId id, const std::string& tabName) {
    auto view = std::make_unique<DocumentView>(*manager_.at(id), viewportHost_);
    view->editorSession_.setReportCallback([this, id](const ActionReport& report) { presentReport(id, report); });
    const int index = viewportHost_.addController(&view->viewportController_);
    if (index < 0) {
        throw std::runtime_error("Could not create a viewport for the document");
    }
    auto* controller = &view->viewportController_;
    const bool registered = documentIds_.contains(tabName);
    const auto previous = activeTabName_;
    try {
        documentIds_[tabName] = id;
        indicesMap_[tabName] = index;
        controllerMap_[controller] = index;
        views_.push_back(std::move(view));
        projectManager_.addTabSlot(QString::fromStdString(tabName));
        setActiveFile(tabName);
    } catch (...) {
        viewportHost_.removeController(controller);
        std::erase_if(views_, [controller](const auto& candidate) { return &candidate->viewportController_ == controller; });
        indicesMap_.erase(tabName);
        controllerMap_.erase(controller);
        if (!registered) {
            documentIds_.erase(tabName);
        }
        projectManager_.deleteTabSlot(QString::fromStdString(tabName));
        activeTabName_ = previous;
        throw;
    }
}

void UIController::presentReport(DocumentId id, const ActionReport& report) {
    if (report.change != ModelChange::Unchanged) {
        projectModified_ = true;
    }
    for (const auto& [name, documentId] : documentIds_) {
        if (documentId == id) {
            reportPresenter_.present(name, report);
            return;
        }
    }
}

void UIController::setActiveFile(const std::string& fileName) {
    if (!findView(fileName)) {
        return;
    }
    if (activeTabName_ != fileName) {
        if (auto* previous = findView(activeTabName_)) {
            previous->editorSession_.endInteraction();
        }
    }

    activeTabName_ = fileName;
    int index = indicesMap_[fileName];
    viewportHost_.setActiveController(index);
    projectManager_.setActiveTabNameProject(QString::fromStdString(fileName));
    viewportHost_.requestRedraw();
}

void UIController::removeFile(const std::string& fileName) {
    const auto* target = findView(fileName);
    for (auto it = views_.begin(); it != views_.end(); ++it) {
        if (it->get() == target) {
            (*it)->editorSession_.setReportCallback({});
            const auto removed = controllerMap_.find(&(*it)->viewportController_);
            const int removedIndex = removed == controllerMap_.end() ? -1 : removed->second;
            viewportHost_.removeController(&(*it)->viewportController_);
            controllerMap_.erase(&(*it)->viewportController_);
            views_.erase(it);
            indicesMap_.erase(fileName);
            if (removedIndex >= 0) {
                for (auto& [name, index] : indicesMap_) {
                    if (index > removedIndex) {
                        --index;
                    }
                }
                for (auto& [controller, index] : controllerMap_) {
                    if (index > removedIndex) {
                        --index;
                    }
                }
            }
            if (activeTabName_ == fileName) {
                activeTabName_ = indicesMap_.empty() ? std::string{} : indicesMap_.begin()->first;
            }
            if (!activeTabName_.empty()) {
                setActiveFile(activeTabName_);
            }
            return;
        }
    }
}

void UIController::renameTab(const std::string& oldName, const std::string& newName) {
    auto* view = findView(oldName);
    if (!view) {
        return;
    }
    if (newName.empty() || (oldName != newName && documentIds_.contains(newName))) {
        projectManager_.renameTabSlot(QString::fromStdString(newName), QString::fromStdString(oldName));
        return;
    }
    int index = indicesMap_[oldName];
    indicesMap_.erase(oldName);
    indicesMap_[newName] = index;
    const auto id = documentIds_.at(oldName);
    documentIds_.erase(oldName);
    documentIds_[newName] = id;
    view->document().name() = newName;
    projectModified_ = true;
    if (activeTabName_ == oldName) {
        activeTabName_ = newName;
    }
    projectManager_.renameTabSlot(QString::fromStdString(oldName), QString::fromStdString(newName));
}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}











