#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "saveload/DocumentSaveLoad.h"

namespace core::persistence {
namespace {

using Json = nlohmann::json;

Json emptyEnvelope() {
    return {{"format", "ourpaint-document"},
            {"version", 1},
            {"name", "Empty sketch"},
            {"entities", Json::array()},
            {"constraints", Json::array()},
            {"last-entity-id", 0},
            {"last-constraint-id", 0}};
}

auto decodeEnvelope(const Json& envelope) {
    const auto bytes = Json::to_cbor(envelope);
    return decodeDocument(bytes);
}

Json geometryEnvelope(const Json& geometry) {
    auto envelope = emptyEnvelope();
    envelope["entities"] = Json::array({{{"id", 7}, {"construction", true}, {"geometry", geometry}}});
    envelope["last-entity-id"] = 11;
    return envelope;
}

Json nurbsRecord() {
    return {{"type", "nurbs"},
            {"degree", 3},
            {"controls", {{4, 1}, {-2, 3}, {6, -4}, {1, 0}}},
            {"weights", {1, 2, 0.5, 1}},
            {"knots", {0, 0, 0, 0, 1, 1, 1, 1}}};
}

std::vector<sketch::SketchGeometry> geometryCases() {
    using namespace sketch;
    return {Point2{{-3, 2}},
            Line2{{0, 0}, {3, 4}},
            Circle2{{5, -2}, 2.5},
            Arc2{{0, 0}, {2, 0}, {0, 2}},
            NurbsCurve2{3, {{{4, 1}}, {{-2, 3}}, {{6, -4}}, {{1, 0}}}, {1, 2, 0.5, 1}, {0, 0, 0, 0, 1, 1, 1, 1}},
            NurbsCurve2{2, {{{0, 0}}, {{1, 3}}, {{4, -2}}, {{5, 0}}}, {1, 0.5, 2, 1}, {0, 0, 0, 0.5, 1, 1, 1}}};
}

void expectGeometry(const sketch::SketchGeometry& actual, const sketch::SketchGeometry& expected) {
    ASSERT_EQ(actual.index(), expected.index());
    std::visit(
        [&](const auto& value) {
            using T = std::decay_t<decltype(value)>;
            const auto& other = std::get<T>(expected);
            if constexpr (std::is_same_v<T, sketch::Point2>) {
                EXPECT_EQ(value.position, other.position);
            } else if constexpr (std::is_same_v<T, sketch::Line2>) {
                EXPECT_EQ(value.start, other.start);
                EXPECT_EQ(value.end, other.end);
            } else if constexpr (std::is_same_v<T, sketch::Circle2>) {
                EXPECT_EQ(value.center, other.center);
                EXPECT_DOUBLE_EQ(value.radius, other.radius);
            } else if constexpr (std::is_same_v<T, sketch::Arc2>) {
                EXPECT_EQ(value.center, other.center);
                EXPECT_EQ(value.start, other.start);
                EXPECT_EQ(value.end, other.end);
            } else {
                EXPECT_EQ(value.degree, other.degree);
                EXPECT_EQ(value.weights, other.weights);
                EXPECT_EQ(value.knots, other.knots);
                ASSERT_EQ(value.controlPoints.size(), other.controlPoints.size());
                for (size_t i = 0; i < value.controlPoints.size(); ++i) {
                    EXPECT_EQ(value.controlPoints[i].position, other.controlPoints[i].position);
                }
            }
        },
        actual);
}

template <class T>
void expectError(const CodecResult<T>& result, CodecErrorCode code, const std::string& field) {
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, code);
    EXPECT_EQ(result.error().field, field);
    EXPECT_FALSE(result.error().message.empty());
}

TEST(DocumentCodec, EncodesTheVersionOneEnvelopeWithoutChangingTheSource) {
    Document document("Empty sketch");
    const Document& source = document;
    ASSERT_FALSE(source.isDirty());

    const auto encoded = encodeDocument(source);

    ASSERT_TRUE(encoded);
    EXPECT_EQ(Json::from_cbor(encoded.value()), emptyEnvelope());
    EXPECT_FALSE(source.isDirty());
    EXPECT_EQ(source.name(), "Empty sketch");
    EXPECT_TRUE(source.path().empty());
    EXPECT_EQ(source.sketch().entityCount(), 0U);
    EXPECT_EQ(source.sketch().constraintCount(), 0U);
}

TEST(DocumentCodec, ReturnsAnOwnedDocumentAfterInputBytesAreDestroyed) {
    auto decoded = decodeEnvelope(emptyEnvelope());

    ASSERT_TRUE(decoded);
    ASSERT_NE(decoded.value(), nullptr);
    const Document& document = *decoded.value();
    EXPECT_EQ(document.name(), "Empty sketch");
    EXPECT_TRUE(document.path().empty());
    EXPECT_EQ(document.sketch().entityCount(), 0U);
    EXPECT_EQ(document.sketch().constraintCount(), 0U);
}

TEST(DocumentCodec, EmptyRoundTripRetainsUtf8NamesAndConsumedIdsWithoutFloatingPointConversion) {
    const std::string name = "Sketch \xd0\xad\xd1\x81\xd0\xba\xd0\xb8\xd0\xb7 \xf0\x9f\x8e\xa8";
    Document document(name);
    sketch::SketchSnapshot state;
    state.lastEntityId = (int64_t{1} << 53) + 17;
    state.lastConstraintId = std::numeric_limits<int64_t>::max();
    ASSERT_TRUE(document.sketch().replaceState(state));

    const auto encoded = encodeDocument(document);
    ASSERT_TRUE(encoded);
    auto decoded = decodeDocument(encoded.value());
    ASSERT_TRUE(decoded);
    const Document& restored = *decoded.value();
    EXPECT_EQ(restored.name(), name);
    const auto snapshot = restored.sketch().snapshot();
    ASSERT_TRUE(snapshot);
    EXPECT_EQ(snapshot.value().lastEntityId, state.lastEntityId);
    EXPECT_EQ(snapshot.value().lastConstraintId, state.lastConstraintId);
    const auto next = decoded.value()->sketch().addPoint({1, 2});
    ASSERT_TRUE(next);
    EXPECT_EQ(next.value().get(), state.lastEntityId + 1);
}

TEST(DocumentCodec, RejectsConstrainedSourceRatherThanDroppingConstraints) {
    Document document("Populated");
    const auto point = document.sketch().addPoint({3, 4});
    ASSERT_TRUE(point);
    ASSERT_TRUE(document.sketch().addConstraint(
        {sketch::ConstraintType::Fix, {{point.value(), sketch::SubElement::Whole}}, std::nullopt, sketch::Vec2{3, 4}}));
    document.path() = "original.ourp";
    const Document& source = document;

    expectError(encodeDocument(source), CodecErrorCode::UnsupportedData, "constraints");

    EXPECT_EQ(source.name(), "Populated");
    EXPECT_EQ(source.path(), "original.ourp");
    EXPECT_TRUE(source.isDirty());
    EXPECT_EQ(source.sketch().entityCount(), 1U);
    EXPECT_EQ(source.sketch().constraintCount(), 1U);
    const auto position = source.sketch().pointPosition({point.value(), sketch::SubElement::Whole});
    ASSERT_TRUE(position);
    EXPECT_EQ(position.value(), (sketch::Vec2{3, 4}));
}

TEST(DocumentCodec, RejectsConstraintRecordsRatherThanDroppingThemOnLoad) {
    auto envelope = emptyEnvelope();
    envelope["constraints"] = Json::array({Json::object()});
    expectError(decodeEnvelope(envelope), CodecErrorCode::UnsupportedData, "constraints");
}

TEST(DocumentCodec, RoundTripsAllGeometryKindsWithIdsConstructionAndConsumedIds) {
    Document document("Geometry");
    const auto geometry = geometryCases();
    std::vector<sketch::EntityId> ids;
    for (size_t i = 0; i < geometry.size(); ++i) {
        const auto added = document.sketch().addEntity(geometry[i], i % 2 == 0);
        ASSERT_TRUE(added);
        ids.push_back(added.value());
    }
    const auto discarded = document.sketch().addPoint({100, 200});
    ASSERT_TRUE(discarded);
    ASSERT_TRUE(document.sketch().removeEntity(discarded.value()));
    document.path() = "source.ourp";
    const Document& source = document;
    const bool wasDirty = source.isDirty();

    const auto encoded = encodeDocument(source);
    ASSERT_TRUE(encoded);
    auto decoded = decodeDocument(encoded.value());
    ASSERT_TRUE(decoded);
    const Document& restored = *decoded.value();
    EXPECT_EQ(restored.name(), "Geometry");
    EXPECT_EQ(restored.sketch().entityCount(), geometry.size());
    EXPECT_EQ(restored.sketch().constraintCount(), 0U);
    for (size_t i = 0; i < geometry.size(); ++i) {
        const auto entity = restored.sketch().entity(ids[i]);
        ASSERT_TRUE(entity);
        EXPECT_EQ(entity.value().id, ids[i]);
        EXPECT_EQ(entity.value().construction, i % 2 == 0);
        expectGeometry(entity.value().geometry, geometry[i]);
        const auto unchanged = source.sketch().entity(ids[i]);
        ASSERT_TRUE(unchanged);
        expectGeometry(unchanged.value().geometry, geometry[i]);
    }
    EXPECT_EQ(source.path(), "source.ourp");
    EXPECT_EQ(source.isDirty(), wasDirty);
    const auto next = decoded.value()->sketch().addPoint({300, 400});
    ASSERT_TRUE(next);
    EXPECT_GT(next.value().get(), discarded.value().get());
}

TEST(DocumentCodec, WritesVersionOneGeometryNamesAndExpandedNurbsData) {
    Document document;
    for (const auto& geometry : geometryCases()) {
        ASSERT_TRUE(document.sketch().addEntity(geometry));
    }
    const auto encoded = encodeDocument(document);
    ASSERT_TRUE(encoded);
    const auto envelope = Json::from_cbor(encoded.value());
    const auto& entities = envelope.at("entities");
    ASSERT_EQ(entities.size(), 6U);
    EXPECT_EQ(entities[0].at("geometry"), (Json{{"type", "point"}, {"position", {-3, 2}}}));
    EXPECT_EQ(entities[1].at("geometry"), (Json{{"type", "line"}, {"start", {0, 0}}, {"end", {3, 4}}}));
    EXPECT_EQ(entities[2].at("geometry"), (Json{{"type", "circle"}, {"center", {5, -2}}, {"radius", 2.5}}));
    EXPECT_EQ(entities[3].at("geometry"), (Json{{"type", "arc"}, {"center", {0, 0}}, {"start", {2, 0}}, {"end", {0, 2}}}));
    EXPECT_EQ(entities[4].at("geometry"), nurbsRecord());
    EXPECT_EQ(entities[5].at("geometry").at("knots"), (Json{0, 0, 0, 0.5, 1, 1, 1}));
}

TEST(DocumentCodec, LoadsSparseLargeIdsWithoutFloatingPointConversionOrReallocation) {
    const int64_t first = (int64_t{1} << 53) + 17;
    const int64_t second = first + 5;
    auto envelope = geometryEnvelope({{"type", "point"}, {"position", {-3, 2}}});
    auto& records = envelope["entities"];
    records[0]["id"] = second;
    records.push_back({{"id", first}, {"construction", false}, {"geometry", {{"type", "line"}, {"start", {0, 0}}, {"end", {3, 4}}}}});
    envelope["last-entity-id"] = second + 10;
    auto decoded = decodeEnvelope(envelope);
    ASSERT_TRUE(decoded);
    const auto& loadedSketch = std::as_const(*decoded.value()).sketch();
    const auto point = loadedSketch.entity(sketch::EntityId(second));
    const auto line = loadedSketch.entity(sketch::EntityId(first));
    ASSERT_TRUE(point);
    ASSERT_TRUE(line);
    EXPECT_TRUE(point.value().construction);
    EXPECT_FALSE(line.value().construction);
    expectGeometry(point.value().geometry, sketch::Point2{{-3, 2}});
    expectGeometry(line.value().geometry, sketch::Line2{{0, 0}, {3, 4}});
    const auto next = decoded.value()->sketch().addPoint({10, 20});
    ASSERT_TRUE(next);
    EXPECT_EQ(next.value().get(), second + 11);
}

TEST(DocumentCodec, PreservesUnsolvedArcGeometryWithoutRequestingSolve) {
    const sketch::Arc2 arc{{0, 0}, {2, 0}, {0, 3}};
    auto decoded = decodeEnvelope(geometryEnvelope({{"type", "arc"}, {"center", {0, 0}}, {"start", {2, 0}}, {"end", {0, 3}}}));
    ASSERT_TRUE(decoded);
    const auto entity = std::as_const(*decoded.value()).sketch().entity(sketch::EntityId(7));
    ASSERT_TRUE(entity);
    expectGeometry(entity.value().geometry, arc);
    const auto encoded = encodeDocument(*decoded.value());
    ASSERT_TRUE(encoded);
    const auto again = decodeDocument(encoded.value());
    ASSERT_TRUE(again);
    const auto transferred = std::as_const(*again.value()).sketch().entity(sketch::EntityId(7));
    ASSERT_TRUE(transferred);
    expectGeometry(transferred.value().geometry, arc);
}

TEST(DocumentCodec, ReportsInvalidEntityRecordsAndUnknownGeometryWithoutDroppingThem) {
    const Json point{{"type", "point"}, {"position", {1, 2}}};
    for (const Json& id : {Json(0), Json(-1), Json(7.0), Json(true), Json(std::numeric_limits<uint64_t>::max())}) {
        auto envelope = geometryEnvelope(point);
        envelope["entities"][0]["id"] = id;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "entities[0].id");
    }
    auto envelope = geometryEnvelope(point);
    envelope["entities"][0]["construction"] = 1;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "entities[0].construction");
    envelope = geometryEnvelope(point);
    envelope["entities"][0].erase("geometry");
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "entities[0].geometry");
    envelope = geometryEnvelope({{"type", "ellipse"}});
    expectError(decodeEnvelope(envelope), CodecErrorCode::UnsupportedData, "entities[0].geometry.type");
    envelope = geometryEnvelope(point);
    envelope["entities"][0]["geometry"]["solver-handle"] = 42;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "entities[0].geometry.solver-handle");
}

TEST(DocumentCodec, RequiresFiniteTwoNumberPositions) {
    for (const Json& value : {Json(true), Json("1"), Json(std::numeric_limits<double>::infinity()), Json(std::numeric_limits<double>::quiet_NaN())}) {
        auto envelope = geometryEnvelope({{"type", "point"}, {"position", Json::array({value, 2})}});
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "entities[0].geometry.position[0]");
    }
    for (const Json& position : {Json(0), Json::array(), Json::array({1}), Json::array({1, 2, 3})}) {
        expectError(decodeEnvelope(geometryEnvelope({{"type", "point"}, {"position", position}})),
                    CodecErrorCode::InvalidSchema, "entities[0].geometry.position");
    }
}

TEST(DocumentCodec, ChecksNurbsDegreeCountsAndFiniteArraysBeforeRestoringTheModel) {
    for (const Json& degree : {Json(0), Json(-1), Json(3.0), Json(static_cast<int64_t>(std::numeric_limits<int>::max()) + 1)}) {
        auto record = nurbsRecord();
        record["degree"] = degree;
        expectError(decodeEnvelope(geometryEnvelope(record)), CodecErrorCode::InvalidSchema, "entities[0].geometry.degree");
    }
    for (const auto field : {"controls", "weights", "knots"}) {
        auto record = nurbsRecord();
        record[field].erase(record[field].end() - 1);
        expectError(decodeEnvelope(geometryEnvelope(record)), CodecErrorCode::InvalidSchema, "entities[0].geometry");
    }
    auto record = nurbsRecord();
    record["controls"][1][0] = std::numeric_limits<double>::quiet_NaN();
    expectError(decodeEnvelope(geometryEnvelope(record)), CodecErrorCode::InvalidSchema, "entities[0].geometry.controls[1][0]");
    for (const auto field : {"weights", "knots"}) {
        record = nurbsRecord();
        record[field][2] = std::numeric_limits<double>::infinity();
        expectError(decodeEnvelope(geometryEnvelope(record)), CodecErrorCode::InvalidSchema, "entities[0].geometry." + std::string(field) + "[2]");
    }
}

TEST(DocumentCodec, RejectsInvalidGeometryAndIdentityThroughStagedReplacement) {
    std::vector<Json> invalid{{{"type", "line"}, {"start", {1, 2}}, {"end", {1, 2}}},
                              {{"type", "circle"}, {"center", {0, 0}}, {"radius", -1}},
                              {{"type", "arc"}, {"center", {0, 0}}, {"start", {0, 0}}, {"end", {0, 2}}}};
    auto curve = nurbsRecord();
    curve["weights"][0] = 0;
    invalid.push_back(curve);
    curve = nurbsRecord();
    curve["knots"][4] = -1;
    invalid.push_back(curve);
    for (const auto& geometry : invalid) {
        expectError(decodeEnvelope(geometryEnvelope(geometry)), CodecErrorCode::InvalidModel, "$");
    }
    auto envelope = geometryEnvelope({{"type", "point"}, {"position", {1, 2}}});
    envelope["last-entity-id"] = 6;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
    envelope["last-entity-id"] = 11;
    const Json duplicate = envelope["entities"][0];
    envelope["entities"].push_back(duplicate);
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
}

TEST(DocumentCodec, RejectsDuplicateKeysInsideEntityRecordsBeforeDomParsing) {
    auto record = Json::to_cbor(geometryEnvelope({{"type", "point"}, {"position", {1, 2}}}).at("entities")[0]);
    ASSERT_EQ(record[0], 0xa3);
    record[0] = 0xa4;
    DocumentBytes bytes{0xa1, 0x68, 'e', 'n', 't', 'i', 't', 'i', 'e', 's', 0x81};
    bytes.insert(bytes.end(), record.begin(), record.end());
    bytes.insert(bytes.end(), {0x62, 'i', 'd', 0x07});
    expectError(decodeDocument(bytes), CodecErrorCode::InvalidSchema, "entities[0].id");
}

TEST(DocumentCodec, BoundsNestedGeometryBeforeAllocatingTheDom) {
    DocumentBytes deep{0xa1, 0x68, 'e', 'n', 't', 'i', 't', 'i', 'e', 's'};
    deep.insert(deep.end(), 64, 0x81);
    deep.push_back(0);
    const auto decoded = decodeDocument(deep);
    ASSERT_FALSE(decoded);
    EXPECT_EQ(decoded.error().code, CodecErrorCode::LimitExceeded);
    const DocumentBytes many{0xa1, 0x68, 'e', 'n', 't', 'i', 't', 'i', 'e', 's', 0x9a, 0x00, 0x0f, 0x42, 0x40};
    expectError(decodeDocument(many), CodecErrorCode::LimitExceeded, "entities");
}

TEST(DocumentCodec, RejectsTruncationAndTrailingBytes) {
    auto truncated = Json::to_cbor(emptyEnvelope());
    truncated.pop_back();
    auto trailing = Json::to_cbor(emptyEnvelope());
    trailing.push_back(0);
    const std::vector<DocumentBytes> cases{{}, {0xff}, {0xa1}, truncated, trailing};
    for (const auto& bytes : cases) {
        const auto decoded = decodeDocument(bytes);
        ASSERT_FALSE(decoded);
        EXPECT_EQ(decoded.error().code, CodecErrorCode::InvalidCbor);
    }
}

TEST(DocumentCodec, ReportsMissingAndUnknownEnvelopeFields) {
    for (const auto field : {"format", "version", "name", "entities", "constraints", "last-entity-id", "last-constraint-id"}) {
        auto envelope = emptyEnvelope();
        envelope.erase(field);
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, field);
    }
    auto envelope = emptyEnvelope();
    envelope["unknown"] = "data";
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "unknown");
}

TEST(DocumentCodec, RequiresTheDocumentFormatAndRecognizedIntegerVersion) {
    auto envelope = emptyEnvelope();
    envelope["format"] = "ourpaint-project";
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "format");
    envelope = emptyEnvelope();
    envelope["version"] = 2;
    expectError(decodeEnvelope(envelope), CodecErrorCode::UnsupportedVersion, "version");
    for (const Json& version : {Json(true), Json(1.0), Json(-1), Json("1"), Json(std::numeric_limits<uint64_t>::max())}) {
        envelope["version"] = version;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "version");
    }
}

TEST(DocumentCodec, RejectsWrongEnvelopeTypesAndInvalidIdWatermarks) {
    for (const auto field : {"name", "entities", "constraints"}) {
        auto envelope = emptyEnvelope();
        envelope[field] = 42;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, field);
    }
    for (const auto field : {"last-entity-id", "last-constraint-id"}) {
        for (const Json& value : {Json(-1), Json(0.0), Json(true), Json(std::numeric_limits<uint64_t>::max())}) {
            auto envelope = emptyEnvelope();
            envelope[field] = value;
            expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, field);
        }
    }
}

TEST(DocumentCodec, RejectsDuplicateKeysBeforeTheyCanBeOverwritten) {
    const DocumentBytes bytes{0xa2, 0x64, 'n', 'a', 'm', 'e', 0x60, 0x64, 'n', 'a', 'm', 'e', 0x60};
    expectError(decodeDocument(bytes), CodecErrorCode::InvalidSchema, "name");
}

TEST(DocumentCodec, RejectsIndefiniteContainersAndChunkedTextBeforeRecursiveParsing) {
    const DocumentBytes object{0xbf, 0xff};
    const DocumentBytes text{0xa1, 0x64, 'n', 'a', 'm', 'e', 0x7f, 0xff};
    const DocumentBytes array{0xa1, 0x68, 'e', 'n', 't', 'i', 't', 'i', 'e', 's', 0x9f, 0xff};
    expectError(decodeDocument(object), CodecErrorCode::InvalidCbor, "$");
    expectError(decodeDocument(text), CodecErrorCode::InvalidCbor, "name");
    expectError(decodeDocument(array), CodecErrorCode::InvalidCbor, "entities");
}

TEST(DocumentCodec, RejectsInvalidUtf8WithoutRewritingTheName) {
    const std::vector<std::string> names{"\x80", "\xc0\xaf", "\xe2\x82", "\xed\xa0\x80", "\xf4\x90\x80\x80"};
    for (const auto& name : names) {
        const Document source(name);
        expectError(encodeDocument(source), CodecErrorCode::InvalidSchema, "name");
        EXPECT_EQ(source.name(), name);
        EXPECT_FALSE(source.isDirty());
        auto envelope = emptyEnvelope();
        envelope["name"] = name;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "name");
    }
}

TEST(DocumentCodec, RejectsOversizedInputBeforeParsingIt) {
    const DocumentBytes bytes(MaxDocumentBytes + 1, 0);
    expectError(decodeDocument(bytes), CodecErrorCode::LimitExceeded, "$");
}

}  // namespace
}  // namespace core::persistence
