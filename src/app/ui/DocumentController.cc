#include "DocumentController.h"

#include <fstream>
#include <system_error>

#include "DocumentSaveLoad.h"
#include "NativeFile.h"

namespace {
bool writeFileBytes(const std::filesystem::path& path,
                    const core::persistence::DocumentBytes& bytes) {
    if (path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) return false;
    if (!bytes.empty()) {
        file.write(reinterpret_cast<const char*>(bytes.data()),
                   static_cast<std::streamsize>(bytes.size()));
    }
    return file.good();
}
} // namespace

DocumentController::DocumentController(DocumentManager& manager)
    : manager_(manager) {}

DocumentId DocumentController::loadDocument(const std::string& path) {
    const auto key = pathHash(path);
    if (const auto it = documentIds_.find(key); it != documentIds_.end()) {
        return it->second;
    }

    auto bytes = app::detail::readFile(std::filesystem::path(path));
    if (!bytes) return InvalidId;

    auto decoded = core::persistence::decodeDocument(bytes.value());
    if (!decoded) return InvalidId;

    std::string name = std::as_const(*decoded.value()).name();
    if (name.empty()) name = std::filesystem::path(path).stem().string();
    if (name.empty()) name = "Sketch";

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
    if (name.empty()) name = "Sketch";

    const auto id = manager_.createNewDocument(name);

    if (auto* doc = manager_.at(id)) {
        doc->path() = path;
    }

    documentIds_[key] = id;
    return id;
}

bool DocumentController::saveDocument(DocumentId id) {
    const auto* doc = manager_.at(id);
    if (!doc) return false;
    const auto path = doc->path();
    if (path.empty()) return false;
    return saveDocumentAs(id, path);
}

bool DocumentController::saveDocumentAs(DocumentId id, const std::string& newPath) {
    if (newPath.empty()) return false;

    auto* doc = manager_.at(id);
    if (!doc) return false;

    const auto encoded = core::persistence::encodeDocument(*doc);
    if (!encoded) return false;

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

    return  manager_.closeDocument(id);
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

bool DocumentController::isOpened(DocumentId id) const {
    return manager_.at(id) != nullptr;
}

std::size_t DocumentController::pathHash(const std::string& path) const {
    std::error_code ec;
    const auto canonical = std::filesystem::weakly_canonical(path, ec);
    const std::string normalized = ec ? path : canonical.generic_string();
    return std::hash<std::string>{}(normalized);
}


bool DocumentController::renameDocument(DocumentId id, const std::string& newName) {
    auto* doc = manager_.at(id);
    if (!doc) return false;

    const std::string oldPath = doc->path();
    if (oldPath.empty() || newName.empty()) return false;

    std::filesystem::path oldFsPath(oldPath);
    std::filesystem::path newFsPath = oldFsPath.parent_path() / (newName + oldFsPath.extension().string());
    const std::string newPath = newFsPath.string();

    if (oldPath == newPath) return true;

    if (std::filesystem::exists(newFsPath)) {
        return false;
    }

    std::error_code ec;
    std::filesystem::rename(oldFsPath, newFsPath, ec);
    if (ec) {
        return false;
    }

    documentIds_.erase(pathHash(oldPath));
    documentIds_[pathHash(newPath)] = id;

    doc->path() = newPath;
    doc->name() = newName;

    return true;
}