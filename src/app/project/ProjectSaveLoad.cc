#include "ProjectSaveLoad.h"

#include <algorithm>
#include <cstddef>
#include <random>
#include <stdexcept>
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

std::string generateProjectId() {
    std::random_device source;
    std::uniform_int_distribution<unsigned> byte(0, 255);
    constexpr char Hex[] = "0123456789abcdef";
    std::string id(32, '0');
    for (size_t i = 0; i < 16; ++i) {
        const auto value = byte(source);
        id[2 * i] = Hex[value >> 4];
        id[2 * i + 1] = Hex[value & 15];
    }
    return id;
}

std::optional<FileError> validateProjectRoot(const std::filesystem::path& root) {
    const auto invalid = [&root](std::error_code error) -> std::optional<FileError> {
        return FileError{root, FileStage::ValidatePath, error, std::nullopt};
    };
    if (root.empty() || root.native().find(std::filesystem::path::value_type{}) != std::filesystem::path::string_type::npos) {
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

SaveReport saveProject(Project& project) {
    SaveReport report;
    report.root = project.root;
    report.metadata = project.metadata;
    const auto marker = project.root / ".ourpaint";
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
        report.unprocessed.push_back(project.root / document->filename);
    }
    report.unprocessed.push_back(marker);
    if (validation) {
        report.error = FileError{project.root, FileStage::ValidateProject, {}, std::move(validation)};
        return report;
    }
    if (auto error = validateProjectRoot(project.root)) {
        report.error = std::move(error);
        return report;
    }
    for (const auto& path : report.unprocessed) {
        if (auto error = detail::validateFileDestination(path)) {
            report.error = std::move(error);
            return report;
        }
    }
    const bool needsId = project.metadata.id.empty();
    if (needsId) {
        try {
            report.metadata.id = generateProjectId();
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
    auto adoptedId = needsId ? report.metadata.id : std::string{};
    for (size_t i = 0; i < prepared.size(); ++i) {
        auto result = detail::replaceFile(report.unprocessed[i], prepared[i]);
        if (needsId && i + 1 == prepared.size() && result.replaced) {
            project.metadata.id.swap(adoptedId);
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

}  // namespace app
