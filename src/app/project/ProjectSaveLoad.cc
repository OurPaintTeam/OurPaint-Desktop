#include "ProjectSaveLoad.h"

#include <utility>

#include <nlohmann/json.hpp>

#include "saveload/CborCodec.h"

namespace app {
namespace {

using Json = nlohmann::json;
using core::persistence::CodecErrorCode;
using Encoded = core::persistence::CodecResult<std::vector<uint8_t>>;
using Decoded = core::persistence::CodecResult<ProjectMetadata>;

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

}  // namespace app
