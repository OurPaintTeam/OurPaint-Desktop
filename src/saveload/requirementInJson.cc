#include "requirementInJson.h"

requirementInJson::requirementInJson(const nlohmann::json& req) {
    // Type and name
    _name = req.at("name").get<std::string>();
    _req.type = requirement_from_string(req.at("type").get<std::string>());
    _req.id = ID(req.at("id").get<unsigned int>());

    // Objects
    const auto& arr = req.at("object");
    if (!arr.is_array()) {
        throw std::runtime_error("must be an array");
    }
    switch (arr.size()) {
        case 0:
            break;
        case 1:
            _req.obj1 = ID(arr[0].get<unsigned int>());
            break;
        case 2:
            _req.obj1 = ID(arr[0].get<unsigned int>());
            _req.obj2 = ID(arr[1].get<unsigned int>());
            break;
        default:
            _req.obj1 = ID(arr[0].get<unsigned int>());
            _req.obj2 = ID(arr[1].get<unsigned int>());
            _req.obj3 = ID(arr[2].get<unsigned int>());
            if (arr.size() > 3) {
                // it's not ok.
            }
            break;
    }

    // param (optional)
    if (req.contains("param")) {
        _req.param = req.at("param").get<double>();
    }
    else {
        _req.param.reset();
    }
}

requirementInJson::requirementInJson(Requirement req, const std::string& name) {
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

requirementInJson& requirementInJson::operator=(const requirementInJson& other) {
    _name = other._name;
    _req = other._req;
    return *this;
}

requirementInJson& requirementInJson::operator=(requirementInJson&& other) noexcept {
    _name = std::move(other._name);
    _req = std::move(other._req);
    return *this;
}
std::pair<std::string, Requirement> requirementInJson::to_pair() const {
    return std::make_pair(_name, _req);
}
nlohmann::json requirementInJson::to_json() const {
    nlohmann::json json;

    json["id"] = _req.id.get();
    json["type"] = to_string(_req.type);
    json["name"] = _name;

    nlohmann::json arr = nlohmann::json::array();
    if (_req.obj1 != ID(0)) arr.push_back(_req.obj1.get());
    if (_req.obj2 != ID(0)) arr.push_back(_req.obj2.get());
    if (_req.obj3 != ID(0)) arr.push_back(_req.obj3.get());
    json["object"] = std::move(arr);

    if (_req.param.has_value()) {
        json["param"] = *_req.param;
    }
    return json;
}

