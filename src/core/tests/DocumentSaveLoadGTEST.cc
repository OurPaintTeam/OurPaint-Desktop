#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "Document.h"
#include "Scene.h"
#include "objects/Objects.h"
#include "saveload/DocumentSaveLoad.h"

#include <nlohmann/json.hpp>

namespace {

std::filesystem::path tempOurpPath(const std::string& name) {
    return std::filesystem::temp_directory_path() / name;
}

ObjectData makePoint(double x, double y) {
    ObjectData object;
    object.et = ObjType::ET_POINT;
    object.params = {x, y};
    return object;
}

ObjectData makeLine(double x1, double y1, double x2, double y2) {
    ObjectData object;
    object.et = ObjType::ET_LINE;
    object.params = {x1, y1, x2, y2};
    return object;
}

ObjectData makeCircle(double x, double y, double r) {
    ObjectData object;
    object.et = ObjType::ET_CIRCLE;
    object.params = {x, y, r};
    return object;
}

ObjectData makeArc(double x1, double y1, double x2, double y2, double cx, double cy) {
    ObjectData object;
    object.et = ObjType::ET_ARC;
    object.params = {x1, y1, x2, y2, cx, cy};
    return object;
}

ObjectData makeBezier() {
    ObjectData object;
    object.et = ObjType::ET_CUBIC_BEZIER;
    object.params = {0, 0, 30, 0, 10, 15, 20, 15};
    return object;
}

} // namespace

TEST(DocumentSaveLoadTest, RoundTripsObjectsAndRequirements) {
    const auto path = tempOurpPath("ourpaint_roundtrip.ourp");
    const auto path2 = tempOurpPath("ourpaint_roundtrip_second.ourp");
    std::filesystem::remove(path);
    std::filesystem::remove(path2);

    Document document("RoundTrip");
    Scene& scene = document.scene();

    ID pointId = scene.addObject(makePoint(1, 2));
    ID lineId = scene.addObject(makeLine(0, 0, 10, 0));
    scene.addObject(makeCircle(5, 5, 3));
    scene.addObject(makeArc(0, 0, 10, 0, 5, 5));
    scene.addObject(makeBezier());

    ObjectData line = scene.getObjectData(lineId);
    Requirement requirement;
    requirement.type = ReqType::ET_POINTPOINTDIST;
    requirement.obj1 = pointId;
    requirement.obj2 = line.subObjects[0];
    requirement.param = 7.0;
    ID requirementId = scene.addRequirement(requirement, false);
    ASSERT_NE(requirementId, ID(0));

    ASSERT_TRUE(saveDocumentToFile(path.string(), document));

    Document loaded;
    ASSERT_TRUE(loadDocumentFromFile(path.string(), loaded));
    const Document& loadedDocument = loaded;

    EXPECT_EQ(loadedDocument.name(), "RoundTrip");
    EXPECT_FALSE(loaded.isDirty());
    EXPECT_EQ(loaded.scene().getLines().size(), 1);
    EXPECT_EQ(loaded.scene().getCircles().size(), 1);
    EXPECT_EQ(loaded.scene().getArcs().size(), 1);
    EXPECT_EQ(loaded.scene().getBeziers().size(), 1);
    ASSERT_EQ(loaded.scene().getAllRequirementsData().size(), 1);
    EXPECT_EQ(loaded.scene().getAllRequirementsData()[0].type, ReqType::ET_POINTPOINTDIST);
    ASSERT_TRUE(loaded.scene().getAllRequirementsData()[0].param.has_value());
    EXPECT_DOUBLE_EQ(*loaded.scene().getAllRequirementsData()[0].param, 7.0);

    ASSERT_TRUE(saveDocumentToFile(path2.string(), loaded));
    {
        std::ifstream first(path);
        std::ifstream second(path2);
        EXPECT_EQ(nlohmann::json::parse(first), nlohmann::json::parse(second));
    }

    std::filesystem::remove(path);
    std::filesystem::remove(path2);
}

TEST(DocumentSaveLoadTest, SavesAndLoadsEmptyDocument) {
    const auto path = tempOurpPath("ourpaint_empty.ourp");
    std::filesystem::remove(path);

    Document document("Empty");
    ASSERT_TRUE(saveDocumentToFile(path.string(), document));

    Document loaded;
    ASSERT_TRUE(loadDocumentFromFile(path.string(), loaded));
    const Document& loadedDocument = loaded;

    EXPECT_EQ(loadedDocument.name(), "Empty");
    EXPECT_EQ(loaded.scene().objectsCount(), 0);
    EXPECT_EQ(loaded.scene().requirementsCount(), 0);

    std::filesystem::remove(path);
}

TEST(DocumentSaveLoadTest, MalformedJsonFailsWithoutClearingExistingDocument) {
    const auto path = tempOurpPath("ourpaint_bad.ourp");

    {
        std::ofstream out(path);
        out << "{not json";
    }

    Document document("Existing");
    document.scene().addObject(makePoint(1, 2));

    EXPECT_FALSE(loadDocumentFromFile(path.string(), document));
    EXPECT_EQ(document.scene().objectsCount(), 1);

    std::filesystem::remove(path);
}
