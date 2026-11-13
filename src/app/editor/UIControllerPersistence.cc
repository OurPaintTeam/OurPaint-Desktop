#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "../project/DocumentView.h"
#include "../project/NativeFile.h"
#include "UIController.h"

namespace {

std::string utf8Name(const std::filesystem::path& path) {
    const auto text = path.u8string();
    return {text.begin(), text.end()};
}

std::string availableName(const std::unordered_map<std::string, DocumentId>& names, const std::string& preferred) {
    const std::string base = preferred.empty() ? "Sketch" : preferred;
    auto name = base;
    for (size_t suffix = 2; names.contains(name); ++suffix) {
        name = base + " (" + std::to_string(suffix) + ")";
    }
    return name;
}

}  // namespace

std::string UIController::uniqueTabName(const std::string& preferred) const { return availableName(documentIds_, preferred); }

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
    for (auto& view : views_) {
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

std::optional<app::FileError> UIController::openFile(const std::filesystem::path& path) {
    if (!project_.root.empty()) {
        for (const auto& item : project_.documents) {
            std::error_code error;
            if (std::filesystem::equivalent(path, project_.root / item.filename, error)) {
                for (const auto& [name, id] : documentIds_) {
                    if (manager_.at(id) == &item.document.get()) {
                        createFile(name);
                        return {};
                    }
                }
            }
        }
    }
    auto bytes = app::detail::readFile(path);
    if (!bytes) {
        return bytes.error();
    }
    auto decoded = core::persistence::decodeDocument(bytes.value());
    if (!decoded) {
        return app::FileError{path, app::FileStage::Decode, {}, decoded.error()};
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
    return {};
}

void UIController::installProject(app::LoadedProject loaded) {
    // Stage all model and host resources before dropping the currently open project.
    // Project::documents borrows the staged models; adoption by the manager preserves their addresses.
    auto description = loaded.view();
    std::vector<DocumentId> ids;
    std::vector<std::unique_ptr<DocumentView>> candidates;
    std::unordered_map<std::string, DocumentId> names;
    std::vector<std::string> labels;
    ids.reserve(loaded.documents.size());
    candidates.reserve(loaded.documents.size());
    labels.reserve(loaded.documents.size());

    try {
        for (auto& item : loaded.documents) {
            const auto id = manager_.openDocument(std::move(item.document));
            ids.push_back(id);
            Document& document = *manager_.at(id);
            const auto& documentName = std::as_const(document).name();
            auto name = availableName(names, documentName.empty() ? utf8Name(item.filename.stem()) : documentName);
            names.emplace(name, id);
            labels.push_back(std::move(name));

            auto view = std::make_unique<DocumentView>(document);
            view->editorSession_.setToolChangedCallback([this, id](ToolId tool) { presentActiveTool(id, tool); });
            if (callbacks_.attachViewport && !callbacks_.attachViewport(view->viewportController_)) {
                throw std::runtime_error("Could not create a viewport for a loaded document");
            }
            candidates.push_back(std::move(view));
        }
    } catch (...) {
        if (callbacks_.detachViewport) {
            for (auto& view : candidates) {
                callbacks_.detachViewport(view->viewportController_);
            }
        }
        candidates.clear();
        for (auto id : ids) {
            manager_.closeDocument(id);
        }
        throw;
    }

    clearViews();
    project_.documents.clear();
    for (const auto& [name, id] : documentIds_) {
        manager_.closeDocument(id);
    }
    documentIds_ = std::move(names);
    views_ = std::move(candidates);
    project_ = std::move(description);
    projectModified_ = false;
    if (callbacks_.tabAdded) {
        for (const auto& name : labels) {
            callbacks_.tabAdded(name);
        }
    }
    if (!labels.empty()) {
        setActiveFile(labels.back());
    }
}
