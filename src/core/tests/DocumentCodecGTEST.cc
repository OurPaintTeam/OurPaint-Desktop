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

sketch::SketchSnapshot constraintState() {
    using namespace sketch;
    SketchSnapshot state;
    state.entities = {{EntityId(1), Point2{{0, 0}}, false},
                      {EntityId(2), Point2{{3, 4}}, false},
                      {EntityId(3), Line2{{0, 0}, {4, 0}}, false},
                      {EntityId(4), Line2{{0, 3}, {4, 3}}, false},
                      {EntityId(5), Circle2{{0, 2}, 2}, false},
                      {EntityId(6), Circle2{{10, 2}, 5}, false},
                      {EntityId(7), Arc2{{10, 0}, {12, 0}, {10, 2}}, false},
                      {EntityId(8), NurbsCurve2{3, {{{4, 1}}, {{-2, 3}}, {{6, -4}}, {{1, 0}}}, {1, 2, 0.5, 1}, {0, 0, 0, 0, 1, 1, 1, 1}}, true}};
    state.lastEntityId = 8;
    return state;
}

Json constraintEnvelope() {
    auto envelope = geometryEnvelope({{"type", "line"}, {"start", {0, 0}}, {"end", {4, 0}}});
    envelope["constraints"] = Json::array({{{"id", 13}, {"type", "length"},
                                            {"refs", Json::array({{{"entity", 7}, {"sub", "whole"}, {"index", 0}}})}, {"value", 10}}});
    envelope["last-constraint-id"] = 20;
    return envelope;
}

struct ConstraintCase {
    std::string name;
    sketch::ConstraintDefinition definition;
};

std::vector<ConstraintCase> constraintCases() {
    using namespace sketch;
    const GeometryRef point{EntityId(1), SubElement::Whole};
    const GeometryRef otherPoint{EntityId(2), SubElement::Whole};
    const GeometryRef line{EntityId(3), SubElement::Whole};
    const GeometryRef otherLine{EntityId(4), SubElement::Whole};
    const GeometryRef circle{EntityId(5), SubElement::Whole};
    const GeometryRef otherCircle{EntityId(6), SubElement::Whole};
    return {{"coincident", {ConstraintType::Coincident, {point, otherPoint}, std::nullopt, std::nullopt}},
            {"horizontal", {ConstraintType::Horizontal, {line}, std::nullopt, std::nullopt}},
            {"vertical", {ConstraintType::Vertical, {otherLine}, std::nullopt, std::nullopt}},
            {"parallel", {ConstraintType::Parallel, {line, otherLine}, std::nullopt, std::nullopt}},
            {"perpendicular", {ConstraintType::Perpendicular, {line, otherLine}, std::nullopt, std::nullopt}},
            {"tangent", {ConstraintType::Tangent, {line, circle}, std::nullopt, std::nullopt, TangencyBranch::PositiveSide}},
            {"equal", {ConstraintType::Equal, {line, otherLine}, std::nullopt, std::nullopt}},
            {"fix", {ConstraintType::Fix, {point}, std::nullopt, Vec2{0, 0}}},
            {"concentric", {ConstraintType::Concentric, {circle, otherCircle}, std::nullopt, std::nullopt}},
            {"symmetric", {ConstraintType::Symmetric, {point, otherPoint, line}, std::nullopt, std::nullopt}},
            {"midpoint", {ConstraintType::Midpoint, {point, line}, std::nullopt, std::nullopt}},
            {"distance", {ConstraintType::Distance, {{EntityId(3), SubElement::Start}, {EntityId(4), SubElement::End}}, 5, std::nullopt}},
            {"horizontal-distance", {ConstraintType::HorizontalDistance, {point, otherPoint}, 0, std::nullopt}},
            {"vertical-distance", {ConstraintType::VerticalDistance, {point, otherPoint}, 4, std::nullopt}},
            {"length", {ConstraintType::Length, {line}, 10, std::nullopt}},
            {"radius", {ConstraintType::Radius, {circle}, 3, std::nullopt}},
            {"diameter", {ConstraintType::Diameter, {circle}, 6, std::nullopt}},
            {"angle", {ConstraintType::Angle, {line, otherLine}, 0.5, std::nullopt}}};
}

void expectDefinition(const sketch::ConstraintDefinition& actual, const sketch::ConstraintDefinition& expected) {
    EXPECT_EQ(actual.type, expected.type);
    EXPECT_EQ(actual.refs, expected.refs);
    EXPECT_EQ(actual.value, expected.value);
    EXPECT_EQ(actual.fixedPosition, expected.fixedPosition);
    EXPECT_EQ(actual.tangencyBranch, expected.tangencyBranch);
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

TEST(DocumentCodec, RoundTripsConstrainedSourceWithoutChangingItsState) {
    Document document("Populated");
    const auto point = document.sketch().addPoint({3, 4});
    ASSERT_TRUE(point);
    ASSERT_TRUE(document.sketch().addConstraint(
        {sketch::ConstraintType::Fix, {{point.value(), sketch::SubElement::Whole}}, std::nullopt, sketch::Vec2{3, 4}}));
    document.path() = "original.ourp";
    const Document& source = document;

    const auto before = source.sketch().snapshot();
    ASSERT_TRUE(before);
    ASSERT_EQ(before.value().constraints.size(), 1U);
    const auto encoded = encodeDocument(source);
    ASSERT_TRUE(encoded);
    const auto decoded = decodeDocument(encoded.value());
    ASSERT_TRUE(decoded);
    const auto restored = std::as_const(*decoded.value()).sketch().constraint(before.value().constraints[0].id);
    ASSERT_TRUE(restored);
    expectDefinition(restored.value().definition, before.value().constraints[0].definition);

    EXPECT_EQ(source.name(), "Populated");
    EXPECT_EQ(source.path(), "original.ourp");
    EXPECT_TRUE(source.isDirty());
    EXPECT_EQ(source.sketch().entityCount(), 1U);
    EXPECT_EQ(source.sketch().constraintCount(), 1U);
    const auto position = source.sketch().pointPosition({point.value(), sketch::SubElement::Whole});
    ASSERT_TRUE(position);
    EXPECT_EQ(position.value(), (sketch::Vec2{3, 4}));
}

TEST(DocumentCodec, RejectsIncompleteConstraintRecordsRatherThanDroppingThemOnLoad) {
    auto envelope = emptyEnvelope();
    envelope["constraints"] = Json::array({Json::object()});
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].id");
}

TEST(DocumentCodec, RoundTripsNamedDefinitionsForAllCurrentConstraintKinds) {
    for (const auto& item : constraintCases()) {
        SCOPED_TRACE(item.name);
        auto state = constraintState();
        state.constraints = {{sketch::ConstraintId(41), item.definition}};
        state.lastConstraintId = 50;
        Document document;
        ASSERT_TRUE(document.sketch().replaceState(state));

        const auto encoded = encodeDocument(document);
        ASSERT_TRUE(encoded);
        const auto envelope = Json::from_cbor(encoded.value());
        const auto& records = envelope.at("constraints");
        ASSERT_EQ(records.size(), 1U);
        EXPECT_EQ(records[0].at("type"), item.name);
        EXPECT_EQ(records[0].at("id"), 41);
        EXPECT_EQ(envelope.at("last-constraint-id"), 50);
        EXPECT_EQ(records[0].contains("value"), item.definition.value.has_value());
        EXPECT_EQ(records[0].contains("fixed-position"), item.definition.fixedPosition.has_value());
        EXPECT_EQ(records[0].contains("tangency-branch"), item.definition.tangencyBranch.has_value());
        const auto decoded = decodeDocument(encoded.value());
        ASSERT_TRUE(decoded);
        const auto restored = std::as_const(*decoded.value()).sketch().constraint(sketch::ConstraintId(41));
        ASSERT_TRUE(restored);
        EXPECT_EQ(restored.value().id, sketch::ConstraintId(41));
        expectDefinition(restored.value().definition, item.definition);
    }
}

TEST(DocumentCodec, PreservesEverySubElementAndDistinctNurbsControlIndices) {
    using namespace sketch;
    struct RefCase {
        GeometryRef ref;
        std::string name;
        Vec2 position;
    };
    const std::vector<RefCase> cases{{{EntityId(1), SubElement::Whole}, "whole", {0, 0}},
                                     {{EntityId(3), SubElement::Start}, "start", {0, 0}},
                                     {{EntityId(4), SubElement::End}, "end", {4, 3}},
                                     {{EntityId(5), SubElement::Center}, "center", {0, 2}},
                                     {{EntityId(7), SubElement::Center}, "center", {10, 0}},
                                     {{EntityId(8), SubElement::Control, 1}, "control", {-2, 3}},
                                     {{EntityId(8), SubElement::Control, 2}, "control", {6, -4}}};
    for (const auto& item : cases) {
        const ConstraintDefinition definition{ConstraintType::Fix, {item.ref}, std::nullopt, item.position};
        auto state = constraintState();
        state.constraints = {{ConstraintId(31), definition}};
        state.lastConstraintId = 31;
        Document document;
        ASSERT_TRUE(document.sketch().replaceState(state));
        const auto encoded = encodeDocument(document);
        ASSERT_TRUE(encoded);
        const auto envelope = Json::from_cbor(encoded.value());
        const auto& record = envelope.at("constraints")[0];
        EXPECT_EQ(record.at("refs")[0], (Json{{"entity", item.ref.entity.get()}, {"sub", item.name}, {"index", item.ref.index}}));
        EXPECT_EQ(record.at("fixed-position"), (Json{item.position.x, item.position.y}));
        const auto decoded = decodeDocument(encoded.value());
        ASSERT_TRUE(decoded);
        const auto restored = std::as_const(*decoded.value()).sketch().constraint(ConstraintId(31));
        ASSERT_TRUE(restored);
        expectDefinition(restored.value().definition, definition);
    }

    auto state = constraintState();
    const ConstraintDefinition distance{ConstraintType::Distance,
                                        {{EntityId(8), SubElement::Control, 2}, {EntityId(8), SubElement::Control, 1}}, 12, std::nullopt};
    state.constraints = {{ConstraintId(32), distance}};
    state.lastConstraintId = 32;
    Document document;
    ASSERT_TRUE(document.sketch().replaceState(state));
    const auto encoded = encodeDocument(document);
    ASSERT_TRUE(encoded);
    const auto decoded = decodeDocument(encoded.value());
    ASSERT_TRUE(decoded);
    const auto restored = std::as_const(*decoded.value()).sketch().constraint(ConstraintId(32));
    ASSERT_TRUE(restored);
    expectDefinition(restored.value().definition, distance);
}

TEST(DocumentCodec, PreservesAllTangencyBranchesAndOperandOrderWithoutReinferringIntent) {
    using namespace sketch;
    struct BranchCase {
        TangencyBranch branch;
        std::string name;
        std::vector<GeometryRef> refs;
    };
    const GeometryRef line{EntityId(3), SubElement::Whole};
    const GeometryRef small{EntityId(5), SubElement::Whole};
    const GeometryRef large{EntityId(6), SubElement::Whole};
    const std::vector<BranchCase> cases{{TangencyBranch::PositiveSide, "positive-side", {line, small}},
                                       {TangencyBranch::NegativeSide, "negative-side", {small, line}},
                                       {TangencyBranch::External, "external", {small, large}},
                                       {TangencyBranch::FirstContainsSecond, "first-contains-second", {large, small}},
                                       {TangencyBranch::SecondContainsFirst, "second-contains-first", {small, large}}};
    for (const auto& item : cases) {
        SCOPED_TRACE(item.name);
        const ConstraintDefinition definition{ConstraintType::Tangent, item.refs, std::nullopt, std::nullopt, item.branch};
        auto state = constraintState();
        state.constraints = {{ConstraintId(17), definition}};
        state.lastConstraintId = 17;
        Document document;
        ASSERT_TRUE(document.sketch().replaceState(state));
        const auto encoded = encodeDocument(document);
        ASSERT_TRUE(encoded);
        const auto envelope = Json::from_cbor(encoded.value());
        const auto& record = envelope.at("constraints")[0];
        EXPECT_EQ(record.at("tangency-branch"), item.name);
        EXPECT_EQ(record.at("refs")[0].at("entity"), item.refs[0].entity.get());
        EXPECT_EQ(record.at("refs")[1].at("entity"), item.refs[1].entity.get());
        const auto decoded = decodeDocument(encoded.value());
        ASSERT_TRUE(decoded);
        const auto restored = std::as_const(*decoded.value()).sketch().constraint(ConstraintId(17));
        ASSERT_TRUE(restored);
        expectDefinition(restored.value().definition, definition);
    }
}

TEST(DocumentCodec, RetainsLargeIndependentIdsAndConstraintWatermarksAcrossDeletionAndAddition) {
    using namespace sketch;
    const int64_t large = (int64_t{1} << 53) + 17;
    auto state = constraintState();
    state.entities[2].id = EntityId(large);
    state.lastEntityId = large + 20;
    const ConstraintDefinition horizontal{ConstraintType::Horizontal, {{EntityId(large), SubElement::Whole}}, std::nullopt, std::nullopt};
    state.constraints = {{ConstraintId(large), horizontal},
                         {ConstraintId(large + 2), {ConstraintType::Vertical, {{EntityId(4), SubElement::Whole}}, std::nullopt, std::nullopt}}};
    state.lastConstraintId = large + 20;
    Document document;
    ASSERT_TRUE(document.sketch().replaceState(state));
    ASSERT_TRUE(document.sketch().removeConstraint(ConstraintId(large + 2)));
    const auto encoded = encodeDocument(document);
    ASSERT_TRUE(encoded);
    auto decoded = decodeDocument(encoded.value());
    ASSERT_TRUE(decoded);
    const auto& loaded = std::as_const(*decoded.value()).sketch();
    const auto restored = loaded.constraint(ConstraintId(large));
    ASSERT_TRUE(restored);
    expectDefinition(restored.value().definition, horizontal);
    EXPECT_FALSE(loaded.constraint(ConstraintId(large + 2)));
    ASSERT_TRUE(decoded.value()->sketch().addConstraint({ConstraintType::Vertical, {{EntityId(4), SubElement::Whole}}, std::nullopt, std::nullopt}));
    EXPECT_TRUE(loaded.constraint(ConstraintId(large + 21)));
}

TEST(DocumentCodec, RetainsFixWithoutAnExplicitTargetAndDoesNotSolveLengthOnLoading) {
    auto state = constraintState();
    state.constraints = {{sketch::ConstraintId(9),
                          {sketch::ConstraintType::Fix, {{sketch::EntityId(1), sketch::SubElement::Whole}}, std::nullopt, std::nullopt}}};
    state.lastConstraintId = 9;
    Document document;
    ASSERT_TRUE(document.sketch().replaceState(state));
    const auto encoded = encodeDocument(document);
    ASSERT_TRUE(encoded);
    EXPECT_FALSE(Json::from_cbor(encoded.value()).at("constraints")[0].contains("fixed-position"));
    const auto decoded = decodeDocument(encoded.value());
    ASSERT_TRUE(decoded);
    const auto fixed = std::as_const(*decoded.value()).sketch().constraint(sketch::ConstraintId(9));
    ASSERT_TRUE(fixed);
    EXPECT_FALSE(fixed.value().definition.fixedPosition);

    const auto length = decodeEnvelope(constraintEnvelope());
    ASSERT_TRUE(length);
    const auto line = std::as_const(*length.value()).sketch().entity(sketch::EntityId(7));
    ASSERT_TRUE(line);
    expectGeometry(line.value().geometry, sketch::Line2{{0, 0}, {4, 0}});
    const auto dimension = std::as_const(*length.value()).sketch().constraint(sketch::ConstraintId(13));
    ASSERT_TRUE(dimension);
    ASSERT_TRUE(dimension.value().definition.value);
    EXPECT_DOUBLE_EQ(*dimension.value().definition.value, 10);
}

TEST(DocumentCodec, RejectsUnknownNamesAndNumericEnumOrdinalsInConstraints) {
    auto envelope = constraintEnvelope();
    envelope["constraints"][0]["type"] = "future-constraint";
    expectError(decodeEnvelope(envelope), CodecErrorCode::UnsupportedData, "constraints[0].type");
    envelope["constraints"][0]["type"] = 0;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].type");
    for (const Json& sub : {Json("vertex"), Json(0)}) {
        envelope = constraintEnvelope();
        envelope["constraints"][0]["refs"][0]["sub"] = sub;
        expectError(decodeEnvelope(envelope), sub.is_string() ? CodecErrorCode::UnsupportedData : CodecErrorCode::InvalidSchema,
                    "constraints[0].refs[0].sub");
    }
    for (const Json& branch : {Json("future-branch"), Json(0)}) {
        envelope = constraintEnvelope();
        envelope["constraints"][0]["tangency-branch"] = branch;
        expectError(decodeEnvelope(envelope), branch.is_string() ? CodecErrorCode::UnsupportedData : CodecErrorCode::InvalidSchema,
                    "constraints[0].tangency-branch");
    }
}

TEST(DocumentCodec, RequiresCompleteIntegerReferencesAndPositiveConstraintIds) {
    for (const auto field : {"entity", "sub", "index"}) {
        auto envelope = constraintEnvelope();
        envelope["constraints"][0]["refs"][0].erase(field);
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].refs[0]." + std::string(field));
    }
    for (const Json& value : {Json(-1), Json(0.0), Json(true), Json(std::numeric_limits<uint64_t>::max())}) {
        auto envelope = constraintEnvelope();
        envelope["constraints"][0]["refs"][0]["index"] = value;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].refs[0].index");
    }
    for (const Json& value : {Json(0), Json(-1), Json(13.0), Json(true), Json(std::numeric_limits<uint64_t>::max())}) {
        auto envelope = constraintEnvelope();
        envelope["constraints"][0]["id"] = value;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].id");
        envelope = constraintEnvelope();
        envelope["constraints"][0]["refs"][0]["entity"] = value;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].refs[0].entity");
    }
    auto envelope = constraintEnvelope();
    envelope["constraints"][0]["refs"][0]["index"] = 1;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].refs[0].index");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["refs"][0]["native-handle"] = 1;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].refs[0].native-handle");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["native-handle"] = 1;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].native-handle");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["refs"] = Json::object();
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].refs");
}

TEST(DocumentCodec, RequiresApplicableFiniteOptionalFieldsAndExplicitTangencyIntent) {
    for (const Json& value : {Json(true), Json("10"), Json(), Json(std::numeric_limits<double>::infinity()), Json(std::numeric_limits<double>::quiet_NaN())}) {
        auto envelope = constraintEnvelope();
        envelope["constraints"][0]["value"] = value;
        expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].value");
    }
    auto envelope = constraintEnvelope();
    envelope["constraints"][0].erase("value");
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].value");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["type"] = "horizontal";
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].value");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["fixed-position"] = {1, 2};
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].fixed-position");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["tangency-branch"] = "external";
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].tangency-branch");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["type"] = "tangent";
    envelope["constraints"][0].erase("value");
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].tangency-branch");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["type"] = "fix";
    envelope["constraints"][0].erase("value");
    envelope["constraints"][0]["fixed-position"] = {0, std::numeric_limits<double>::infinity()};
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidSchema, "constraints[0].fixed-position[1]");
}

TEST(DocumentCodec, RejectsInvalidConstraintIdentitiesAndReferencesBeforeReturningAModel) {
    auto envelope = constraintEnvelope();
    envelope["last-constraint-id"] = 12;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
    envelope = constraintEnvelope();
    const Json duplicate = envelope["constraints"][0];
    envelope["constraints"].push_back(duplicate);
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["refs"][0]["entity"] = 999;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["refs"] = Json::array();
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["refs"][0]["sub"] = "control";
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
    envelope = geometryEnvelope(nurbsRecord());
    envelope["constraints"] = Json::array({{{"id", 13}, {"type", "fix"},
                                            {"refs", Json::array({{{"entity", 7}, {"sub", "control"}, {"index", 4}}})}}});
    envelope["last-constraint-id"] = 20;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
    envelope = constraintEnvelope();
    envelope["constraints"][0]["value"] = -1;
    expectError(decodeEnvelope(envelope), CodecErrorCode::InvalidModel, "$");
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
