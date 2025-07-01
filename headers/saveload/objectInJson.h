#ifndef OBJECTINJSON_H
#define OBJECTINJSON_H
#include <nlohmann/json.hpp>
#include "Objects.h"
class objectInJson {
  std::vector<std::string> _name;
  ObjectData _obj;
  std::vector<ObjectData> _subObjects;
 public:
  objectInJson(const nlohmann::json &obj);
  objectInJson(const std::vector<std::string> &name, const ObjectData &obj);
  objectInJson(const objectInJson &other);
  objectInJson(objectInJson &&other) noexcept;
  objectInJson &operator=(const objectInJson &other);
  objectInJson &operator=(objectInJson &&other) noexcept;
  nlohmann::json to_json() const;
  std::pair<std::vector<std::string>, ObjectData> to_pair() const {
    return std::make_pair(_name, _obj);
  }
};

#endif  // OBJECTINJSON_H
