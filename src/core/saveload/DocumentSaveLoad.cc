#include "DocumentSaveLoad.h"

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <string_view>

#include <nlohmann/json.hpp>

namespace core::persistence {
namespace {

using Json = nlohmann::json;
using Encoded = CodecResult<DocumentBytes>;
using Decoded = CodecResult<std::unique_ptr<Document>>;

constexpr std::array<std::string_view, 7> DocumentFields{
    "format", "version", "name", "entities", "constraints", "last-entity-id", "last-constraint-id"};

bool validUtf8(std::string_view text) {
    for (size_t i = 0; i < text.size();) {
        const auto first = static_cast<unsigned char>(text[i++]);
        if (first < 0x80) {
            continue;
        }
        uint32_t codepoint;
        uint32_t minimum;
        size_t remaining;
        if (first >= 0xc2 && first <= 0xdf) {
            codepoint = first & 0x1f;
            minimum = 0x80;
            remaining = 1;
        } else if (first >= 0xe0 && first <= 0xef) {
            codepoint = first & 0x0f;
            minimum = 0x800;
            remaining = 2;
        } else if (first >= 0xf0 && first <= 0xf4) {
            codepoint = first & 0x07;
            minimum = 0x10000;
            remaining = 3;
        } else {
            return false;
        }
        if (remaining > text.size() - i) {
            return false;
        }
        for (size_t j = 0; j < remaining; ++j) {
            const auto next = static_cast<unsigned char>(text[i++]);
            if (next < 0x80 || next > 0xbf) {
                return false;
            }
            codepoint = (codepoint << 6) | (next & 0x3f);
        }
        if (codepoint < minimum || codepoint > 0x10ffff || (codepoint >= 0xd800 && codepoint <= 0xdfff)) {
            return false;
        }
    }
    return true;
}

std::optional<int64_t> nonnegativeInteger(const Json& value) {
    if (value.is_number_unsigned()) {
        const auto number = value.get<uint64_t>();
        if (number <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return static_cast<int64_t>(number);
        }
    } else if (value.is_number_integer()) {
        const auto number = value.get<int64_t>();
        if (number >= 0) {
            return number;
        }
    }
    return std::nullopt;
}

// Guard the supported shallow envelope before nlohmann allocates or descends.
// SAX callbacks cannot distinguish definite text from recursively chunked text.
std::optional<CodecError> checkEnvelope(std::span<const uint8_t> bytes) {
    size_t offset = 0;
    auto head = [&](uint8_t& major, uint64_t& argument) {
        if (offset == bytes.size()) {
            return false;
        }
        const auto initial = bytes[offset++];
        major = initial >> 5;
        const auto additional = initial & 0x1f;
        if (additional < 24) {
            argument = additional;
            return true;
        }
        if (additional > 27) {
            return false;
        }
        const size_t width = size_t{1} << (additional - 24);
        if (width > bytes.size() - offset) {
            return false;
        }
        argument = 0;
        for (size_t i = 0; i < width; ++i) {
            argument = (argument << 8) | bytes[offset++];
        }
        return true;
    };
    auto text = [&](uint64_t length) -> std::optional<std::string_view> {
        if (length > bytes.size() - offset) {
            return std::nullopt;
        }
        const std::string_view value(reinterpret_cast<const char*>(bytes.data() + offset), static_cast<size_t>(length));
        offset += static_cast<size_t>(length);
        return value;
    };
    auto malformed = [&](std::string field) -> std::optional<CodecError> {
        return CodecError{CodecErrorCode::InvalidCbor, std::move(field), "Invalid or truncated definite-length CBOR at byte " + std::to_string(offset)};
    };

    uint8_t major;
    uint64_t count;
    if (!head(major, count)) {
        return malformed("$");
    }
    if (major != 5 || count > DocumentFields.size()) {
        return CodecError{CodecErrorCode::InvalidSchema, "$", "Expected a document object with at most seven fields"};
    }
    std::array<bool, DocumentFields.size()> seen{};
    for (uint64_t i = 0; i < count; ++i) {
        uint64_t argument;
        if (!head(major, argument) || major != 3) {
            return malformed("$");
        }
        const auto key = text(argument);
        if (!key) {
            return malformed("$");
        }
        const auto found = std::find(DocumentFields.begin(), DocumentFields.end(), *key);
        if (found == DocumentFields.end()) {
            return CodecError{CodecErrorCode::InvalidSchema, std::string(*key), "Unknown document field"};
        }
        const auto index = static_cast<size_t>(found - DocumentFields.begin());
        if (seen[index]) {
            return CodecError{CodecErrorCode::InvalidSchema, std::string(*key), "Duplicate document field"};
        }
        seen[index] = true;
        const std::string field(*key);
        if (!head(major, argument)) {
            return malformed(field);
        }
        if (major == 3) {
            const auto value = text(argument);
            if (!value) {
                return malformed(field);
            }
            if (!validUtf8(*value)) {
                return CodecError{CodecErrorCode::InvalidSchema, field, "Text must be valid UTF-8"};
            }
        } else if (major == 4 && (field == "entities" || field == "constraints")) {
            if (argument != 0) {
                return CodecError{CodecErrorCode::UnsupportedData, field, "Nonempty " + field + " are not supported by this codec yet"};
            }
        } else if (major != 0 && major != 7) {
            return CodecError{CodecErrorCode::InvalidSchema, field, "Unsupported CBOR type in document envelope"};
        }
    }
    if (offset != bytes.size()) {
        return malformed("$");
    }
    return std::nullopt;
}

}  // namespace

Encoded encodeDocument(const Document& document) {
    if (document.name().size() >= MaxDocumentBytes) {
        return Encoded::failure({CodecErrorCode::LimitExceeded, "name", "Encoded document exceeds the 64 MiB limit"});
    }
    if (!validUtf8(document.name())) {
        return Encoded::failure({CodecErrorCode::InvalidSchema, "name", "Document name must be valid UTF-8"});
    }
    if (document.sketch().entityCount() != 0) {
        return Encoded::failure({CodecErrorCode::UnsupportedData, "entities", "Geometry encoding is not supported by this codec yet"});
    }
    if (document.sketch().constraintCount() != 0) {
        return Encoded::failure({CodecErrorCode::UnsupportedData, "constraints", "Constraint encoding is not supported by this codec yet"});
    }
    const auto snapshot = document.sketch().snapshot();
    if (!snapshot) {
        return Encoded::failure({CodecErrorCode::InvalidModel, "$", snapshot.error().message});
    }
    const Json envelope{{"format", "ourpaint-document"},
                        {"version", DocumentFormatVersion},
                        {"name", document.name()},
                        {"entities", Json::array()},
                        {"constraints", Json::array()},
                        {"last-entity-id", snapshot.value().lastEntityId},
                        {"last-constraint-id", snapshot.value().lastConstraintId}};
    auto bytes = Json::to_cbor(envelope);
    if (bytes.size() > MaxDocumentBytes) {
        return Encoded::failure({CodecErrorCode::LimitExceeded, "$", "Encoded document exceeds the 64 MiB limit"});
    }
    return Encoded::success(std::move(bytes));
}

Decoded decodeDocument(std::span<const uint8_t> bytes) {
    if (bytes.size() > MaxDocumentBytes) {
        return Decoded::failure({CodecErrorCode::LimitExceeded, "$", "Input exceeds the 64 MiB limit"});
    }
    if (auto error = checkEnvelope(bytes)) {
        return Decoded::failure(std::move(*error));
    }
    Json envelope;
    try {
        envelope = Json::from_cbor(bytes.begin(), bytes.end(), true);
    } catch (const Json::exception& error) {
        return Decoded::failure({CodecErrorCode::InvalidCbor, "$", error.what()});
    }
    for (const auto field : DocumentFields) {
        if (!envelope.contains(field)) {
            return Decoded::failure({CodecErrorCode::InvalidSchema, std::string(field), "Missing required document field"});
        }
    }
    if (envelope.at("format") != "ourpaint-document") {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "format", "Expected the ourpaint-document format"});
    }
    const auto version = nonnegativeInteger(envelope.at("version"));
    if (!version) {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "version", "Version must be a nonnegative signed-64-bit-compatible integer"});
    }
    if (*version != DocumentFormatVersion) {
        return Decoded::failure({CodecErrorCode::UnsupportedVersion, "version", "Unsupported document version: " + std::to_string(*version)});
    }
    if (!envelope.at("name").is_string()) {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "name", "Document name must be text"});
    }
    for (const auto field : {"entities", "constraints"}) {
        if (!envelope.at(field).is_array()) {
            return Decoded::failure({CodecErrorCode::InvalidSchema, field, "Expected an array"});
        }
    }
    sketch::SketchSnapshot snapshot;
    const auto lastEntityId = nonnegativeInteger(envelope.at("last-entity-id"));
    const auto lastConstraintId = nonnegativeInteger(envelope.at("last-constraint-id"));
    if (!lastEntityId) {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "last-entity-id", "ID watermark must be a nonnegative signed-64-bit integer"});
    }
    if (!lastConstraintId) {
        return Decoded::failure({CodecErrorCode::InvalidSchema, "last-constraint-id", "ID watermark must be a nonnegative signed-64-bit integer"});
    }
    snapshot.lastEntityId = *lastEntityId;
    snapshot.lastConstraintId = *lastConstraintId;
    auto document = std::make_unique<Document>(envelope.at("name").get<std::string>());
    const auto restored = document->sketch().replaceState(snapshot);
    if (!restored) {
        return Decoded::failure({CodecErrorCode::InvalidModel, "$", restored.error().message});
    }
    return Decoded::success(std::move(document));
}

}  // namespace core::persistence
