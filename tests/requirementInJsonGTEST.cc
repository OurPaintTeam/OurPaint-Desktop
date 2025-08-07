#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "requirementInJson.h"


TEST(RequirementInJsonTest, SerializeDeserialize) {
    Requirement data;
    data.type = ReqType::ET_POINTSECTIONDIST;
    data.id = ID(42);
    data.obj1 = ID(1);
    data.obj2 = ID(2);
    data.param = 100.5;

    std::string name = "ConstraintTest";
    requirementInJson r(data, name);

    nlohmann::json j = r.to_json();

    EXPECT_EQ(j["id"], 42);
    EXPECT_EQ(j["type"], "pointsectiondist");
    EXPECT_EQ(j["name"], name);
    ASSERT_EQ(j["object"].size(), 2);
    EXPECT_EQ(j["object"][0], 1);
    EXPECT_EQ(j["object"][1], 2);
    EXPECT_DOUBLE_EQ(j["param"], 100.5);

    requirementInJson r2(j);
    auto [name2, data2] = r2.to_pair();

    EXPECT_EQ(name2, name);
    EXPECT_EQ(data2.id.get(), 42);
    EXPECT_EQ(data2.type, ReqType::ET_POINTSECTIONDIST);
    EXPECT_EQ(data2.obj1.get(), 1);
    EXPECT_EQ(data2.obj2.get(), 2);
    EXPECT_DOUBLE_EQ(data2.param.value(), 100.5);
}

TEST(RequirementInJsonTest, CopyConstructorAndAssignment) {
    Requirement data;
    data.type = ReqType::ET_POINTSECTIONDIST;
    data.id = ID(1);
    data.obj1 = ID(10);
    data.obj2 = ID(20);
    data.param = { 300.0 };

    std::string name = "CopyTest";
    requirementInJson orig(data, name);
    requirementInJson copy = orig;

    EXPECT_EQ(copy.to_json(), orig.to_json());

    requirementInJson assigned(data, "temp");
    assigned = orig;
    EXPECT_EQ(assigned.to_json(), orig.to_json());
}

TEST(RequirementInJsonTest, MoveConstructorAndAssignment) {
    Requirement data;
    data.type = ReqType::ET_POINTSECTIONDIST;
    data.id = ID(5);
    data.obj1 = ID(100);
    data.obj2 = ID(200);
    data.param = 50.0;

    std::string name = "MoveTest";
    requirementInJson orig(data, name);

    requirementInJson moved = std::move(orig);
    EXPECT_EQ(moved.to_pair().first, name);
    EXPECT_EQ(moved.to_pair().second.id.get(), 5);

    requirementInJson another(data, "temp");
    another = std::move(moved);
    EXPECT_EQ(another.to_pair().second.obj1.get(), 100);
}
