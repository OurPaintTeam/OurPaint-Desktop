#include "ProjectSaveLoad.h"

#include <algorithm>
#include <set>

#include "saveload/CborCodec.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace app {
namespace {

using core::persistence::CodecError;
using core::persistence::CodecErrorCode;

struct FilenameLess {
    bool operator()(const std::filesystem::path& first, const std::filesystem::path& second) const {
#ifdef _WIN32
        return CompareStringOrdinal(first.c_str(), -1, second.c_str(), -1, TRUE) == CSTR_LESS_THAN;
#else
        return first.native() < second.native();
#endif
    }
};

bool validFilename(const std::filesystem::path& filename) {
    if (filename.empty() || filename.has_parent_path() || filename.filename() != filename) {
        return false;
    }
    for (const auto value : filename.native()) {
        switch (value) {
            case 0:
            case '/':
            case '\\':
            case '<':
            case '>':
            case ':':
            case '"':
            case '|':
            case '?':
            case '*':
                return false;
            default:
                break;
        }
    }
    const auto extension = filename.extension();
    const std::filesystem::path expected(".ourp");
    const FilenameLess less;
    return !less(extension, expected) && !less(expected, extension);
}

}  // namespace

std::optional<CodecError> validateProjectMetadata(const ProjectMetadata& metadata, bool allowEmptyId) {
    if (!(allowEmptyId && metadata.id.empty()) &&
        (metadata.id.size() != 32 || !std::all_of(metadata.id.begin(), metadata.id.end(), [](char value) {
             return (value >= '0' && value <= '9') || (value >= 'a' && value <= 'f');
         }))) {
        return CodecError{CodecErrorCode::InvalidSchema, "id", "Project ID must contain 32 lowercase hexadecimal digits"};
    }
    if (metadata.name.size() >= core::persistence::MaxDocumentBytes) {
        return CodecError{CodecErrorCode::LimitExceeded, "name", "Project metadata exceeds the 64 MiB limit"};
    }
    if (!core::persistence::detail::isValidUtf8(metadata.name)) {
        return CodecError{CodecErrorCode::InvalidSchema, "name", "Project name must be valid UTF-8"};
    }
    return std::nullopt;
}

std::optional<CodecError> validateProject(const Project& project) {
    if (auto error = validateProjectMetadata(project.metadata, true)) {
        error->field = "metadata." + error->field;
        return error;
    }
    std::set<std::filesystem::path, FilenameLess> filenames;
    for (size_t i = 0; i < project.documents.size(); ++i) {
        const auto& filename = project.documents[i].filename;
        const auto field = "documents[" + std::to_string(i) + "].filename";
        if (!validFilename(filename)) {
            return CodecError{CodecErrorCode::InvalidSchema, field, "Expected a single relative .ourp filename without forbidden characters"};
        }
        if (!filenames.insert(filename).second) {
            return CodecError{CodecErrorCode::InvalidSchema, field, "Duplicate document filename"};
        }
    }
    return std::nullopt;
}

}  // namespace app

