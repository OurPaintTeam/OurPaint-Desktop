//
// Created by Eugene Bychkov on 30.06.2025.
//
#include "requirementInJson.h"
requirementInJson::requirementInJson(const nlohmann::json& req) {
  _name = req.at("name").get<std::string>();
  _req.req = requirement_from_string(req.at("type").get<std::string>());
  _req.id = ID(req.at("id").get<unsigned int>());
  _req.objects.clear();
  for (const auto& objId : req.at("object")) {
    _req.objects.push_back(ID(objId.get<unsigned int>()));
  }
  _req.params.clear();
  if (req.contains("param")) {
    _req.params.push_back(req.at("param").get<double>());
  }
}

requirementInJson::requirementInJson(RequirementData req,
                                     const std::string& name) {
  _name = name;
  _req = req;
}
requirementInJson::requirementInJson(const requirementInJson& other) {
  _name = other._name;
  _req = other._req;
}
requirementInJson::requirementInJson(requirementInJson&& other) noexcept {
  std::swap(_name, other._name);
  std::swap(_req, other._req);
}

requirementInJson& requirementInJson::operator=(
    const requirementInJson& other) {
  _name = other._name;
  _req = other._req;
  return *this;
}

requirementInJson& requirementInJson::operator=(
    requirementInJson&& other) noexcept {
  _name = std::move(other._name);
  _req = std::move(other._req);
  return *this;
}
std::pair<std::string, RequirementData> requirementInJson::to_pair() const {
  return std::make_pair(_name, _req);
}
nlohmann::json requirementInJson::to_json() const {
  nlohmann::json json;
  json["id"] = _req.id.get();
  json["type"] = to_string(_req.req);
  json["name"] = _name;
  nlohmann::json obj = nlohmann::json::array();
  for (auto objId : _req.objects) {
    obj.push_back(objId.get());
  }
  json["object"] = obj;
  if (!_req.params.empty()) {
    json["param"] = _req.params[0];
  }
  return json;
}

