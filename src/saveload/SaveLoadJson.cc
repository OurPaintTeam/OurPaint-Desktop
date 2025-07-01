#include "SaveLoadJson.h"

SaveLoadJson::SaveLoadJson(Scene &scene): _scene(scene) {
  std::vector<ObjectData> obj = _scene.getObjects();
  std::vector<RequirementData> req = _scene.getRequirements();
// TODO get names from Timofey's classes
  for (const auto &o : obj) {
    _objects.push_back(objectInJson(std::vector<std::string>(),o));
  }
  for (const auto &r : req) {
    _reqs.push_back(requirementInJson(r, std::string()));
  }
}
SaveLoadJson::SaveLoadJson(const std::vector<objectInJson> &objects,
                           const std::vector<requirementInJson> &reqs)
    : _objects(objects), _reqs(reqs), _scene() {}

nlohmann::json SaveLoadJson::to_json() const {
  nlohmann::json j;
  j["objects"] = nlohmann::json::array();
  j["reqs"] = nlohmann::json::array();

  for (const auto &o : _objects) {
    j["objects"].push_back(o.to_json());
  }
  for (const auto &r : _reqs) {
    j["reqs"].push_back(r.to_json());
  }
  return j;
}
void SaveLoadJson::from_json(const nlohmann::json &obj) {
  for (const auto &o : obj["objects"]) {
    _objects.push_back(o);
  }
  for (const auto &r : obj["reqs"]) {
    _reqs.push_back(r);
  }
}