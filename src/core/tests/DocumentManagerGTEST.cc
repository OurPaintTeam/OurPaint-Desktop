#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Document.h"
#include "DocumentManager.h"

namespace {

static_assert(!std::is_copy_constructible_v<DocumentManager>);
static_assert(!std::is_copy_assignable_v<DocumentManager>);

TEST(DocumentManager, EmptyAndMissingDocumentsAreSafeToQuery) {
    DocumentManager manager;

    EXPECT_EQ(manager.count(), 0U);
    EXPECT_EQ(manager.at(DocumentId{0}), nullptr);
    EXPECT_EQ(manager.at(InvalidDocumentId), nullptr);
    EXPECT_EQ(manager.at("missing"), nullptr);
    EXPECT_EQ(manager.findIdByName("missing"), InvalidDocumentId);
    EXPECT_FALSE(manager.isDocumentSaved(InvalidDocumentId));
    EXPECT_FALSE(manager.isDocumentSaved("missing"));
    EXPECT_TRUE(manager.isAllDocumentsSaved());
    EXPECT_FALSE(manager.closeDocument(InvalidDocumentId));
    EXPECT_FALSE(manager.closeDocument("missing"));
}

TEST(DocumentManager, AdoptsTheStagedDocumentWithoutReplacingItsSketch) {
    using namespace core::sketch;
    DocumentManager manager;
    auto staged = std::make_unique<Document>("Loaded sketch");
    Document* original = staged.get();
    auto& sketch = staged->sketch();
    const auto line = sketch.addLine({1, 2}, {5, 2});
    ASSERT_TRUE(line);
    const auto constraint = sketch.addConstraint({ConstraintType::Horizontal, {{line.value(), SubElement::Whole}}, std::nullopt, std::nullopt});
    ASSERT_TRUE(constraint);
    const auto constraints = sketch.constraints();
    ASSERT_TRUE(constraints);
    ASSERT_EQ(constraints.value().size(), 1U);
    const ConstraintId constraintId = constraints.value()[0].id;
    staged->path() = "sketch.ourp";

    const DocumentId id = manager.openDocument(std::move(staged));

    EXPECT_EQ(staged, nullptr);
    ASSERT_EQ(manager.at(id), original);
    EXPECT_EQ(manager.count(), 1U);
    const Document& adopted = *manager.at(id);
    EXPECT_EQ(adopted.name(), "Loaded sketch");
    EXPECT_EQ(adopted.path(), "sketch.ourp");
    EXPECT_TRUE(adopted.isDirty());
    EXPECT_EQ(&adopted.sketch(), &sketch);
    const auto entity = adopted.sketch().entity(line.value());
    ASSERT_TRUE(entity);
    const auto& geometry = std::get<Line2>(entity.value().geometry);
    EXPECT_EQ(geometry.start, (Vec2{1, 2}));
    EXPECT_EQ(geometry.end, (Vec2{5, 2}));
    EXPECT_TRUE(adopted.sketch().constraint(constraintId));
}

TEST(DocumentManager, RejectsNullAdoptionWithoutChangingDocumentsOrConsumingAnId) {
    DocumentManager manager;
    const DocumentId first = manager.createNewDocument("First");
    Document* original = manager.at(first);

    EXPECT_THROW(manager.openDocument(nullptr), std::invalid_argument);

    EXPECT_EQ(manager.count(), 1U);
    EXPECT_EQ(manager.at(first), original);
    EXPECT_EQ(manager.createNewDocument("Second"), first + 1);
}

TEST(DocumentManager, ClosingAMiddleDocumentPreservesOtherIdsAndBorrowedAddresses) {
    DocumentManager manager;
    const DocumentId first = manager.createNewDocument("First");
    const DocumentId middle = manager.createNewDocument("Middle");
    const DocumentId last = manager.createNewDocument("Last");
    Document* firstDocument = manager.at(first);
    Document* lastDocument = manager.at(last);

    ASSERT_TRUE(manager.closeDocument(middle));

    EXPECT_EQ(manager.count(), 2U);
    EXPECT_EQ(manager.at(middle), nullptr);
    EXPECT_EQ(manager.at("Middle"), nullptr);
    EXPECT_EQ(manager.findIdByName("Middle"), InvalidDocumentId);
    EXPECT_FALSE(manager.isDocumentSaved(middle));
    EXPECT_FALSE(manager.closeDocument(middle));
    EXPECT_EQ(manager.at(first), firstDocument);
    EXPECT_EQ(manager.at(last), lastDocument);
    EXPECT_EQ(manager.at("Last"), lastDocument);
    EXPECT_EQ(manager.findIdByName("Last"), last);
    EXPECT_TRUE(manager.isDocumentSaved(last));
    EXPECT_TRUE(manager.isAllDocumentsSaved());
    EXPECT_GT(manager.createNewDocument("Next"), last);
}

TEST(DocumentManager, NameLookupsDoNotMarkUnchangedDocumentsDirty) {
    DocumentManager manager;
    const DocumentId id = manager.createNewDocument("Unchanged");
    const DocumentManager& query = manager;

    EXPECT_EQ(query.at("Unchanged"), query.at(id));
    EXPECT_EQ(query.findIdByName("Unchanged"), id);
    EXPECT_TRUE(query.isDocumentSaved("Unchanged"));
    EXPECT_TRUE(query.isDocumentSaved(id));
    EXPECT_TRUE(query.isAllDocumentsSaved());
    ASSERT_NE(query.at(id), nullptr);
    EXPECT_FALSE(query.at(id)->isDirty());
}

TEST(DocumentManager, RenamingPreservesTheIdAndClosingDoesNotAttemptToSave) {
    DocumentManager manager;
    const DocumentId id = manager.createNewDocument("Original");
    Document* document = manager.at(id);
    ASSERT_NE(document, nullptr);
    document->name() = "Renamed";
    EXPECT_EQ(manager.at("Original"), nullptr);
    EXPECT_EQ(manager.at("Renamed"), document);
    EXPECT_EQ(manager.findIdByName("Renamed"), id);
    EXPECT_FALSE(manager.isDocumentSaved("Renamed"));
    EXPECT_FALSE(manager.isAllDocumentsSaved());

    // Dirty documents without a file path still close: persistence is the caller's decision.
    EXPECT_TRUE(manager.closeDocument("Renamed"));
    EXPECT_EQ(manager.at(id), nullptr);
    EXPECT_EQ(manager.count(), 0U);
    EXPECT_TRUE(manager.isAllDocumentsSaved());
    EXPECT_FALSE(manager.closeDocument("Renamed"));
}

TEST(DocumentManager, ClosingAllDocumentsNeverReusesIdsEvenAcrossRepeatedCalls) {
    DocumentManager manager;
    const DocumentId first = manager.createNewDocument("First");
    const DocumentId last = manager.createNewDocument("Last");
    ASSERT_NE(manager.at(last), nullptr);
    manager.at(last)->name() = "Dirty";
    ASSERT_TRUE(manager.closeDocument(first));

    ASSERT_TRUE(manager.closeAllDocuments());
    EXPECT_EQ(manager.count(), 0U);
    EXPECT_EQ(manager.at(first), nullptr);
    EXPECT_EQ(manager.at(last), nullptr);
    EXPECT_EQ(manager.at("Dirty"), nullptr);
    EXPECT_TRUE(manager.isAllDocumentsSaved());
    ASSERT_TRUE(manager.closeAllDocuments());

    const DocumentId next = manager.createNewDocument("New");
    EXPECT_GT(next, last);
    EXPECT_EQ(manager.count(), 1U);
    EXPECT_EQ(manager.at(first), nullptr);
    EXPECT_EQ(manager.at(last), nullptr);
    EXPECT_EQ(manager.findIdByName("New"), next);
}

}  // namespace
