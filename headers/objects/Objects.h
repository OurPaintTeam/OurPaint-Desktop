#ifndef OURPAINT_HEADERS_OBJECTS_H_
#define OURPAINT_HEADERS_OBJECTS_H_

#include <vector>
#include <optional>
#include "ID.h"
#include "Enums.h"

struct ObjectData {
    ObjType et;
    std::vector<ID> subObjects;
    std::vector<double> params;
    ID id;

    bool operator==(const ObjectData& other) const {
        return et == other.et && params == other.params;
    }

    bool operator!=(const ObjectData& other) const {
        return !(*this == other);
    }
};

struct Requirement {
    ID id{0};
    ReqType type;
    ID obj1{0};
    ID obj2{0};
    ID obj3{0};
    std::optional<double> param;
};

inline bool operator==(Requirement const& a, Requirement const& b) noexcept {
    return a.type      == b.type
           && a.obj1   == b.obj1
           && a.obj2   == b.obj2
           && a.obj3   == b.obj3
           && a.param  == b.param;
}

inline bool operator!=(Requirement const& a, Requirement const& b) noexcept {
    return !(a == b);
}

namespace std {
    template<>
    struct hash<Requirement> {
        size_t operator()(Requirement const& r) const noexcept {
            auto h1 = std::hash<uint8_t>()(static_cast<uint8_t>(r.type));
            auto h2 = std::hash<ID>()(r.obj1);
            auto h3 = std::hash<ID>()(r.obj2);
            auto h4 = std::hash<ID>()(r.obj3);
            auto h5 = r.param ? std::hash<double>()(*r.param) : 0u;
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3) ^ (h5 << 4);
        }
    };
}

#endif // ! OURPAINT_HEADERS_OBJECTS_H_