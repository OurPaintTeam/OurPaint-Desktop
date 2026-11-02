#include "ProjectSaveLoad.h"

#include <algorithm>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>

#include <nlohmann/json.hpp>

#include "NativeFile.h"
#include "saveload/CborCodec.h"

namespace app {
namespace {

using Json = nlohmann::json;
using core::persistence::CodecErrorCode;
using Encoded = core::persistence::CodecResult<std::vector<uint8_t>>;
using Decoded = core::persistence::CodecResult<ProjectMetadata>;

std::string generateProjectId(std::string_view excluded) {
    std::random_device source;
    std::uniform_int_distribution<unsigned> byte(0, 255);
    constexpr char Hex[] = "0123456789abcdef";
    std::string id(32, '0');
    for (unsigned attempt = 0; attempt < 16; ++attempt) {
        for (size_t i = 0; i < 16; ++i) {
            const auto value = byte(source);
            id[2 * i] = Hex[value >> 4];
            id[2 * i + 1] = Hex[value & 15];
        }
        if (id != excluded) {
            return id;
        }
    }
    throw std::runtime_error("Could not generate a distinct project ID");
}

bool validRootPath(const std::filesystem::path& root) {
    return !root.empty() && root.native().find(std::filesystem::path::value_type{}) == std::filesystem::path::string_type::npos;
}

std::filesystem::path directoryEntryPath(std::filesystem::path root) {
    // Ignore trailing separators and dot components, without collapsing '..'
    // across a possible symlink. This also exposes a final directory symlink.
    while (root.has_relative_path() && (root.filename().empty() || root.filename() == ".")) {
        root = root.parent_path();
    }
    return root.empty() ? std::filesystem::path(".") : root;
}

std::optional<FileError> validateProjectRoot(const std::filesystem::path& root) {
    const auto invalid = [&root](std::error_code error) -> std::optional<FileError> {
        return FileError{root, FileStage::ValidatePath, error, std::nullopt};
    };
    if (!validRootPath(root)) {
        return invalid(std::make_error_code(std::errc::invalid_argument));
    }
    std::error_code error;
    const auto status = std::filesystem::symlink_status(root, error);
    if (error) {
        return invalid(error);
    }
    if (!std::filesystem::is_directory(status)) {
        return invalid(std::make_error_code(std::errc::not_a_directory));
    }
    return std::nullopt;
}

std::filesystem::path normalizedRoot(const std::filesystem::path& root, std::error_code& error) {
    const auto absolute = std::filesystem::absolute(root, error);
    if (error) {
        return {};
    }
    const auto normalized = std::filesystem::weakly_canonical(absolute, error);
    return error ? std::filesystem::path{} : directoryEntryPath(normalized);
}

std::optional<FileError> validateCopyRoot(const std::filesystem::path& source, const std::filesystem::path& destination, bool& createRoot) {
    namespace fs = std::filesystem;
    const auto invalid = [&destination](std::error_code error) -> std::optional<FileError> {
        return FileError{destination, FileStage::ValidatePath, error, std::nullopt};
    };
    if (!validRootPath(destination) || (!source.empty() && !validRootPath(source))) {
        return invalid(std::make_error_code(std::errc::invalid_argument));
    }
    std::error_code error;
    const auto entry = directoryEntryPath(destination);
    const auto status = fs::symlink_status(entry, error);
    if (error && error != std::errc::no_such_file_or_directory) {
        return invalid(error);
    }
    createRoot = status.type() == fs::file_type::not_found;
    if (createRoot) {
        const auto parent = entry.has_parent_path() ? entry.parent_path() : fs::path(".");
        const auto parentStatus = fs::status(parent, error);
        if (error) {
            return invalid(error);
        }
        if (!fs::is_directory(parentStatus)) {
            return invalid(std::make_error_code(std::errc::not_a_directory));
        }
    } else {
        if (!fs::is_directory(status)) {
            return invalid(std::make_error_code(std::errc::not_a_directory));
        }
        const auto empty = fs::is_empty(destination, error);
        if (error) {
            return invalid(error);
        }
        if (!empty) {
            return invalid(std::make_error_code(std::errc::directory_not_empty));
        }
    }
    if (!source.empty()) {
        const auto targetRoot = normalizedRoot(destination, error);
        if (error) {
            return invalid(error);
        }
        const auto sourceRoot = normalizedRoot(source, error);
        if (error) {
            return invalid(error);
        }
        const detail::FilenameLess less;
        if (!less(sourceRoot, targetRoot) && !less(targetRoot, sourceRoot)) {
            return invalid(std::make_error_code(std::errc::invalid_argument));
        }
        if (!createRoot) {
            const bool same = fs::equivalent(source, destination, error);
            if (error && error != std::errc::no_such_file_or_directory) {
                return invalid(error);
            }
            if (same) {
                return invalid(std::make_error_code(std::errc::invalid_argument));
            }
        }
    }
    return std::nullopt;
}

}  // namespace

Encoded encodeProjectMetadata(const ProjectMetadata& metadata) {
    if (auto error = validateProjectMetadata(metadata)) {
        return Encoded::failure(std::move(*error));
    }
    const Json marker{{"format", "ourpaint-project"}, {"version", ProjectFormatVersion}, {"id", metadata.id}, {"name", metadata.name}};
    auto bytes = Json::to_cbor(marker);
    if (auto error = core::persistence::detail::checkCbor(bytes)) {
        return Encoded::failure(std::move(*error));
    }
    return Encoded::success(std::move(bytes));
}

Decoded decodeProjectMetadata(std::span<const uint8_t> bytes) {
    if (auto error = core::persistence::detail::checkCbor(bytes)) {
        return Decoded::failure(std::move(*error));
    }
    Json marker;
    try {
        marker = Json::from_cbor(bytes.begin(), bytes.end(), true);
    } catch (const Json::exception& error) {
        return Decoded::failure({CodecErrorCode::InvalidCbor, "$", error.what()});
    }
    for (const auto field : {"format", "version", "id", "name"}) {
        if (!marker.contains(field)) {
            return Decoded::failure({CodecErrorCode::InvalidSchema, field, "Missing required project field"});
        }
    }
    if (marker.size() != 4) {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "$", "Unknown project metadata field"});
    }
    if (marker.at("format") != "ourpaint-project") {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "format", "Expected the ourpaint-project format"});
    }
    const auto& version = marker.at("version");
    if (!version.is_number_integer() || (!version.is_number_unsigned() && version.get<int64_t>() < 0)) {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "version", "Project version must be a nonnegative integer"});
    }
    if (version != ProjectFormatVersion) {
        return Decoded::failure({CodecErrorCode::UnsupportedVersion, "version", "Unsupported project version"});
    }
    for (const auto field : {"id", "name"}) {
        if (!marker.at(field).is_string()) {
            return Decoded::failure({CodecErrorCode::InvalidSchema, field, "Expected UTF-8 text"});
        }
    }
    ProjectMetadata metadata{marker.at("id").get<std::string>(), marker.at("name").get<std::string>()};
    if (auto error = validateProjectMetadata(metadata)) {
        return Decoded::failure(std::move(*error));
    }
    return Decoded::success(std::move(metadata));
}

namespace {

// adoptedId is provided only for an ordinary save of an unsaved description.
SaveReport saveProjectImpl(const Project& project, const std::filesystem::path& root, bool copy, std::string* adoptedId) {
    SaveReport report;
    report.root = root;
    report.metadata = project.metadata;
    const auto marker = root / ".ourpaint";
    auto validation = validateProject(project);
    std::vector<const ProjectDocument*> documents;
    documents.reserve(project.documents.size());
    for (const auto& document : project.documents) {
        documents.push_back(&document);
    }
    if (!validation) {
        const detail::FilenameLess less;
        std::sort(documents.begin(), documents.end(), [&less](const auto* first, const auto* second) {
            return less(first->filename, second->filename);
        });
    }
    report.unprocessed.reserve(documents.size() + 1);
    for (const auto* document : documents) {
        report.unprocessed.push_back(root / document->filename);
    }
    report.unprocessed.push_back(marker);
    if (validation) {
        report.error = FileError{root, FileStage::ValidateProject, {}, std::move(validation)};
        return report;
    }
    bool createRoot = false;
    if (auto error = copy ? validateCopyRoot(project.root, root, createRoot) : validateProjectRoot(root)) {
        report.error = std::move(error);
        return report;
    }
    if (!createRoot) {
        for (const auto& path : report.unprocessed) {
            if (auto error = detail::validateFileDestination(path)) {
                report.error = std::move(error);
                return report;
            }
        }
    }
    const bool needsId = copy || project.metadata.id.empty();
    if (needsId) {
        try {
            report.metadata.id = generateProjectId(project.metadata.id);
        } catch (const std::system_error& error) {
            report.error = FileError{marker, FileStage::GenerateId, error.code(), std::nullopt};
            return report;
        } catch (const std::runtime_error&) {
            report.error = FileError{marker, FileStage::GenerateId, std::make_error_code(std::errc::io_error), std::nullopt};
            return report;
        }
    }
    std::vector<core::persistence::DocumentBytes> prepared;
    prepared.reserve(report.unprocessed.size());
    for (size_t i = 0; i < documents.size(); ++i) {
        auto encoded = core::persistence::encodeDocument(documents[i]->document.get());
        if (!encoded) {
            report.error = FileError{report.unprocessed[i], FileStage::Encode, {}, encoded.error()};
            return report;
        }
        prepared.push_back(std::move(encoded.value()));
    }
    auto metadata = encodeProjectMetadata(report.metadata);
    if (!metadata) {
        report.error = FileError{marker, FileStage::Encode, {}, metadata.error()};
        return report;
    }
    prepared.push_back(std::move(metadata.value()));

    // Allocate report storage and the adopted ID before the first publication.
    // Moving paths and swapping the ID below cannot allocate after replacement.
    report.saved.reserve(prepared.size());
    auto idToAdopt = adoptedId ? report.metadata.id : std::string{};
    if (createRoot) {
        std::error_code error;
        const bool created = std::filesystem::create_directory(directoryEntryPath(root), error);
        if (!created) {
            report.error = FileError{root, FileStage::CreateDirectory, error ? error : std::make_error_code(std::errc::file_exists), std::nullopt};
            return report;
        }
    }
    for (size_t i = 0; i < prepared.size(); ++i) {
        auto result = detail::replaceFile(report.unprocessed[i], prepared[i]);
        if (adoptedId && i + 1 == prepared.size() && result.replaced) {
            adoptedId->swap(idToAdopt);
        }
        if (!result) {
            report.error = std::move(result.error);
            report.failedFileReplaced = result.replaced;
            report.unprocessed.erase(report.unprocessed.begin(), report.unprocessed.begin() + static_cast<std::ptrdiff_t>(i + 1));
            return report;
        }
        report.saved.push_back(std::move(report.unprocessed[i]));
    }
    report.unprocessed.clear();
    return report;
}

}  // namespace

SaveReport saveProject(Project& project) {
    return saveProjectImpl(project, project.root, false, project.metadata.id.empty() ? &project.metadata.id : nullptr);
}

SaveReport saveProjectAs(const Project& project, const std::filesystem::path& destination) {
    return saveProjectImpl(project, destination, true, nullptr);
}

}  // namespace app
