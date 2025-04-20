#include "requirementInFile.h"
requirementInFile::requirementInFile(const std::pair<unsigned int, RequirementData> &obj) : id(obj.first), s_req(obj.second) {}

requirementInFile::requirementInFile(const requirementInFile &other) {
    id = other.id;
    s_req = other.s_req;
}
requirementInFile::requirementInFile(requirementInFile &&other) noexcept{
    id = other.id;
    s_req = other.s_req;
}
requirementInFile &requirementInFile::operator=(const requirementInFile &other) = default;
requirementInFile &requirementInFile::operator=(requirementInFile &&other) noexcept{
    id = other.id;
    s_req = other.s_req;
    return *this;
}

std::pair<unsigned int, RequirementData> requirementInFile::to_pair() const {
    return {id, s_req};
}

std::string requirementInFile::to_string() const {
    std::string s;
    s+= "{\n";
    s += "ID " + std::to_string(id) + "\n";
    s += "addreq " + std::to_string(s_req.req) + " " + std::to_string(s_req.objects[0]) + " " +
            std::to_string(s_req.objects[1]) + " " + (!s_req.params.empty() ? std::to_string(s_req.params[0]): "0") + "\n";
    s += "}";
    return s;
}



