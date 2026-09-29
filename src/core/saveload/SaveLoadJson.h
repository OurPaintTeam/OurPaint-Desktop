#ifndef JSONSAVER_H
#define JSONSAVER_H

#include <nlohmann/json.hpp>

#include "Scene.h"
#include "objectInJson.h"
#include "requirementInJson.h"

class SaveLoadJson {
    std::vector<objectInJson> _objects;
    std::vector<requirementInJson> _reqs;
    core::Scene& _scene;
public:
    SaveLoadJson(core::Scene& scene);
    //SaveLoadJson(const std::vector<objectInJson> &objects,const std::vector<requirementInJson> &reqs);
    nlohmann::json to_json() const;
    void from_json(const nlohmann::json &obj);
    void loadToScene();
    std::vector<core::ObjectData> getObjects();
    std::vector<core::Requirement> getRequirements();
};

#endif  // JSONSAVER_H
