#include "UIController.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "DocumentView.h"

using namespace app;

UIController::UIController() = default;
UIController::~UIController() { clearDocumentViews(); }

DocumentId UIController::createDocument(std::string name) {
    if (name.empty()) {
        return InvalidDocumentId;
    }
    return addDocument(std::make_unique<core::Document>(name));
}

std::string UIController::documentName(DocumentId id) const {
    const core::Document* document = manager_.at(id);
    return document ? document->name() : std::string{};
}

bool UIController::renameDocument(DocumentId id, const std::string& name) {
    auto* document = manager_.at(id);
    if (!document || name.empty()) {
        return false;
    }
    if (std::as_const(*document).name() != name) {
        document->name() = name;
    }
    return true;
}

ViewId UIController::createDocumentView(DocumentId id) {
    auto* document = manager_.at(id);
    if (!document) {
        return InvalidViewId;
    }
    if (nextViewId_ == 0) {
        throw std::overflow_error("View IDs exhausted");
    }
    const ViewId view{nextViewId_++};
    auto state = std::make_unique<DocumentView>(*document, id);
    state->editorSession_.setToolChangedCallback([this, view](ToolId tool) {
        if (observer_) {
            observer_->toolChanged(view, tool);
        }
    });
    state->viewportController_.setSiblingRedrawCallback([this, id, view] {
        for (const auto& [other, entry] : views_) {
            if (other != view && entry->id_ == id) {
                requestRedraw(other);
            }
        }
    });
    views_.emplace(view, std::move(state));
    return view;
}

ViewportController* UIController::findViewport(ViewId view) {
    const auto it = views_.find(view);
    return it == views_.end() ? nullptr : &it->second->viewportController_;
}

DocumentId UIController::documentOf(ViewId view) const {
    const auto it = views_.find(view);
    return it == views_.end() ? InvalidDocumentId : it->second->id_;
}

void UIController::closeDocumentView(ViewId view) {
    const auto it = views_.find(view);
    if (it == views_.end()) {
        return;
    }
    it->second->editorSession_.setToolChangedCallback({});
    it->second->editorSession_.activeTool()->cancel();
    for (auto sibling : documentViews(it->second->id_)) {
        if (sibling != view) {
            requestRedraw(sibling);
        }
    }
    if (observer_) {
        observer_->viewAboutToClose(view);
    }
    views_.erase(it);
    if (active_ == view) {
        active_ = InvalidViewId;
        setActiveDocumentView(views_.empty() ? InvalidViewId : views_.begin()->first);
    }
}

void UIController::clearDocumentViews() {
    // Do not activate intermediate tabs while closing all views.
    const bool hadActiveView = active_ != InvalidViewId;
    active_ = InvalidViewId;
    while (!views_.empty()) {
        closeDocumentView(views_.begin()->first);
    }
    if (hadActiveView && observer_) {
        observer_->activeViewChanged(InvalidViewId);
    }
}

void UIController::setActiveDocumentView(ViewId view) {
    if (view != InvalidViewId && !views_.contains(view)) {
        return;
    }
    if (view != active_) {
        if (const auto old = views_.find(active_); old != views_.end()) {
            old->second->editorSession_.activeTool()->cancel();
            for (auto sibling : documentViews(old->second->id_)) {
                requestRedraw(sibling);
            }
        }
        active_ = view;
    }
    if (observer_) {
        observer_->activeViewChanged(view);
    }
}

void UIController::selectTool(ViewId view, ToolId tool, std::optional<double> value) {
    if (const auto it = views_.find(view); it != views_.end()) {
        it->second->editorSession_.selectTool(tool, value);
        for (auto sibling : documentViews(it->second->id_)) {
            requestRedraw(sibling);
        }
    }
}

std::optional<ToolId> UIController::activeToolId(ViewId view) const {
    const auto it = views_.find(view);
    return it == views_.end() ? std::nullopt : std::optional{it->second->editorSession_.activeToolId()};
}

void UIController::requestRedraw(ViewId view) {
    if (auto* viewport = findViewport(view)) {
        viewport->dirty_ = true;
        if (observer_) {
            observer_->redrawRequested(view);
        }
    }
}

void UIController::endInteractions() {
    for (auto& [id, view] : views_) {
        view->editorSession_.activeTool()->cancel();
        requestRedraw(id);
    }
}

std::vector<ViewId> UIController::documentViews(DocumentId document) const {
    std::vector<ViewId> result;
    for (const auto& [id, view] : views_) {
        if (view->id_ == document) {
            result.push_back(id);
        }
    }
    return result;
}

bool UIController::closeDocument(DocumentId document) {
    if (!manager_.at(document)) {
        return false;
    }
    for (auto view : documentViews(document)) {
        closeDocumentView(view);
    }
    documentStates_.erase(document);
    std::erase(documents_, document);
    return manager_.closeDocument(document);
}

int UIController::getDOFs(DocumentId id) const {
    auto* document = manager_.at(id);
    if (!document) {
        return -1;
    }
    auto result = document->sketch().diagnose();
    if (!result || !result.value().degreesOfFreedom.has_value()) {
        return -1;
    }
    return result.value().degreesOfFreedom.value();
}
