//
// Created by Eugene Bychkov on 28.09.2024.
//

#include "objectInFile.h"

std::string objectInFile::to_string() const {
    if (s_obj->type() == ET_POINT) {
        point *p = dynamic_cast<point *>(s_obj);
        return "{\nID " + std::to_string(id.id) + "\npoint " + std::to_string(p->x) + " " + std::to_string(p->y) +
               "\n}";
    } else if (s_obj->type() == ET_SECTION) {
        section *s = dynamic_cast<section *>(s_obj);
        return "{\nID " + std::to_string(id.id) + "\nsection" + "\n}";
    } else if (s_obj->type() == ET_CIRCLE) {
        circle *s = dynamic_cast<circle *>(s_obj);
        return "{\nID " + std::to_string(id.id) + "\ncircle " + std::to_string(s->R) + "\n}";
    }
    return "error";
}

objectInFile::objectInFile(const objectInFile &other) {
    s_obj = other.s_obj;
    id = other.id;
}

objectInFile::objectInFile(objectInFile &&other) noexcept {
    s_obj = other.s_obj;
    id = other.id;
    other.s_obj = nullptr;
    other.id = 0;
}

objectInFile &objectInFile::operator=(const objectInFile &other) {
    if (this != &other) {
        s_obj = other.s_obj;
        id = other.id;
    }
    return *this;
}

objectInFile &objectInFile::operator=(objectInFile &&other) noexcept {
    if (this != &other) {
        s_obj = other.s_obj;
        id = other.id;
        other.s_obj = nullptr;
        other.id = 0;
    }
    return *this;
}

std::pair<ID, primitive *> objectInFile::to_pair() const { return std::make_pair(id, s_obj); };

objectInFile::objectInFile(std::pair<ID, primitive *> &obj) {
    s_obj = obj.second;
    id = obj.first;
}




