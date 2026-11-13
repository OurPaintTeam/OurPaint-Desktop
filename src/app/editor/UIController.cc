#include "UIController.h"

#include <stdexcept>
#include <utility>
#include <algorithm>
#include <ranges>

#include "DocumentView.h"
#include "NativeFile.h"

UIController::UIController() = default;
UIController::~UIController() {
    clearDocumentViews();
}

DocumentId UIController::createDocument(std::string name) {
    if (name.empty()) {
        return InvalidDocumentId;
    }
    return addDocument(std::make_unique<Document>(name));
}

DocumentId UIController::addDocument(std::unique_ptr<Document> document) {
    documents_.reserve(documents_.size() + 1);
    const DocumentId id = manager_.openDocument(std::move(document));
    try {
        project_.documents.push_back({nextFilename(id), std::cref(*manager_.at(id))});
    } catch (...) {
        manager_.closeDocument(id);
        throw;
    }
    documents_.push_back(id);
    projectModified_ = true;
    return id;
}

std::string UIController::documentName(DocumentId id) const {
    const Document* document = manager_.at(id);
    return document ? document->name() : std::string{};
}

bool UIController::renameDocument(DocumentId id, const std::string& name) {
    auto* document = manager_.at(id);
    if (!document || name.empty()) {
        return false;
    }
    if (std::as_const(*document).name() != name) {
        document->name() = name;
        projectModified_ = true;
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
    auto state = std::make_unique<DocumentView>(*document);
    state->editorSession_.setToolChangedCallback([this, view](ToolId tool) {
        if (observer_) {
            observer_->toolChanged(view, tool);
        }
    });
    state->viewportController_.setSiblingRedrawCallback([this, id, view] {
        for (const auto& [other, entry] : views_) {
            if (other != view && entry->id == id) {
                requestRedraw(other);
            }
        }
    });
    views_.emplace(view, std::move(state));
    return view;
}

IViewportController* UIController::findViewport(ViewId view) {
    const auto it = views_.find(view);
    return it == views_.end() ? nullptr : &it->second->viewportController_;
}

DocumentId UIController::documentOf(ViewId view) const {
    const auto it = views_.find(view);
    return it == views_.end() ? InvalidDocumentId : it->second->id;
}

void UIController::closeDocumentView(ViewId view) {
    const auto it = views_.find(view);
    if (it == views_.end()) {
        return;
    }
    it->second->editorSession_.setToolChangedCallback({});
    it->second->editorSession_.activeTool()->cancel();
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
    // Do not activate intermediate tabs while closing a whole project.
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
        requestRedraw(view);
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

// void UIController::executeConsoleCommand(const std::string& /*command*/) {
//     // The existing console has no command interpreter yet.
//     requestRedraw(active_);
// }
//
// void UIController::previewConsoleSelection(const std::vector<int>& ids) {
//     if (const auto it = views_.find(active_); it != views_.end()) {
//         std::vector<core::sketch::GeometryRef> refs;
//         refs.reserve(ids.size());
//         for (const int id : ids) {
//             refs.push_back({core::sketch::EntityId(id), core::sketch::SubElement::Whole});
//         }
//         it->second->overlay_.selection_.model.replace(refs);
//         requestRedraw(active_);
//     }
// }


namespace {

std::string utf8Name(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {text.begin(), text.end()};
}

}  // namespace

std::filesystem::path UIController::nextFilename(DocumentId id) const {
    const app::detail::FilenameLess less;
    for (uint64_t suffix = 0;; ++suffix) {
        const std::filesystem::path candidate = "sketch-" + std::to_string(id) + (suffix ? "-" + std::to_string(suffix) : std::string{}) + ".ourp";
        const bool used =
            std::ranges::any_of(project_.documents, [&](const auto& item) { return !less(item.filename, candidate) && !less(candidate, item.filename); });
        if (!used) {
            return candidate;
        }
    }
}

void UIController::endInteractions() {
    for (auto& [id, view] : views_) {
        view->editorSession_.activeTool()->cancel();
    }
}

const std::filesystem::path& UIController::projectRoot() const { return project_.root; }

const app::ProjectMetadata& UIController::projectMetadata() const { return project_.metadata; }

bool UIController::needsSaveConfirmation() const {
    if (projectModified_) {
        return !project_.root.empty() || !project_.documents.empty();
    }
    // Tools currently hold mutable sketch references; Document::isDirty() cannot distinguish
    // a query from an edit, or identify edits since a successful save. Compare one document
    // at a time on close/replacement. A missing or unreadable file also warrants confirmation.
    for (const auto& item : project_.documents) {
        const auto current = core::persistence::encodeDocument(item.document.get());
        const auto saved = app::detail::readFile(project_.root / item.filename);
        if (!current || !saved || saved.value() != current.value()) {
            return true;
        }
    }
    return false;
}

app::SaveReport UIController::saveProject() {
    endInteractions();
    auto report = app::saveProject(project_);
    if (report) {
        projectModified_ = false;
    }
    return report;
}

app::SaveReport UIController::saveProjectAs(const std::filesystem::path& destination) {
    endInteractions();
    auto description = project_;
    if (description.root.empty()) {
        description.metadata.name = utf8Name(destination.filename());
    }
    auto report = app::saveProjectAs(description, destination);
    if (report) {
        project_.root = report.root;
        project_.metadata = report.metadata;
        projectModified_ = false;
    }
    return report;
}

app::SaveReport UIController::createProject(const std::filesystem::path& destination) {
    app::Project draft{{}, {{}, utf8Name(destination.filename())}, {}};
    auto report = app::saveProjectAs(draft, destination);
    if (report) {
        installProject({report.root, report.metadata, {}});
    }
    return report;
}

std::optional<app::FileError> UIController::openProject(const std::filesystem::path& path) {
    auto root = path;
    if (root.filename() == ".ourpaint") {
        root = root.has_parent_path() ? root.parent_path() : std::filesystem::path(".");
    }
    auto loaded = app::loadProject(root);
    if (!loaded) {
        return loaded.error();
    }
    installProject(std::move(loaded.value()));
    return {};
}

app::FileResult<DocumentId> UIController::openFile(const std::filesystem::path& path) {
    using Result = app::FileResult<DocumentId>;
    if (!project_.root.empty()) {
        for (size_t i = 0; i < project_.documents.size(); ++i) {
            std::error_code error;
            if (std::filesystem::equivalent(path, project_.root / project_.documents[i].filename, error)) {
                return Result::success(documents_[i]);
            }
        }
    }
    auto bytes = app::detail::readFile(path);
    if (!bytes) {
        return Result::failure(bytes.error());
    }
    auto decoded = core::persistence::decodeDocument(bytes.value());
    if (!decoded) {
        return Result::failure({path, app::FileStage::Decode, {}, decoded.error()});
    }
    return Result::success(addDocument(std::move(decoded.value())));
}

void UIController::installProject(app::LoadedProject loaded) {
    // Stage adoption before invalidating borrowed viewports from the previous
    // project. A command-line client can load the same project without any views.
    auto description = loaded.view();
    std::vector<DocumentId> ids;
    ids.reserve(loaded.documents.size());
    try {
        for (auto& item : loaded.documents) {
            ids.push_back(manager_.openDocument(std::move(item.document)));
        }
    } catch (...) {
        for (auto id : ids) {
            manager_.closeDocument(id);
        }
        throw;
    }

    clearDocumentViews();
    project_ = std::move(description);
    for (auto id : documents_) {
        manager_.closeDocument(id);
    }
    documents_ = std::move(ids);
    projectModified_ = false;
}

