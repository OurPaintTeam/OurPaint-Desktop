#ifndef REQUIREMENTINJSON_H
#define REQUIREMENTINJSON_H
#include <nlohmann/json.hpp>
#include "Objects.h"

class requirementInJson {
  std::string _name;
  RequirementData _req;
public:
  requirementInJson(const nlohmann::json& req);
  requirementInJson(RequirementData req, const std::string& name);
  requirementInJson(const requirementInJson &other);
  requirementInJson(requirementInJson &&other) noexcept;
  requirementInJson &operator=(const requirementInJson &other);
  requirementInJson &operator=(requirementInJson &&other) noexcept;
  std::pair<std::string, RequirementData> to_pair() const;
  nlohmann::json to_json() const;
};
#endif //REQUIREMENTINJSON_H
