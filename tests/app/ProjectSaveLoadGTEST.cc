#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "DocumentManager.h"
#include "ProjectSaveLoad.h"

namespace app {
namespace {

namespace fs = std::filesystem;

class ProjectPersistence : public testing::Test {
protected:
    void SetUp() override {
        std::random_device random;
        for (unsigned attempt = 0; attempt < 16; ++attempt) {
            const auto candidate = fs::temp_directory_path() / ("ourpaint-persistence-test-" + std::to_string(random()));
            std::error_code error;
            if (fs::create_directory(candidate, error)) {
                root_ = candidate;
                return;
            }
            ASSERT_FALSE(error) << error.message();
        }
        FAIL() << "Could not create an exclusive temporary directory";
    }

    void TearDown() override {
        if (!root_.empty()) {
            std::error_code error;
            fs::remove_all(root_, error);
        }
    }

    fs::path root_;
};

TEST_F(ProjectPersistence, SavesLoadsAndAdoptsDocumentsWithoutChangingTheirIdentityOrSource) {
    Document first("Same name");
    Document second("Same name");
    ASSERT_TRUE(first.sketch().addPoint({2, 3}));
    ASSERT_TRUE(second.sketch().addLine({0, 0}, {4, 0}));
    const auto firstBytes = core::persistence::encodeDocument(first);
    ASSERT_TRUE(firstBytes);
    const auto secondBytes = core::persistence::encodeDocument(second);
    ASSERT_TRUE(secondBytes);
    Project source{root_ / "source", {"11111111111111111111111111111111", "Example"},
                   {{"z.ourp", std::cref(second)}, {"a.ourp", std::cref(first)}}};
    const auto copy = root_ / "copy";
    const auto saved = saveProjectAs(source, copy);
    ASSERT_TRUE(saved);
    EXPECT_EQ(saved.saved, (std::vector<fs::path>{copy / "a.ourp", copy / "z.ourp", copy / ".ourpaint"}));
    EXPECT_TRUE(saved.unprocessed.empty());
    EXPECT_NE(saved.metadata.id, source.metadata.id);
    EXPECT_EQ(source.metadata.id, "11111111111111111111111111111111");
    EXPECT_FALSE(fs::exists(source.root));
    EXPECT_TRUE(std::as_const(first).path().empty());
    EXPECT_TRUE(first.isDirty());

    // Neither an unrelated file nor a directory ending in .ourp is a model.
    ASSERT_TRUE(fs::create_directory(copy / "nested.ourp"));
    std::ofstream(copy / "notes.txt") << "Keep this file";
    auto loaded = loadProject(copy);
    ASSERT_TRUE(loaded);
    ASSERT_EQ(loaded.value().documents.size(), 2U);
    EXPECT_EQ(loaded.value().documents[0].filename, "a.ourp");
    EXPECT_EQ(loaded.value().documents[1].filename, "z.ourp");
    EXPECT_EQ(loaded.value().metadata.id, saved.metadata.id);
    const auto restoredFirst = core::persistence::encodeDocument(*loaded.value().documents[0].document);
    ASSERT_TRUE(restoredFirst);
    EXPECT_EQ(restoredFirst.value(), firstBytes.value());
    const auto restoredSecond = core::persistence::encodeDocument(*loaded.value().documents[1].document);
    ASSERT_TRUE(restoredSecond);
    EXPECT_EQ(restoredSecond.value(), secondBytes.value());

    auto view = loaded.value().view();
    DocumentManager manager;
    const auto firstId = manager.openDocument(std::move(loaded.value().documents[0].document));
    manager.openDocument(std::move(loaded.value().documents[1].document));
    EXPECT_EQ(manager.at(firstId), &view.documents[0].document.get());
    EXPECT_THROW(static_cast<void>(loaded.value().view()), std::logic_error);
    EXPECT_TRUE(saveProject(view));
    EXPECT_TRUE(fs::exists(copy / "notes.txt"));
    EXPECT_EQ(view.metadata.id, saved.metadata.id);
}

TEST_F(ProjectPersistence, RejectsACorruptLaterDocumentWithoutChangingOpenModelsOrFiles) {
    Document document("Saved");
    Project source{{}, {{}, "Example"}, {{"z.ourp", std::cref(document)}, {"a.ourp", std::cref(document)}}};
    const auto copy = root_ / "copy";
    ASSERT_TRUE(saveProjectAs(source, copy));
    {
        std::ofstream broken(copy / "z.ourp", std::ios::binary | std::ios::trunc);
        broken << "Broken CBOR";
        ASSERT_TRUE(broken.good());
    }
    DocumentManager manager;
    const auto id = manager.createNewDocument("Open model");
    const auto* before = manager.at(id);
    const auto loaded = loadProject(copy);
    ASSERT_FALSE(loaded);
    EXPECT_EQ(loaded.error().path, copy / "z.ourp");
    EXPECT_EQ(loaded.error().stage, FileStage::Decode);
    ASSERT_TRUE(loaded.error().codecError);
    EXPECT_EQ(manager.count(), 1U);
    EXPECT_EQ(manager.at(id), before);
    EXPECT_EQ(std::as_const(*manager.at(id)).name(), "Open model");
    std::ifstream broken(copy / "z.ourp", std::ios::binary);
    std::string contents;
    std::getline(broken, contents);
    EXPECT_EQ(contents, "Broken CBOR");
}

TEST_F(ProjectPersistence, EncodingFailureLeavesTheSaveAsDirectoryAbsent) {
    Document valid("Valid");
    Document invalid(std::string("\xc3\x28", 2));
    Project source{{}, {{}, "Example"}, {{"a.ourp", std::cref(valid)}, {"z.ourp", std::cref(invalid)}}};
    const auto copy = root_ / "copy";
    const auto saved = saveProjectAs(source, copy);
    ASSERT_FALSE(saved);
    ASSERT_TRUE(saved.error);
    EXPECT_EQ(saved.error->path, copy / "z.ourp");
    EXPECT_EQ(saved.error->stage, FileStage::Encode);
    EXPECT_TRUE(saved.saved.empty());
    EXPECT_EQ(saved.unprocessed.size(), 3U);
    EXPECT_FALSE(saved.failedFileReplaced);
    EXPECT_FALSE(fs::exists(copy));
    EXPECT_TRUE(source.metadata.id.empty());
}

}  // namespace
}  // namespace app
