#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <limits>
#include <string>
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

TEST(DocumentCodec, RejectsPopulatedSourceRatherThanWritingAnEmptySketch) {
    Document document("Populated");
    const auto point = document.sketch().addPoint({3, 4});
    ASSERT_TRUE(point);
    document.path() = "original.ourp";
    const Document& source = document;

    expectError(encodeDocument(source), CodecErrorCode::UnsupportedData, "entities");

    EXPECT_EQ(source.name(), "Populated");
    EXPECT_EQ(source.path(), "original.ourp");
    EXPECT_TRUE(source.isDirty());
    EXPECT_EQ(source.sketch().entityCount(), 1U);
    const auto position = source.sketch().pointPosition({point.value(), sketch::SubElement::Whole});
    ASSERT_TRUE(position);
    EXPECT_EQ(position.value(), (sketch::Vec2{3, 4}));
}

TEST(DocumentCodec, RejectsPopulatedRecordsRatherThanDroppingThemOnLoad) {
    for (const auto field : {"entities", "constraints"}) {
        auto envelope = emptyEnvelope();
        envelope[field] = Json::array({Json::object()});
        expectError(decodeEnvelope(envelope), CodecErrorCode::UnsupportedData, field);
    }
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
    envelope.erase("name");
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
