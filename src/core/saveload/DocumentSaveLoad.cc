#include "DocumentSaveLoad.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <limits>
#include <optional>
#include <set>
#include <string_view>
#include <type_traits>

#include <nlohmann/json.hpp>

namespace core::persistence {
namespace {

using Json = nlohmann::json;
using Encoded = CodecResult<DocumentBytes>;
using Decoded = CodecResult<std::unique_ptr<Document>>;

constexpr size_t MaxCborDepth = 64;
constexpr size_t MaxCborItems = 1'000'000;

constexpr std::array<std::pair<sketch::ConstraintType, std::string_view>, 18> ConstraintNames{{
    {sketch::ConstraintType::Coincident, "coincident"},
    {sketch::ConstraintType::Horizontal, "horizontal"},
    {sketch::ConstraintType::Vertical, "vertical"},
    {sketch::ConstraintType::Parallel, "parallel"},
    {sketch::ConstraintType::Perpendicular, "perpendicular"},
    {sketch::ConstraintType::Tangent, "tangent"},
    {sketch::ConstraintType::Equal, "equal"},
    {sketch::ConstraintType::Fix, "fix"},
    {sketch::ConstraintType::Concentric, "concentric"},
    {sketch::ConstraintType::Symmetric, "symmetric"},
    {sketch::ConstraintType::Midpoint, "midpoint"},
    {sketch::ConstraintType::Distance, "distance"},
    {sketch::ConstraintType::HorizontalDistance, "horizontal-distance"},
    {sketch::ConstraintType::VerticalDistance, "vertical-distance"},
    {sketch::ConstraintType::Length, "length"},
    {sketch::ConstraintType::Radius, "radius"},
    {sketch::ConstraintType::Diameter, "diameter"},
    {sketch::ConstraintType::Angle, "angle"}}};

constexpr std::array<std::pair<sketch::SubElement, std::string_view>, 5> SubElementNames{{
    {sketch::SubElement::Whole, "whole"},
    {sketch::SubElement::Start, "start"},
    {sketch::SubElement::End, "end"},
    {sketch::SubElement::Center, "center"},
    {sketch::SubElement::Control, "control"}}};

constexpr std::array<std::pair<sketch::TangencyBranch, std::string_view>, 5> BranchNames{{
    {sketch::TangencyBranch::PositiveSide, "positive-side"},
    {sketch::TangencyBranch::NegativeSide, "negative-side"},
    {sketch::TangencyBranch::External, "external"},
    {sketch::TangencyBranch::FirstContainsSecond, "first-contains-second"},
    {sketch::TangencyBranch::SecondContainsFirst, "second-contains-first"}}};

template <class T, size_t N>
std::optional<std::string_view> enumName(T value, const std::array<std::pair<T, std::string_view>, N>& names) {
    for (const auto& [candidate, name] : names) {
        if (candidate == value) {
            return name;
        }
    }
    return std::nullopt;
}

template <class T, size_t N>
CodecResult<T> readEnum(const Json& value, const std::array<std::pair<T, std::string_view>, N>& names, const std::string& path) {
    if (!value.is_string()) {
        return CodecResult<T>::failure({CodecErrorCode::InvalidSchema, path, "Expected a type name"});
    }
    const auto& text = value.get_ref<const std::string&>();
    for (const auto& [candidate, name] : names) {
        if (name == text) {
            return CodecResult<T>::success(candidate);
        }
    }
    return CodecResult<T>::failure({CodecErrorCode::UnsupportedData, path, "Unsupported type name: " + text});
}

std::string fieldPath(std::string_view parent, std::string_view field) {
    return parent == "$" ? std::string(field) : std::string(parent) + "." + std::string(field);
}

std::optional<CodecError> checkFields(const Json& object, std::initializer_list<std::string_view> fields, const std::string& path,
                                    std::initializer_list<std::string_view> optionalFields = {}) {
    if (!object.is_object()) {
        return CodecError{CodecErrorCode::InvalidSchema, path, "Expected an object"};
    }
    for (const auto field : fields) {
        if (!object.contains(field)) {
            return CodecError{CodecErrorCode::InvalidSchema, fieldPath(path, field), "Missing required field"};
        }
    }
    for (auto it = object.begin(); it != object.end(); ++it) {
        if (std::find(fields.begin(), fields.end(), it.key()) == fields.end() &&
            std::find(optionalFields.begin(), optionalFields.end(), it.key()) == optionalFields.end()) {
            return CodecError{CodecErrorCode::InvalidSchema, fieldPath(path, it.key()), "Unknown field"};
        }
    }
    return std::nullopt;
}

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

// Bound CBOR structure before nlohmann allocates or descends into records.
// SAX callbacks cannot distinguish definite text from recursively chunked text.
std::optional<CodecError> checkEnvelope(std::span<const uint8_t> bytes) {
    size_t offset = 0;
    size_t items = 0;
    auto head = [&](uint8_t& major, uint8_t& additional, uint64_t& argument) {
        if (offset == bytes.size()) {
            return false;
        }
        const auto initial = bytes[offset++];
        major = initial >> 5;
        additional = initial & 0x1f;
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

    auto scan = [&](auto&& self, size_t depth, const std::string& path) -> std::optional<CodecError> {
        if (depth >= MaxCborDepth || items == MaxCborItems) {
            return CodecError{CodecErrorCode::LimitExceeded, path, "CBOR depth or item limit exceeded"};
        }
        ++items;
        uint8_t major;
        uint8_t additional;
        uint64_t argument;
        if (!head(major, additional, argument)) {
            return malformed(path);
        }
        if (depth == 0 && major != 5) {
            return CodecError{CodecErrorCode::InvalidSchema, path, "Document root must be an object"};
        }
        if (major == 0 || major == 1) {
            // nlohmann stores negative integers in int64_t; reject CBOR values
            // outside that range before they could wrap during conversion.
            if (major == 1 && argument > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
                return CodecError{CodecErrorCode::InvalidSchema, path, "Negative integer exceeds the signed-64-bit range"};
            }
            return std::nullopt;
        }
        if (major == 3) {
            const auto value = text(argument);
            if (!value) {
                return malformed(path);
            }
            if (!validUtf8(*value)) {
                return CodecError{CodecErrorCode::InvalidSchema, path, "Text must be valid UTF-8"};
            }
            return std::nullopt;
        }
        if (major == 7 && (additional == 20 || additional == 21 || additional == 22 || additional == 25 || additional == 26 || additional == 27)) {
            return std::nullopt;
        }
        if (major != 4 && major != 5) {
            return CodecError{CodecErrorCode::InvalidSchema, path, "Unsupported CBOR type in document"};
        }
        const size_t perElement = major == 5 ? 2 : 1;
        if (argument > (MaxCborItems - items) / perElement) {
            return CodecError{CodecErrorCode::LimitExceeded, path, "CBOR item limit exceeded"};
        }
        std::set<std::string_view> keys;
        for (uint64_t i = 0; i < argument; ++i) {
            std::string child;
            if (major == 5) {
                uint8_t keyMajor;
                uint8_t keyAdditional;
                uint64_t keyLength;
                if (items == MaxCborItems) {
                    return CodecError{CodecErrorCode::LimitExceeded, path, "CBOR item limit exceeded"};
                }
                ++items;
                if (!head(keyMajor, keyAdditional, keyLength) || keyMajor != 3) {
                    return malformed(path);
                }
                // All schema keys fit this bound. Reject oversized unknown keys
                // before repeatedly copying them into nested diagnostic paths.
                if (keyLength > std::string_view("last-constraint-id").size()) {
                    return CodecError{CodecErrorCode::InvalidSchema, path, "Unknown oversized field name"};
                }
                const auto key = text(keyLength);
                if (!key) {
                    return malformed(path);
                }
                if (!validUtf8(*key)) {
                    return CodecError{CodecErrorCode::InvalidSchema, path, "Object keys must be valid UTF-8"};
                }
                child = fieldPath(path, *key);
                if (!keys.insert(*key).second) {
                    return CodecError{CodecErrorCode::InvalidSchema, child, "Duplicate field"};
                }
            } else {
                child = path + "[" + std::to_string(i) + "]";
            }
            if (auto error = self(self, depth + 1, child)) {
                return error;
            }
        }
        return std::nullopt;
    };
    if (auto error = scan(scan, 0, "$")) {
        return error;
    }
    if (offset != bytes.size()) {
        return malformed("$");
    }
    return std::nullopt;
}

CodecResult<double> readNumber(const Json& value, const std::string& path) {
    using Number = CodecResult<double>;
    if (!value.is_number()) {
        return Number::failure({CodecErrorCode::InvalidSchema, path, "Expected a finite number"});
    }
    const double number = value.get<double>();
    if (!std::isfinite(number)) {
        return Number::failure({CodecErrorCode::InvalidSchema, path, "Expected a finite number"});
    }
    return Number::success(number);
}

CodecResult<sketch::Vec2> readPosition(const Json& value, const std::string& path) {
    using Position = CodecResult<sketch::Vec2>;
    if (!value.is_array() || value.size() != 2) {
        return Position::failure({CodecErrorCode::InvalidSchema, path, "Expected a two-number position"});
    }
    const auto x = readNumber(value[0], path + "[0]");
    if (!x) {
        return Position::failure(x.error());
    }
    const auto y = readNumber(value[1], path + "[1]");
    if (!y) {
        return Position::failure(y.error());
    }
    return Position::success({x.value(), y.value()});
}

CodecResult<std::vector<double>> readNumbers(const Json& value, const std::string& path) {
    using Numbers = CodecResult<std::vector<double>>;
    if (!value.is_array()) {
        return Numbers::failure({CodecErrorCode::InvalidSchema, path, "Expected an array of finite numbers"});
    }
    std::vector<double> numbers;
    numbers.reserve(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        const auto number = readNumber(value[i], path + "[" + std::to_string(i) + "]");
        if (!number) {
            return Numbers::failure(number.error());
        }
        numbers.push_back(number.value());
    }
    return Numbers::success(std::move(numbers));
}

CodecResult<sketch::SketchGeometry> readGeometry(const Json& record, const std::string& path) {
    using Geometry = CodecResult<sketch::SketchGeometry>;
    if (!record.is_object() || !record.contains("type") || !record.at("type").is_string()) {
        return Geometry::failure({CodecErrorCode::InvalidSchema, path + ".type", "Geometry must have a text type"});
    }
    const auto& type = record.at("type").get_ref<const std::string&>();
    if (type == "point") {
        if (auto error = checkFields(record, {"type", "position"}, path)) {
            return Geometry::failure(std::move(*error));
        }
        const auto position = readPosition(record.at("position"), path + ".position");
        if (!position) {
            return Geometry::failure(position.error());
        }
        return Geometry::success(sketch::Point2{position.value()});
    }
    if (type == "line") {
        if (auto error = checkFields(record, {"type", "start", "end"}, path)) {
            return Geometry::failure(std::move(*error));
        }
        const auto start = readPosition(record.at("start"), path + ".start");
        if (!start) {
            return Geometry::failure(start.error());
        }
        const auto end = readPosition(record.at("end"), path + ".end");
        if (!end) {
            return Geometry::failure(end.error());
        }
        return Geometry::success(sketch::Line2{start.value(), end.value()});
    }
    if (type == "circle") {
        if (auto error = checkFields(record, {"type", "center", "radius"}, path)) {
            return Geometry::failure(std::move(*error));
        }
        const auto center = readPosition(record.at("center"), path + ".center");
        if (!center) {
            return Geometry::failure(center.error());
        }
        const auto radius = readNumber(record.at("radius"), path + ".radius");
        if (!radius) {
            return Geometry::failure(radius.error());
        }
        return Geometry::success(sketch::Circle2{center.value(), radius.value()});
    }
    if (type == "arc") {
        if (auto error = checkFields(record, {"type", "center", "start", "end"}, path)) {
            return Geometry::failure(std::move(*error));
        }
        const auto center = readPosition(record.at("center"), path + ".center");
        if (!center) {
            return Geometry::failure(center.error());
        }
        const auto start = readPosition(record.at("start"), path + ".start");
        if (!start) {
            return Geometry::failure(start.error());
        }
        const auto end = readPosition(record.at("end"), path + ".end");
        if (!end) {
            return Geometry::failure(end.error());
        }
        return Geometry::success(sketch::Arc2{center.value(), start.value(), end.value()});
    }
    if (type == "nurbs") {
        if (auto error = checkFields(record, {"type", "degree", "controls", "weights", "knots"}, path)) {
            return Geometry::failure(std::move(*error));
        }
        const auto degree = nonnegativeInteger(record.at("degree"));
        if (!degree || *degree == 0 || *degree > std::numeric_limits<int>::max()) {
            return Geometry::failure({CodecErrorCode::InvalidSchema, path + ".degree", "NURBS degree must be a positive int"});
        }
        for (const auto field : {"controls", "weights", "knots"}) {
            if (!record.at(field).is_array()) {
                return Geometry::failure({CodecErrorCode::InvalidSchema, fieldPath(path, field), "Expected an array"});
            }
        }
        const auto& controls = record.at("controls");
        const auto count = controls.size();
        const auto degreeValue = static_cast<size_t>(*degree);
        if (count <= degreeValue || record.at("weights").size() != count || count > std::numeric_limits<size_t>::max() - degreeValue - 1 ||
            record.at("knots").size() != count + degreeValue + 1) {
            return Geometry::failure({CodecErrorCode::InvalidSchema, path, "NURBS control, weight and expanded knot counts do not match the degree"});
        }
        sketch::NurbsCurve2 curve;
        curve.degree = static_cast<int>(*degree);
        curve.controlPoints.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            const auto control = readPosition(controls[i], path + ".controls[" + std::to_string(i) + "]");
            if (!control) {
                return Geometry::failure(control.error());
            }
            curve.controlPoints.push_back({control.value()});
        }
        auto weights = readNumbers(record.at("weights"), path + ".weights");
        if (!weights) {
            return Geometry::failure(weights.error());
        }
        auto knots = readNumbers(record.at("knots"), path + ".knots");
        if (!knots) {
            return Geometry::failure(knots.error());
        }
        curve.weights = std::move(weights.value());
        curve.knots = std::move(knots.value());
        return Geometry::success(std::move(curve));
    }
    return Geometry::failure({CodecErrorCode::UnsupportedData, path + ".type", "Unsupported geometry type: " + type});
}

CodecResult<sketch::SketchEntity> readEntity(const Json& record, const std::string& path) {
    using Entity = CodecResult<sketch::SketchEntity>;
    if (auto error = checkFields(record, {"id", "construction", "geometry"}, path)) {
        return Entity::failure(std::move(*error));
    }
    const auto id = nonnegativeInteger(record.at("id"));
    if (!id || *id == 0) {
        return Entity::failure({CodecErrorCode::InvalidSchema, path + ".id", "Entity ID must be a positive signed-64-bit integer"});
    }
    if (!record.at("construction").is_boolean()) {
        return Entity::failure({CodecErrorCode::InvalidSchema, path + ".construction", "Construction flag must be boolean"});
    }
    auto geometry = readGeometry(record.at("geometry"), path + ".geometry");
    if (!geometry) {
        return Entity::failure(geometry.error());
    }
    return Entity::success({sketch::EntityId(*id), std::move(geometry.value()), record.at("construction").get<bool>()});
}

Json writePosition(sketch::Vec2 position) { return Json::array({position.x, position.y}); }

std::optional<CodecError> checkReference(const sketch::GeometryRef& ref, const std::string& path) {
    if (ref.entity.get() <= 0) {
        return CodecError{CodecErrorCode::InvalidSchema, path + ".entity", "Reference entity must be a positive signed-64-bit integer"};
    }
    if (ref.index > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
        return CodecError{CodecErrorCode::InvalidSchema, path + ".index", "Reference index exceeds the signed-64-bit range"};
    }
    if (ref.sub != sketch::SubElement::Control && ref.index != 0) {
        return CodecError{CodecErrorCode::InvalidSchema, path + ".index", "Only control references may have a nonzero index"};
    }
    return std::nullopt;
}

CodecResult<sketch::GeometryRef> readReference(const Json& record, const std::string& path) {
    using Reference = CodecResult<sketch::GeometryRef>;
    if (auto error = checkFields(record, {"entity", "sub", "index"}, path)) {
        return Reference::failure(std::move(*error));
    }
    const auto entity = nonnegativeInteger(record.at("entity"));
    if (!entity || *entity == 0) {
        return Reference::failure({CodecErrorCode::InvalidSchema, path + ".entity", "Reference entity must be a positive signed-64-bit integer"});
    }
    const auto sub = readEnum(record.at("sub"), SubElementNames, path + ".sub");
    if (!sub) {
        return Reference::failure(sub.error());
    }
    const auto index = nonnegativeInteger(record.at("index"));
    if (!index || static_cast<uint64_t>(*index) > std::numeric_limits<size_t>::max()) {
        return Reference::failure({CodecErrorCode::InvalidSchema, path + ".index", "Reference index must fit both int64_t and size_t"});
    }
    const sketch::GeometryRef ref{sketch::EntityId(*entity), sub.value(), static_cast<size_t>(*index)};
    if (auto error = checkReference(ref, path)) {
        return Reference::failure(std::move(*error));
    }
    return Reference::success(ref);
}

std::optional<CodecError> checkDefinitionFields(const sketch::ConstraintDefinition& definition, const std::string& path) {
    bool dimensional = false;
    switch (definition.type) {
        case sketch::ConstraintType::Distance:
        case sketch::ConstraintType::HorizontalDistance:
        case sketch::ConstraintType::VerticalDistance:
        case sketch::ConstraintType::Length:
        case sketch::ConstraintType::Radius:
        case sketch::ConstraintType::Diameter:
        case sketch::ConstraintType::Angle:
            dimensional = true;
            break;
        default:
            break;
    }
    if (definition.value.has_value() != dimensional || (definition.value && !std::isfinite(*definition.value))) {
        return CodecError{CodecErrorCode::InvalidSchema, path + ".value", "Only dimensional constraints require a finite value"};
    }
    if (definition.fixedPosition && (definition.type != sketch::ConstraintType::Fix || !std::isfinite(definition.fixedPosition->x) ||
                                     !std::isfinite(definition.fixedPosition->y))) {
        return CodecError{CodecErrorCode::InvalidSchema, path + ".fixed-position", "Only Fix may contain a finite fixed position"};
    }
    // Saved tangency intent must not be re-inferred from coordinates on loading.
    if (definition.tangencyBranch.has_value() != (definition.type == sketch::ConstraintType::Tangent)) {
        return CodecError{CodecErrorCode::InvalidSchema, path + ".tangency-branch", "Tangent requires an explicit branch; other types cannot contain one"};
    }
    return std::nullopt;
}

CodecResult<sketch::SketchConstraint> readConstraint(const Json& record, const std::string& path) {
    using Constraint = CodecResult<sketch::SketchConstraint>;
    if (auto error = checkFields(record, {"id", "type", "refs"}, path, {"value", "fixed-position", "tangency-branch"})) {
        return Constraint::failure(std::move(*error));
    }
    const auto id = nonnegativeInteger(record.at("id"));
    if (!id || *id == 0) {
        return Constraint::failure({CodecErrorCode::InvalidSchema, path + ".id", "Constraint ID must be a positive signed-64-bit integer"});
    }
    const auto type = readEnum(record.at("type"), ConstraintNames, path + ".type");
    if (!type) {
        return Constraint::failure(type.error());
    }
    if (!record.at("refs").is_array()) {
        return Constraint::failure({CodecErrorCode::InvalidSchema, path + ".refs", "Expected an ordered reference array"});
    }
    sketch::ConstraintDefinition definition;
    definition.type = type.value();
    const auto& refs = record.at("refs");
    definition.refs.reserve(refs.size());
    for (size_t i = 0; i < refs.size(); ++i) {
        const auto ref = readReference(refs[i], path + ".refs[" + std::to_string(i) + "]");
        if (!ref) {
            return Constraint::failure(ref.error());
        }
        definition.refs.push_back(ref.value());
    }
    if (record.contains("value")) {
        const auto value = readNumber(record.at("value"), path + ".value");
        if (!value) {
            return Constraint::failure(value.error());
        }
        definition.value = value.value();
    }
    if (record.contains("fixed-position")) {
        const auto position = readPosition(record.at("fixed-position"), path + ".fixed-position");
        if (!position) {
            return Constraint::failure(position.error());
        }
        definition.fixedPosition = position.value();
    }
    if (record.contains("tangency-branch")) {
        const auto branch = readEnum(record.at("tangency-branch"), BranchNames, path + ".tangency-branch");
        if (!branch) {
            return Constraint::failure(branch.error());
        }
        definition.tangencyBranch = branch.value();
    }
    if (auto error = checkDefinitionFields(definition, path)) {
        return Constraint::failure(std::move(*error));
    }
    return Constraint::success({sketch::ConstraintId(*id), std::move(definition)});
}

CodecResult<Json> writeConstraint(const sketch::SketchConstraint& constraint, const std::string& path) {
    using Record = CodecResult<Json>;
    const auto& definition = constraint.definition;
    const auto type = enumName(definition.type, ConstraintNames);
    if (!type) {
        return Record::failure({CodecErrorCode::UnsupportedData, path + ".type", "Constraint type has no format name"});
    }
    if (auto error = checkDefinitionFields(definition, path)) {
        return Record::failure(std::move(*error));
    }
    Json refs = Json::array();
    for (size_t i = 0; i < definition.refs.size(); ++i) {
        const auto& ref = definition.refs[i];
        const auto refPath = path + ".refs[" + std::to_string(i) + "]";
        if (auto error = checkReference(ref, refPath)) {
            return Record::failure(std::move(*error));
        }
        const auto sub = enumName(ref.sub, SubElementNames);
        if (!sub) {
            return Record::failure({CodecErrorCode::UnsupportedData, refPath + ".sub", "Sub-element has no format name"});
        }
        refs.push_back({{"entity", ref.entity.get()}, {"sub", std::string(*sub)}, {"index", static_cast<uint64_t>(ref.index)}});
    }
    Json record{{"id", constraint.id.get()}, {"type", std::string(*type)}, {"refs", std::move(refs)}};
    if (definition.value) {
        record["value"] = *definition.value;
    }
    if (definition.fixedPosition) {
        record["fixed-position"] = writePosition(*definition.fixedPosition);
    }
    if (definition.tangencyBranch) {
        const auto branch = enumName(*definition.tangencyBranch, BranchNames);
        if (!branch) {
            return Record::failure({CodecErrorCode::UnsupportedData, path + ".tangency-branch", "Tangency branch has no format name"});
        }
        record["tangency-branch"] = std::string(*branch);
    }
    return Record::success(std::move(record));
}

Json writeGeometry(const sketch::SketchGeometry& geometry) {
    return std::visit(
        [](const auto& value) -> Json {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, sketch::Point2>) {
                return {{"type", "point"}, {"position", writePosition(value.position)}};
            } else if constexpr (std::is_same_v<T, sketch::Line2>) {
                return {{"type", "line"}, {"start", writePosition(value.start)}, {"end", writePosition(value.end)}};
            } else if constexpr (std::is_same_v<T, sketch::Circle2>) {
                return {{"type", "circle"}, {"center", writePosition(value.center)}, {"radius", value.radius}};
            } else if constexpr (std::is_same_v<T, sketch::Arc2>) {
                return {{"type", "arc"}, {"center", writePosition(value.center)}, {"start", writePosition(value.start)}, {"end", writePosition(value.end)}};
            } else {
                Json controls = Json::array();
                for (const auto& control : value.controlPoints) {
                    controls.push_back(writePosition(control.position));
                }
                return {{"type", "nurbs"}, {"degree", value.degree}, {"controls", std::move(controls)}, {"weights", value.weights}, {"knots", value.knots}};
            }
        },
        geometry);
}

}  // namespace

Encoded encodeDocument(const Document& document) {
    if (document.name().size() >= MaxDocumentBytes) {
        return Encoded::failure({CodecErrorCode::LimitExceeded, "name", "Encoded document exceeds the 64 MiB limit"});
    }
    if (!validUtf8(document.name())) {
        return Encoded::failure({CodecErrorCode::InvalidSchema, "name", "Document name must be valid UTF-8"});
    }
    const auto snapshot = document.sketch().snapshot();
    if (!snapshot) {
        return Encoded::failure({CodecErrorCode::InvalidModel, "$", snapshot.error().message});
    }
    // A failed solver iterate must not become a document that cannot be restored.
    sketch::Sketch validation;
    const auto valid = validation.replaceState(snapshot.value());
    if (!valid) {
        return Encoded::failure({CodecErrorCode::InvalidModel, "$", valid.error().message});
    }
    Json entities = Json::array();
    for (const auto& entity : snapshot.value().entities) {
        entities.push_back({{"id", entity.id.get()}, {"construction", entity.construction}, {"geometry", writeGeometry(entity.geometry)}});
    }
    Json constraints = Json::array();
    for (size_t i = 0; i < snapshot.value().constraints.size(); ++i) {
        auto record = writeConstraint(snapshot.value().constraints[i], "constraints[" + std::to_string(i) + "]");
        if (!record) {
            return Encoded::failure(record.error());
        }
        constraints.push_back(std::move(record.value()));
    }
    const Json envelope{{"format", "ourpaint-document"},
                        {"version", DocumentFormatVersion},
                        {"name", document.name()},
                        {"entities", std::move(entities)},
                        {"constraints", std::move(constraints)},
                        {"last-entity-id", snapshot.value().lastEntityId},
                        {"last-constraint-id", snapshot.value().lastConstraintId}};
    auto bytes = Json::to_cbor(envelope);
    if (bytes.size() > MaxDocumentBytes) {
        return Encoded::failure({CodecErrorCode::LimitExceeded, "$", "Encoded document exceeds the 64 MiB limit"});
    }
    if (auto error = checkEnvelope(bytes)) {
        return Encoded::failure(std::move(*error));
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
    if (auto error = checkFields(envelope, {"format", "version", "name", "entities", "constraints", "last-entity-id", "last-constraint-id"}, "$")) {
        return Decoded::failure(std::move(*error));
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
    const auto& records = envelope.at("entities");
    snapshot.entities.reserve(records.size());
    for (size_t i = 0; i < records.size(); ++i) {
        auto entity = readEntity(records[i], "entities[" + std::to_string(i) + "]");
        if (!entity) {
            return Decoded::failure(entity.error());
        }
        snapshot.entities.push_back(std::move(entity.value()));
    }
    const auto& constraints = envelope.at("constraints");
    snapshot.constraints.reserve(constraints.size());
    for (size_t i = 0; i < constraints.size(); ++i) {
        auto constraint = readConstraint(constraints[i], "constraints[" + std::to_string(i) + "]");
        if (!constraint) {
            return Decoded::failure(constraint.error());
        }
        snapshot.constraints.push_back(std::move(constraint.value()));
    }
    auto document = std::make_unique<Document>(envelope.at("name").get<std::string>());
    const auto restored = document->sketch().replaceState(snapshot);
    if (!restored) {
        return Decoded::failure({CodecErrorCode::InvalidModel, "$", restored.error().message});
    }
    return Decoded::success(std::move(document));
}

}  // namespace core::persistence
