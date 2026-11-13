#include "UIController.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "../project/DocumentView.h"

UIController::UIController() = default;
UIController::~UIController() { clearViews(); }

void UIController::setCallbacks(Callbacks callbacks) { callbacks_ = std::move(callbacks); }

DocumentView* UIController::findView(const std::string& name) const {
    const auto entry = documentIds_.find(name);
    if (entry == documentIds_.end()) {
        return nullptr;
    }
    const auto* document = manager_.at(entry->second);
    const auto view = std::find_if(views_.begin(), views_.end(), [document](const auto& candidate) { return &candidate->document() == document; });
    return view == views_.end() ? nullptr : view->get();
}

std::optional<ToolId> UIController::activeToolId() const {
    if (auto* view = findView(activeTabName_)) {
        return view->editorSession_.activeToolId();
    }
    return std::nullopt;
}

void UIController::selectTool(ToolId tool, std::optional<double> value, const std::string& tabName) {
    if (auto* view = findView(tabName.empty() ? activeTabName_ : tabName)) {
        view->editorSession_.selectTool(tool, value);
        view->viewportController_.requestRedraw();
        if (callbacks_.redraw) {
            callbacks_.redraw();
        }
    }
}

void UIController::presentActiveTool(DocumentId id, ToolId tool) {
    if (!callbacks_.toolChanged) {
        return;
    }
    const auto entry = std::find_if(documentIds_.begin(), documentIds_.end(), [id](const auto& item) { return item.second == id; });
    if (entry != documentIds_.end()) {
        callbacks_.toolChanged(entry->first, tool, entry->first == activeTabName_);
    }
}

void UIController::executeConsoleCommand(const std::string& /*command*/) {
    // The existing console has no command interpreter yet.
    if (findView(activeTabName_) && callbacks_.redraw) {
        callbacks_.redraw();
    }
}

void UIController::previewConsoleSelection(const std::vector<int>& ids) {
    if (auto* view = findView(activeTabName_)) {
        std::vector<core::sketch::GeometryRef> refs;
        refs.reserve(ids.size());
        for (const int id : ids) {
            refs.push_back({core::sketch::EntityId(id), core::sketch::SubElement::Whole});
        }
        view->overlay_.selection_.model.replace(refs);
        view->viewportController_.requestRedraw();
        if (callbacks_.redraw) {
            callbacks_.redraw();
        }
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
    if (const auto it = documentIds_.find(fileName); it != documentIds_.end()) {
        attachDocument(it->second, fileName);
        return;
    }
    const auto id = manager_.createNewDocument(fileName);
    try {
        project_.documents.push_back({nextFilename(id), std::cref(*manager_.at(id))});
        attachDocument(id, fileName);
        projectModified_ = true;
    } catch (...) {
        std::erase_if(project_.documents, [this, id](const auto& item) { return &item.document.get() == manager_.at(id); });
        manager_.closeDocument(id);
        throw;
    }
}

void UIController::attachDocument(DocumentId id, const std::string& tabName) {
    auto view = std::make_unique<DocumentView>(*manager_.at(id));
    view->editorSession_.setToolChangedCallback([this, id](ToolId tool) { presentActiveTool(id, tool); });
    auto* viewport = &view->viewportController_;
    views_.reserve(views_.size() + 1);
    const auto previous = activeTabName_;
    const bool newName = documentIds_.try_emplace(tabName, id).second;
    bool attached = false;
    bool publishedTab = false;
    try {
        if (callbacks_.attachViewport) {
            if (!callbacks_.attachViewport(*viewport)) {
                throw std::runtime_error("Could not create a viewport for the document");
            }
            attached = true;
        }
        views_.push_back(std::move(view));
        if (callbacks_.tabAdded) {
            publishedTab = true;
            callbacks_.tabAdded(tabName);
        }
        setActiveFile(tabName);
    } catch (...) {
        if (attached && callbacks_.detachViewport) {
            callbacks_.detachViewport(*viewport);
        }
        if (publishedTab && callbacks_.tabRemoved) {
            callbacks_.tabRemoved(tabName);
        }
        std::erase_if(views_, [viewport](const auto& candidate) { return &candidate->viewportController_ == viewport; });
        if (newName) {
            documentIds_.erase(tabName);
        }
        activeTabName_ = previous;
        setActiveFile(previous);
        throw;
    }
}

void UIController::setActiveFile(const std::string& fileName) {
    auto* view = findView(fileName);
    if (!view) {
        return;
    }
    if (fileName != activeTabName_) {
        if (auto* previous = findView(activeTabName_)) {
            previous->editorSession_.activeTool()->cancel();
        }
        activeTabName_ = fileName;
    }
    if (callbacks_.activeChanged) {
        callbacks_.activeChanged(fileName, view->viewportController_);
    }
    presentActiveTool(documentIds_.at(fileName), view->editorSession_.activeToolId());
    if (callbacks_.redraw) {
        callbacks_.redraw();
    }
}

void UIController::removeFile(const std::string& fileName) {
    auto* view = findView(fileName);
    if (!view) {
        return;
    }
    view->editorSession_.setToolChangedCallback({});
    view->editorSession_.activeTool()->cancel();
    if (callbacks_.detachViewport) {
        callbacks_.detachViewport(view->viewportController_);
    }
    std::erase_if(views_, [view](const auto& candidate) { return candidate.get() == view; });
    if (callbacks_.tabRemoved) {
        callbacks_.tabRemoved(fileName);
    }
    if (activeTabName_ == fileName) {
        activeTabName_.clear();
        for (const auto& [name, id] : documentIds_) {
            if (!views_.empty() && manager_.at(id) == &views_.front()->document()) {
                setActiveFile(name);
                break;
            }
        }
    }
}

void UIController::clearViews() {
    activeTabName_.clear();
    for (const auto& [name, id] : documentIds_) {
        removeFile(name);
    }
}

bool UIController::renameTab(const std::string& oldName, const std::string& newName) {
    auto* view = findView(oldName);
    if (!view || newName.empty() || (oldName != newName && documentIds_.contains(newName))) {
        return false;
    }
    if (oldName != newName) {
        const auto id = documentIds_.at(oldName);
        documentIds_.emplace(newName, id);
        documentIds_.erase(oldName);
        view->document().name() = newName;
        if (activeTabName_ == oldName) {
            activeTabName_ = newName;
        }
        projectModified_ = true;
    }
    return true;
}
