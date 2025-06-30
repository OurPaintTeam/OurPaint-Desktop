//
// Created by Eugene Bychkov on 30.06.2025.
//

#ifndef OBJECTINJSON_H
#define OBJECTINJSON_H
#include <nlohmann/json.hpp>
#include "Objects.h"
class objectInJson {
  std::vector<std::string> _name;
  ObjectData _obj;
  std::vector<ObjectData> _subObjects;
public:
  objectInJson(const std::vector<std::string>& name, const ObjectData& obj);
  objectInJson(const objectInJson &other);
  objectInJson(objectInJson &&other) noexcept;
  objectInJson &operator=(const objectInJson &other);
  objectInJson &operator=(objectInJson &&other) noexcept;
  nlohmann::json to_json() const;
  void from_json(const nlohmann::json& obj);
};

#endif //OBJECTINJSON_H
