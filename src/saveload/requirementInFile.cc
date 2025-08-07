#include "requirementInFile.h"

requirementInFile::requirementInFile(const std::pair<unsigned int, Requirement> &obj) : id(obj.first), s_req(obj.second) {}

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

std::pair<unsigned int, Requirement> requirementInFile::to_pair() const {
    return {id, s_req};
}

std::string requirementInFile::to_string() const {
    std::string s;
    s+= "{\n";
    s += "ID " + std::to_string(id) + "\n";
    s += "addreq " + std::to_string(static_cast<int>(s_req.type)) + " " + std::to_string(s_req.obj1.get()) + " " +
            std::to_string(s_req.obj2.get()) + " " + (!s_req.param.has_value() ? std::to_string(s_req.param.value()): "0") + "\n";
    s += "}";
    return s;
}



