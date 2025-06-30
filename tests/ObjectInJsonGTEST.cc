#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "objectInJson.h"

TEST(objectInJsonTest, SectionSerializationDeserialization) {
  ObjectData obj;
  obj.id = ID(3);
  obj.et = ET_SECTION;
  obj.params = {100.0, 200.0, 300.0, 400.0};
  obj.subObjects = {ID(1), ID(2)};
  std::vector<std::string> names = {"sect2", "point1", "point2"};

  objectInJson o1(names, obj);
  nlohmann::json j = o1.to_json();

  EXPECT_EQ(j["id"], 3);
  EXPECT_EQ(j["type"], "section");
  EXPECT_EQ(j["name"], "sect2");
  EXPECT_EQ(j["points"].size(), 2);
  EXPECT_EQ(j["points"][0]["id"], 1);
  EXPECT_EQ(j["points"][0]["x"], 100.0);
  EXPECT_EQ(j["points"][0]["y"], 200.0);
  EXPECT_EQ(j["points"][1]["id"], 2);
  EXPECT_EQ(j["points"][1]["x"], 300.0);
  EXPECT_EQ(j["points"][1]["y"], 400.0);

  // Проверка десериализации
  objectInJson o2({}, {});
  o2.from_json(j);
  EXPECT_EQ(o2.to_json(), j);
}
