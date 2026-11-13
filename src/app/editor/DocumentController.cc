#include "DocumentController.h"

#include <fstream>
#include <system_error>

#include "DocumentSaveLoad.h"
#include "NativeFile.h"

namespace {
bool writeFileBytes(const std::filesystem::path& path, const core::persistence::DocumentBytes& bytes) {
    if (path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return false;
    }
    if (!bytes.empty()) {
        file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    }
    return file.good();
}
}  // namespace

DocumentController::DocumentController(DocumentManager& manager, std::vector<std::unique_ptr<DocumentView>>& views) : manager_(manager), views_(views) {}

DocumentId DocumentController::loadDocument(const std::string& path) {
    const auto key = pathHash(path);
    if (const auto it = documentIds_.find(key); it != documentIds_.end()) {
        return it->second;
    }

    auto bytes = app::detail::readFile(std::filesystem::path(path));
    if (!bytes) {
        return InvalidId;
    }

    auto decoded = core::persistence::decodeDocument(bytes.value());
    if (!decoded) {
        return InvalidId;
    }

    std::string name = std::as_const(*decoded.value()).name();
    if (name.empty()) {
        name = std::filesystem::path(path).stem().string();
    }
    if (name.empty()) {
        name = "Sketch";
    }

    const auto id = manager_.openDocument(std::move(decoded.value()));

    if (auto* doc = manager_.at(id)) {
        doc->path() = path;
        doc->name() = name;
    }

    documentIds_[key] = id;
    return id;
}

DocumentId DocumentController::createDocument(const std::string& path) {
    const auto key = pathHash(path);
    if (const auto it = documentIds_.find(key); it != documentIds_.end()) {
        return it->second;
    }

    std::string name = std::filesystem::path(path).stem().string();
    if (name.empty()) {
        name = "Sketch";
    }

    const auto id = manager_.createNewDocument(name);

    if (auto* doc = manager_.at(id)) {
        doc->path() = path;
    }

    documentIds_[key] = id;
    return id;
}

bool DocumentController::saveDocument(DocumentId id) {
    const auto* doc = manager_.at(id);
    if (!doc) {
        return false;
    }
    const auto path = doc->path();
    if (path.empty()) {
        return false;
    }
    return saveDocumentAs(id, path);
}

bool DocumentController::saveDocumentAs(DocumentId id, const std::string& newPath) {
    if (newPath.empty()) {
        return false;
    }

    auto* doc = manager_.at(id);
    if (!doc) {
        return false;
    }

    const auto encoded = core::persistence::encodeDocument(*doc);
    if (!encoded) {
        return false;
    }

    if (!writeFileBytes(std::filesystem::path(newPath), encoded.value())) {
        return false;
    }

    const std::string oldPath = doc->path();
    if (!oldPath.empty() && oldPath != newPath) {
        documentIds_.erase(pathHash(oldPath));
    }

    doc->path() = newPath;
    documentIds_[pathHash(newPath)] = id;
    return true;
}

bool DocumentController::closeDocument(DocumentId id) {
    const auto* doc = manager_.at(id);
    if (doc && !doc->path().empty()) {
        documentIds_.erase(pathHash(doc->path()));
    }

    return manager_.closeDocument(id);
}

bool DocumentController::renameDocument(DocumentId id, const std::string& newName) {
    auto* doc = manager_.at(id);
    if (!doc) {
        return false;
    }

    const std::string oldPath = doc->path();
    if (oldPath.empty() || newName.empty()) {
        return false;
    }

    std::filesystem::path oldFsPath(oldPath);
    std::filesystem::path newFsPath = oldFsPath.parent_path() / (newName + oldFsPath.extension().string());
    const std::string newPath = newFsPath.string();

    if (oldPath == newPath) {
        return true;
    }
    if (std::filesystem::exists(newFsPath)) {
        return false;
    }

    std::error_code ec;
    std::filesystem::rename(oldFsPath, newFsPath, ec);
    if (ec) {
        return false;
    }

    const std::size_t oldKey = pathHash(oldPath);
    const std::size_t newKey = pathHash(newPath);

    documentIds_.erase(oldKey);
    documentIds_[newKey] = id;

    if (auto idxIt = indicesMap_.find(oldKey); idxIt != indicesMap_.end()) {
        const int index = idxIt->second;
        indicesMap_.erase(idxIt);
        indicesMap_[newKey] = index;
    }

    if (activeTabHash_ == oldKey) {
        activeTabHash_ = newKey;
    }

    doc->path() = newPath;
    doc->name() = newName;
    return true;
}

DocumentId DocumentController::idFromPath(const std::string& path) const {
    const auto it = documentIds_.find(pathHash(path));
    return it != documentIds_.end() ? it->second : InvalidId;
}

std::string DocumentController::pathOf(DocumentId id) const {
    const auto* doc = manager_.at(id);
    return doc ? doc->path() : std::string{};
}

std::string DocumentController::nameOf(DocumentId id) const {
    const auto* doc = manager_.at(id);
    return doc ? doc->name() : std::string{};
}

bool DocumentController::isOpened(DocumentId id) const { return manager_.at(id) != nullptr; }

std::size_t DocumentController::pathHash(const std::string& path) {
    std::error_code ec;
    const auto canonical = std::filesystem::weakly_canonical(path, ec);
    const std::string normalized = ec ? path : canonical.generic_string();
    return std::hash<std::string>{}(normalized);
}

int DocumentController::registerView(std::unique_ptr<DocumentView> view, int controllerIndex) {
    if (!view || controllerIndex < 0) {
        return -1;
    }

    auto* controller = &view->viewportController_;
    const std::size_t key = pathHash(view->document().path());

    indicesMap_[key] = controllerIndex;
    controllerMap_[controller] = controllerIndex;
    views_.push_back(std::move(view));

    return controllerIndex;
}

std::unique_ptr<DocumentView> DocumentController::extractView(DocumentId id, int* outRemovedIndex) {
    const Document* document = manager_.at(id);
    if (!document) {
        return nullptr;
    }

    const std::size_t key = pathHash(document->path());
    clearActiveTabHashIf(key);

    for (auto it = views_.begin(); it != views_.end(); ++it) {
        if (&(*it)->document() != document) {
            continue;
        }

        (*it)->editorSession_.setToolChangedCallback({});

        const auto removed = controllerMap_.find(&(*it)->viewportController_);
        const int removedIndex = (removed != controllerMap_.end()) ? removed->second : -1;
        if (outRemovedIndex) {
            *outRemovedIndex = removedIndex;
        }

        controllerMap_.erase(&(*it)->viewportController_);
        indicesMap_.erase(key);
        if (removedIndex >= 0) {
            shiftIndicesAfterRemoval(removedIndex);
        }

        auto view = std::move(*it);
        views_.erase(it);
        return view;
    }
    return nullptr;
}

void DocumentController::clearViews() {
    for (auto& view : views_) {
        if (view) {
            view->editorSession_.setToolChangedCallback({});
        }
    }
    views_.clear();
    indicesMap_.clear();
    controllerMap_.clear();
    activeTabHash_ = 0;
}

DocumentView* DocumentController::findView(const std::string& path) const {
    for (const auto& view : views_) {
        if (view && view->document().path() == path) {
            return view.get();
        }
    }
    return nullptr;
}

DocumentView* DocumentController::findViewByHash(std::size_t hash) const {
    if (hash == 0) {
        return nullptr;
    }
    for (const auto& view : views_) {
        if (view && pathHash(view->document().path()) == hash) {
            return view.get();
        }
    }
    return nullptr;
}

DocumentView* DocumentController::findViewById(DocumentId id) const {
    Document* doc = manager_.at(id);
    if (!doc) {
        return nullptr;
    }
    for (const auto& view : views_) {
        if (view && &view->document() == doc) {
            return view.get();
        }
    }
    return nullptr;
}

DocumentId DocumentController::selectTool(std::optional<DocumentId> id, ToolId tool, std::optional<double> value) {
    DocumentView* view = id ? findViewById(*id) : findViewByHash(activeTabHash_);
    if (!view) {
        return InvalidId;
    }

    view->editorSession_.selectTool(tool, value);
    view->viewportController_.requestRedraw();

    return idFromPath(view->document().path());
}

std::optional<DocumentController::ActivatedView> DocumentController::activateByPath(const std::string& path) {
    DocumentView* view = findView(path);
    if (!view) {
        return std::nullopt;
    }

    const std::size_t newHash = pathHash(path);

    if (activeTabHash_ != newHash) {
        if (auto* previous = findViewByHash(activeTabHash_)) {
            if (auto* tool = previous->editorSession_.activeTool()) {
                tool->cancel();
            }
        }
    }
    activeTabHash_ = newHash;

    auto docIt = documentIds_.find(newHash);
    auto idxIt = indicesMap_.find(newHash);
    if (docIt == documentIds_.end() || idxIt == indicesMap_.end()) {
        return std::nullopt;
    }

    return ActivatedView{docIt->second, view->editorSession_.activeToolId(), idxIt->second};
}

void DocumentController::setActiveTabHash(std::size_t hash) { activeTabHash_ = hash; }

void DocumentController::clearActiveTabHashIf(std::size_t hash) {
    if (activeTabHash_ == hash) {
        activeTabHash_ = 0;
    }
}

int DocumentController::indexOfHash(std::size_t hash) const {
    auto it = indicesMap_.find(hash);
    return it != indicesMap_.end() ? it->second : -1;
}

void DocumentController::setIndexOfHash(std::size_t hash, int index) { indicesMap_[hash] = index; }

void DocumentController::eraseHashIndex(std::size_t hash) { indicesMap_.erase(hash); }

void DocumentController::shiftIndicesAfterRemoval(int removedIndex) {
    for (auto& [n, idx] : indicesMap_) {
        if (idx > removedIndex) {
            --idx;
        }
    }
    for (auto& [c, idx] : controllerMap_) {
        if (idx > removedIndex) {
            --idx;
        }
    }
}