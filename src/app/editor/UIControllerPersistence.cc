#include <algorithm>
#include <nlohmann/json.hpp>
#include <stdexcept>

#include "NativeFile.h"
#include "UIController.h"

namespace app {
namespace {
using Path = std::filesystem::path;

FileError invalidPath(const Path& path, std::errc code = std::errc::invalid_argument) {
    return {path, FileStage::ValidatePath, std::make_error_code(code), {}};
}

Path normalized(const Path& path, std::error_code& error) {
    const auto absolute = std::filesystem::absolute(path, error);
    return error ? Path{} : std::filesystem::weakly_canonical(absolute, error);
}

bool samePath(const Path& a, const Path& b) {
    if (a.empty() || b.empty()) {
        return false;
    }
    std::error_code error;
    if (std::filesystem::equivalent(a, b, error)) {
        return true;
    }
    const detail::FilenameLess less;
    return !less(a, b) && !less(b, a);
}

bool documentExtension(const Path& path) {
    auto extension = path.extension().native();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](auto c) { return c >= 'A' && c <= 'Z' ? static_cast<decltype(c)>(c + ('a' - 'A')) : c; });
    return extension == Path(".ourp").native();
}

std::string utf8(const Path& path) {
    const auto value = path.u8string();
    return {value.begin(), value.end()};
}
}  // namespace

DocumentId UIController::addDocument(std::unique_ptr<core::Document> document, Path path) {
    auto encoded = core::persistence::encodeDocument(*document);
    if (!encoded) {
        throw std::runtime_error(encoded.error().message);
    }
    DocumentState state{std::move(path), std::move(encoded.value())};
    documents_.reserve(documents_.size() + 1);
    const auto id = manager_.openDocument(std::move(document));
    try {
        documentStates_.emplace(id, std::move(state));
    } catch (...) {
        manager_.closeDocument(id);
        throw;
    }
    documents_.push_back(id);
    return id;
}

FileResult<DocumentId> UIController::openDocument(const Path& path) {
    using Result = FileResult<DocumentId>;
    if (path.empty() || !documentExtension(path)) {
        return Result::failure(invalidPath(path));
    }
    std::error_code error;
    auto absolute = normalized(path, error);
    if (error) {
        return Result::failure({path, FileStage::ValidatePath, error, {}});
    }
    for (const auto& [id, state] : documentStates_) {
        if (samePath(state.path, absolute)) {
            rememberDocument(absolute);
            return Result::success(id);
        }
    }
    auto bytes = detail::readFile(path);
    if (!bytes) {
        return Result::failure(bytes.error());
    }
    auto decoded = core::persistence::decodeDocument(bytes.value());
    if (!decoded) {
        return Result::failure({path, FileStage::Decode, {}, decoded.error()});
    }
    const auto id = addDocument(std::move(decoded.value()), absolute);
    rememberDocument(absolute);
    return Result::success(id);
}

Path UIController::documentPath(DocumentId id) const {
    const auto found = documentStates_.find(id);
    return found == documentStates_.end() ? Path{} : found->second.path;
}

bool UIController::documentModified(DocumentId id) const {
    const auto found = documentStates_.find(id);
    const auto* document = manager_.at(id);
    if (!document || found == documentStates_.end()) {
        return false;
    }
    // Document::isDirty() tracks mutable access, not edits since the last save.
    // Keep this check at explicit user decisions until Core exposes a content revision.
    const auto encoded = core::persistence::encodeDocument(*document);
    return !encoded || encoded.value() != found->second.savedState;
}

FileResult<Path> UIController::saveDocument(DocumentId id) { return saveDocumentAs(id, documentPath(id)); }

FileResult<Path> UIController::saveDocumentAs(DocumentId id, const Path& path) {
    using Result = FileResult<Path>;
    auto* document = manager_.at(id);
    if (!document || path.empty() || !documentExtension(path)) {
        return Result::failure(invalidPath(path));
    }
    if (auto error = detail::validateFileDestination(path)) {
        return Result::failure(*error);
    }
    std::error_code error;
    auto absolute = normalized(path, error);
    if (error) {
        return Result::failure({path, FileStage::ValidatePath, error, {}});
    }
    for (const auto& [other, state] : documentStates_) {
        if (other != id && samePath(state.path, absolute)) {
            return Result::failure(invalidPath(path, std::errc::file_exists));
        }
    }
    endInteractions();
    auto encoded = core::persistence::encodeDocument(*document);
    if (!encoded) {
        return Result::failure({path, FileStage::Encode, {}, encoded.error()});
    }
    // Allocate the next state before replacing the file. A failed save must not
    // change the document's destination or claim its current contents are saved.
    DocumentState next{absolute, std::move(encoded.value())};
    const auto written = detail::replaceFile(absolute, next.savedState);
    if (!written) {
        return Result::failure(*written.error);
    }
    documentStates_.at(id) = std::move(next);
    rememberDocument(absolute);
    return Result::success(std::move(absolute));
}

std::optional<FileError> UIController::loadApplicationMetadata(const Path& path) {
    metadataPath_.clear();
    recentDocuments_.clear();
    metadataError_.reset();
    std::error_code error;
    const bool exists = std::filesystem::exists(path, error);
    if (error) {
        return metadataError_ = FileError{path, FileStage::Read, error, {}};
    }
    if (!exists) {
        metadataPath_ = path;
        return {};
    }
    const auto bytes = detail::readFile(path);
    if (!bytes) {
        return metadataError_ = bytes.error();
    }
    try {
        const auto json = nlohmann::json::parse(bytes.value());
        if (!json.is_object() || json.at("version") != 1 || !json.at("recentDocuments").is_array()) {
            throw std::runtime_error("Unsupported application metadata format");
        }
        std::vector<Path> recent;
        for (const auto& item : json.at("recentDocuments")) {
            const auto text = item.get<std::string>();
            auto value = Path(std::u8string(text.begin(), text.end()));
            if (!value.is_absolute() || !documentExtension(value)) {
                continue;
            }
            if (std::none_of(recent.begin(), recent.end(), [&](const Path& p) { return samePath(p, value); })) {
                recent.push_back(value);
            }
            if (recent.size() == 20) {
                break;
            }
        }
        recentDocuments_ = std::move(recent);
        metadataPath_ = path;
    } catch (const std::exception& exception) {
        // Do not overwrite corrupt or future-version metadata on the next save.
        metadataError_ =
            FileError{path, FileStage::Decode, {}, core::persistence::CodecError{core::persistence::CodecErrorCode::InvalidSchema, "$", exception.what()}};
    }
    return metadataError_;
}

void UIController::rememberDocument(const Path& path) {
    std::erase_if(recentDocuments_, [&](const Path& item) { return samePath(item, path); });
    recentDocuments_.insert(recentDocuments_.begin(), path);
    if (recentDocuments_.size() > 20) {
        recentDocuments_.resize(20);
    }
    if (metadataPath_.empty()) {
        return;
    }
    nlohmann::json json{{"version", 1}, {"recentDocuments", nlohmann::json::array()}};
    for (const auto& item : recentDocuments_) {
        json["recentDocuments"].push_back(utf8(item));
    }
    const auto text = json.dump(2);
    std::error_code error;
    std::filesystem::create_directories(metadataPath_.parent_path(), error);
    if (error) {
        metadataError_ = FileError{metadataPath_, FileStage::CreateDirectory, error, {}};
        return;
    }
    if (auto validation = detail::validateFileDestination(metadataPath_)) {
        metadataError_ = validation;
        return;
    }
    // Metadata failure is reported separately: it must not turn a successful CAD
    // save into a failed save or discard the user's in-memory document.
    metadataError_ = detail::replaceFile(metadataPath_, {reinterpret_cast<const uint8_t*>(text.data()), text.size()}).error;
}
}  // namespace app
