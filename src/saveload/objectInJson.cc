//
// Created by Eugene Bychkov on 30.06.2025.
//
#include "objectInJson.h"
objectInJson::objectInJson(const std::vector<std::string>& name, const ObjectData& obj) {
  _name = name;
  _obj = obj;
  Element t = _obj.et;
  ObjectData p;
  _subObjects = std::vector<ObjectData>();
  switch (t) {
    case ET_POINT:
      break;
    case ET_SECTION:
      p.et = ET_POINT;
      p.params.push_back(obj.params[0]);
      p.params.push_back(obj.params[1]);
      p.subObjects.push_back(obj.subObjects[0]);
      _subObjects.push_back(p);
      p.params.clear();
      p.subObjects.clear();
      p.params.push_back(obj.params[2]);
      p.params.push_back(obj.params[3]);
      p.subObjects.push_back(obj.subObjects[1]);
      break;
    case ET_CIRCLE:
      p.et = ET_POINT;
      p.params.push_back(obj.params[0]);
      p.params.push_back(obj.params[1]);
      p.subObjects.push_back(obj.subObjects[0]);
      _subObjects.push_back(p);
      break;
    case ET_ARC:
      p.et = ET_POINT;
      p.params.push_back(obj.params[0]);
      p.params.push_back(obj.params[1]);
      p.subObjects.push_back(obj.subObjects[0]);
      _subObjects.push_back(p);
      p.params.clear();
      p.subObjects.clear();
      p.params.push_back(obj.params[2]);
      p.params.push_back(obj.params[3]);
      p.subObjects.push_back(obj.subObjects[1]);
      p.params.clear();
      p.subObjects.clear();
      p.params.push_back(obj.params[4]);
      p.params.push_back(obj.params[5]);
      p.subObjects.push_back(obj.subObjects[2]);
      break;
  }
}
objectInJson::objectInJson(const objectInJson& other) {
  _obj = other._obj;
  _name = other._name;
  _subObjects = other._subObjects;
}
objectInJson::objectInJson(objectInJson&& other) noexcept {
  std::swap(_obj, other._obj);
  std::swap(_name, other._name);
  std::swap(_subObjects, other._subObjects);
}
objectInJson& objectInJson::operator=(const objectInJson& other) {
  _obj = other._obj;
  _name = other._name;
  _subObjects = other._subObjects;
  return *this;
}
objectInJson& objectInJson::operator=(objectInJson&& other) noexcept {
  std::swap(_obj, other._obj);
  std::swap(_name, other._name);
  std::swap(_subObjects, other._subObjects);
  return *this;
}
nlohmann::json objectInJson::to_json() const {
  nlohmann::json json;

  json["id"] = _obj.id.get();
  json["type"] = to_string(_obj.et);
  json["name"] = _name.at(0);

  nlohmann::json points = nlohmann::json::array();
  for (std::size_t i = 0; i < _subObjects.size(); ++i) {
    const ObjectData& pt = _subObjects[i];
    nlohmann::json ptJson;
    ptJson["id"] = pt.subObjects[0].get();
    ptJson["type"] = "point";
    ptJson["name"] = _name.at(i + 1);
    ptJson["x"] = pt.params[0];
    ptJson["y"] = pt.params[1];
    points.push_back(ptJson);
  }

  json["points"] = points;
  return json;
}

void objectInJson::from_json(const nlohmann::json& j) {
  _obj.id = ID(j.at("id").get<unsigned int>());
  _obj.et = element_from_string(j.at("type").get<std::string>());
  _name.clear();
  _name.push_back(j.at("name").get<std::string>());

  _obj.params.clear();
  _obj.subObjects.clear();
  _subObjects.clear();

  const auto& points = j.at("points");
  for (const auto& ptJson : points) {
    ID ptId(ptJson.at("id").get<unsigned int>());
    std::string ptName = ptJson.at("name").get<std::string>();
    double x = ptJson.at("x").get<double>();
    double y = ptJson.at("y").get<double>();

    _name.push_back(ptName);

    _obj.params.push_back(x);
    _obj.params.push_back(y);
    _obj.subObjects.push_back(ptId);

    ObjectData pt;
    pt.et = ET_POINT;
    pt.params = {x, y};
    pt.subObjects = {ptId};

    _subObjects.push_back(pt);
  }
}
