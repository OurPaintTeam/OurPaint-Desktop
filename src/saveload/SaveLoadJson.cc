#include "SaveLoadJson.h"

SaveLoadJson::SaveLoadJson(Scene &scene): _scene(scene) {
    std::vector<ObjectData> obj = _scene.getObjects();
    std::vector<Requirement> req = _scene.getRequirements();
    std::unordered_set<ID> childIds;
    for (const auto &o : obj) {
        for (const auto &sub : o.subObjects) {
            childIds.insert(sub);
        }
    }
    for (const auto &o : obj) {
        if (childIds.count(o.id) > 0) {
            continue;
        }
        std::vector<std::string> names = {"TODO"};
        names.insert(names.end(), o.subObjects.size(), "TODO");
        _objects.push_back(objectInJson(names, o));
    }

    for (const auto &r : req) {
        _reqs.push_back(requirementInJson(r, "TODO"));
    }
}


nlohmann::json SaveLoadJson::to_json() const {
    nlohmann::json j;
    j["objects"] = nlohmann::json::array();
    j["reqs"] = nlohmann::json::array();

    for (const auto &o: _objects) {
        j["objects"].push_back(o.to_json());
    }
    for (const auto &r: _reqs) {
        j["reqs"].push_back(r.to_json());
    }
    return j;
}

void SaveLoadJson::from_json(const nlohmann::json &obj) {
    for (const auto &o: obj["objects"]) {
        _objects.push_back(o);
    }
    for (const auto &r: obj["reqs"]) {
        _reqs.push_back(r);
    }
}

void SaveLoadJson::loadToScene() {
    std::vector<ObjectData> objs;
    std::vector<Requirement> reqs;
    for (const auto &o: _objects) {
        objs.push_back(o.to_pair().second);
    }
    for (const auto &r: _reqs) {
        reqs.push_back(r.to_pair().second);
    }
    _scene.load(objs, reqs);
}

std::vector<ObjectData> SaveLoadJson::getObjects() {
    std::vector<ObjectData> ret;
    //TODO get OBJDATA
    for (const auto &o: _objects) {
        ret.push_back(o.to_pair().second);
    }
    return ret;
}

std::vector<Requirement> SaveLoadJson::getRequirements() {
    std::vector<Requirement> ret;
    for (const auto &o: _reqs) {
        ret.push_back(o.to_pair().second);
    }
    return ret;
}
