#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <random>
#include <string>

#include "Document.h"
#include "UIController.h"

namespace app {
namespace {
namespace fs = std::filesystem;

class UIControllerDocuments : public testing::Test {
protected:
    void SetUp() override {
        std::random_device random;
        for (int i = 0; i < 16; ++i) {
            auto candidate = fs::temp_directory_path() / ("ourpaint-documents-" + std::to_string(random()));
            std::error_code error;
            if (fs::create_directory(candidate, error)) {
                root = std::move(candidate);
                return;
            }
            ASSERT_FALSE(error) << error.message();
        }
        FAIL() << "Could not create an exclusive temporary directory";
    }
    void TearDown() override {
        if (!root.empty()) {
            std::error_code error;
            fs::remove_all(root, error);
        }
    }
    fs::path root;
};

TEST_F(UIControllerDocuments, DraftIsOnlyInMemoryAndLastViewDoesNotOwnTheDocument) {
    UIController ui;
    EXPECT_FALSE(ui.loadApplicationMetadata(root / ".ourpaint"));
    const auto document = ui.createDocument("Untitled");
    const auto first = ui.createDocumentView(document);
    const auto second = ui.createDocumentView(document);
    EXPECT_FALSE(ui.documentModified(document));
    EXPECT_TRUE(ui.documentPath(document).empty());
    EXPECT_TRUE(ui.recentDocuments().empty());
    EXPECT_FALSE(fs::exists(root / ".ourpaint"));
    ui.closeDocumentView(first);
    EXPECT_NE(ui.findViewport(second), nullptr);
    ui.closeDocumentView(second);
    EXPECT_EQ(ui.documentName(document), "Untitled");
    EXPECT_TRUE(ui.closeDocument(document));
    EXPECT_TRUE(ui.documents().empty());
    EXPECT_TRUE(fs::is_empty(root));
}

TEST_F(UIControllerDocuments, SaveAndReopenPreserveIdentityAndFailedSavePreservesDestination) {
    UIController ui;
    const auto document = ui.createDocument("Untitled");
    ASSERT_TRUE(ui.renameDocument(document, "Plate"));
    EXPECT_TRUE(ui.documentModified(document));
    const auto first = root / "first.ourp";
    ASSERT_TRUE(ui.saveDocumentAs(document, first));
    EXPECT_FALSE(ui.documentModified(document));
    const auto opened = ui.openDocument(root / "." / "first.ourp");
    ASSERT_TRUE(opened);
    EXPECT_EQ(opened.value(), document);
    EXPECT_EQ(ui.documents().size(), 1U);
    EXPECT_TRUE(ui.renameDocument(document, "Changed"));
    EXPECT_FALSE(ui.saveDocumentAs(document, root / "missing" / "fail.ourp"));
    EXPECT_EQ(ui.documentPath(document), fs::weakly_canonical(first));
    EXPECT_TRUE(ui.documentModified(document));
    UIController reader;
    const auto loaded = reader.openDocument(first);
    ASSERT_TRUE(loaded);
    EXPECT_EQ(reader.documentName(loaded.value()), "Plate");
    EXPECT_FALSE(reader.documentModified(loaded.value()));
    EXPECT_TRUE(ui.saveDocument(document));
    EXPECT_FALSE(ui.documentModified(document));
}

TEST_F(UIControllerDocuments, SavingAnOpenedDocumentPreservesGeometryAndIndependentViews) {
    core::Document source("Geometry");
    ASSERT_TRUE(source.sketch().addPoint({2.0, 3.0}));
    ASSERT_TRUE(source.sketch().addLine({0.0, 0.0}, {4.0, 0.0}));
    const auto encoded = core::persistence::encodeDocument(source);
    ASSERT_TRUE(encoded);
    const auto original = root / "original.ourp";
    {
        std::ofstream stream(original, std::ios::binary);
        stream.write(reinterpret_cast<const char*>(encoded.value().data()), std::streamsize(encoded.value().size()));
        ASSERT_TRUE(stream);
    }
    UIController ui;
    const auto opened = ui.openDocument(original);
    ASSERT_TRUE(opened);
    const auto first = ui.createDocumentView(opened.value());
    const auto second = ui.createDocumentView(opened.value());
    auto* viewport = ui.findViewport(second);
    const auto copy = root / "copy.ourp";
    ASSERT_TRUE(ui.saveDocumentAs(opened.value(), copy));
    EXPECT_EQ(ui.findViewport(second), viewport);
    ui.closeDocumentView(first);
    EXPECT_EQ(ui.documentOf(second), opened.value());
    std::ifstream stream(copy, std::ios::binary);
    const core::persistence::DocumentBytes bytes((std::istreambuf_iterator<char>(stream)), {});
    EXPECT_EQ(bytes, encoded.value());
    EXPECT_TRUE(fs::exists(original));
}

TEST_F(UIControllerDocuments, InvalidInputAndDestinationCollisionLeaveOpenViewsIntact) {
    UIController ui;
    const auto document = ui.createDocument("A");
    const auto view = ui.createDocumentView(document);
    auto* viewport = ui.findViewport(view);
    const auto file = root / "a.ourp";
    ASSERT_TRUE(ui.saveDocumentAs(document, file));
    const auto other = ui.createDocument("B");
    EXPECT_FALSE(ui.saveDocumentAs(other, file));
    EXPECT_TRUE(ui.documentPath(other).empty());
    const auto corrupt = root / "corrupt.ourp";
    std::ofstream(corrupt, std::ios::binary) << "not a document";
    EXPECT_FALSE(ui.openDocument(corrupt));
    EXPECT_FALSE(ui.openDocument(root / "missing.ourp"));
    EXPECT_EQ(ui.documents().size(), 2U);
    EXPECT_EQ(ui.findViewport(view), viewport);
    EXPECT_TRUE(ui.closeDocument(document));
    EXPECT_EQ(ui.findViewport(view), nullptr);
    EXPECT_NE(ui.createDocument("C"), document);
}

TEST_F(UIControllerDocuments, MetadataContainsRecentPathsAndNoUnsavedDraft) {
    const auto metadata = root / "settings" / ".ourpaint";
    const auto file = root / "drawing.ourp";
    {
        UIController ui;
        ASSERT_FALSE(ui.loadApplicationMetadata(metadata));
        ui.createDocument("Private unsaved draft");
        const auto document = ui.createDocument("Drawing");
        ASSERT_TRUE(ui.saveDocumentAs(document, file));
        ASSERT_FALSE(ui.metadataError());
        ASSERT_TRUE(ui.saveDocument(document));
        ASSERT_EQ(ui.recentDocuments().size(), 1U);
    }
    UIController restored;
    ASSERT_FALSE(restored.loadApplicationMetadata(metadata));
    ASSERT_EQ(restored.recentDocuments().size(), 1U);
    EXPECT_EQ(restored.recentDocuments().front(), fs::weakly_canonical(file));
    EXPECT_TRUE(restored.documents().empty());
    std::ifstream stream(metadata);
    const std::string text((std::istreambuf_iterator<char>(stream)), {});
    EXPECT_EQ(text.find("Private unsaved draft"), std::string::npos);
}

TEST_F(UIControllerDocuments, BrokenMetadataIsNotOverwrittenAndDoesNotFailDocumentSave) {
    const auto metadata = root / ".ourpaint";
    std::ofstream(metadata) << "future or corrupt metadata";
    UIController ui;
    EXPECT_TRUE(ui.loadApplicationMetadata(metadata));
    const auto document = ui.createDocument("Drawing");
    EXPECT_TRUE(ui.saveDocumentAs(document, root / "drawing.ourp"));
    EXPECT_FALSE(ui.documentModified(document));
    EXPECT_TRUE(ui.metadataError());
    std::ifstream stream(metadata);
    const std::string contents((std::istreambuf_iterator<char>(stream)), {});
    EXPECT_EQ(contents, "future or corrupt metadata");
}

TEST_F(UIControllerDocuments, MetadataWriteFailureDoesNotFailDocumentSave) {
    const auto metadata = root / "blocked" / ".ourpaint";
    UIController ui;
    ASSERT_FALSE(ui.loadApplicationMetadata(metadata));
    std::ofstream(root / "blocked") << "This is a file, not a directory";
    const auto document = ui.createDocument("Drawing");
    EXPECT_TRUE(ui.saveDocumentAs(document, root / "drawing.ourp"));
    EXPECT_FALSE(ui.documentModified(document));
    EXPECT_TRUE(ui.metadataError());
}
}  // namespace
}  // namespace app
