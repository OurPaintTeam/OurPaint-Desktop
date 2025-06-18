#include "objectInFile.h"

std::string objectInFile::to_string() const {
    if (s_obj->getType() == Element::ET_POINT) {
        Point *p = dynamic_cast<Point *>(s_obj);
        return "{\nID " + std::to_string(id) + "\npoint " + std::to_string(p->x) + " " + std::to_string(p->y) +
               "\n}";
    } else if (s_obj->getType() == Element::ET_SECTION) {
        return "{\nID " + std::to_string(id) + "\nsection" + "\n}";
    } else if (s_obj->getType() == Element::ET_CIRCLE) {
        Circle *s = dynamic_cast<Circle *>(s_obj);
        return "{\nID " + std::to_string(id) + "\ncircle " + std::to_string(s->r) + "\n}";
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

std::pair<unsigned int, IGeometricObject *> objectInFile::to_pair() const { return std::make_pair(id, s_obj); };

objectInFile::objectInFile(std::pair<unsigned int, IGeometricObject *> &obj) : id(obj.first), s_obj(obj.second) {}