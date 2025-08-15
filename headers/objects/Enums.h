#ifndef OURPAINT_HEADERS_ENUMS_H_
#define OURPAINT_HEADERS_ENUMS_H_

#include <cstdint>
#include <span>
#include <string>

enum class ObjType : uint8_t {
    ET_POINT,
    ET_SECTION,
    ET_CIRCLE,
    ET_ARC,
    ERROR
};

enum class ReqType : uint8_t {
    ET_POINTSECTIONDIST,
    ET_POINTONSECTION,
    ET_POINTPOINTDIST,
    ET_POINTONPOINT,
    ET_SECTIONCIRCLEDIST,
    ET_SECTIONONCIRCLE,
    ET_SECTIONINCIRCLE,
    ET_SECTIONSECTIONPARALLEL,
    ET_SECTIONSECTIONPERPENDICULAR,
    ET_SECTIONSECTIONANGLE,
    ET_ARCCENTERONPERPENDICULAR,
    COUNT,
    ERROR
};

struct ReqRule {
    std::span<const ObjType> types;
    bool symmetric;
    bool needsParam;
};

constexpr ObjType P_S[] = {ObjType::ET_POINT, ObjType::ET_SECTION};
constexpr ObjType P_P[] = {ObjType::ET_POINT, ObjType::ET_POINT};
constexpr ObjType S_C[] = {ObjType::ET_SECTION, ObjType::ET_CIRCLE};
constexpr ObjType S_S[] = {ObjType::ET_SECTION, ObjType::ET_SECTION};
constexpr ObjType P_P_P[] = {ObjType::ET_POINT, ObjType::ET_POINT, ObjType::ET_POINT};

static constexpr ReqRule ReqRules[static_cast<std::size_t>(ReqType::COUNT)] = {
    {P_S, true, true},    // ET_POINTSECTIONDIST
    {P_S, false, false},  // ET_POINTONSECTION
    {P_P, true, true},    // ET_POINTPOINTDIST
    {P_P, false, false},  // ET_POINTONPOINT
    {S_C, true, true},    // ET_SECTIONCIRCLEDIST
    {S_C, false, false},  // ET_SECTIONONCIRCLE
    {S_C, false, false},  // ET_SECTIONINCIRCLE
    {S_S, true, false},   // ET_SECTIONSECTIONPARALLEL
    {S_S, true, false},   // ET_SECTIONSECTIONPERPENDICULAR
    {S_S, false, true},   // ET_SECTIONSECTIONANGLE
    {P_P_P, true, false}  // ET_ARCCENTERONPERPENDICULAR
};

inline std::string to_string(ObjType el) {
    switch (el) {
        case ObjType::ET_POINT:
            return "point";
        case ObjType::ET_SECTION:
            return "section";
        case ObjType::ET_CIRCLE:
            return "circle";
        case ObjType::ET_ARC:
            return "arc";
        case ObjType::ERROR:
            return "error";
    }
    return "None";
}

inline ObjType element_from_string(const std::string& s) {
    if (s == "point") return ObjType::ET_POINT;
    if (s == "section") return ObjType::ET_SECTION;
    if (s == "circle") return ObjType::ET_CIRCLE;
    if (s == "arc") return ObjType::ET_ARC;
    return ObjType::ERROR;
}

inline std::string to_string(ReqType requirement) {
    switch (requirement) {
        case ReqType::ET_POINTSECTIONDIST:
            return "pointsectiondist";
        case ReqType::ET_POINTONSECTION:
            return "pointonsection";
        case ReqType::ET_POINTPOINTDIST:
            return "pointpointdist";
        case ReqType::ET_POINTONPOINT:
            return "pointonpoint";
        case ReqType::ET_SECTIONCIRCLEDIST:
            return "sectioncircledist";
        case ReqType::ET_SECTIONONCIRCLE:
            return "sectiononcircle";
        case ReqType::ET_SECTIONINCIRCLE:
            return "sectionincircle";
        case ReqType::ET_SECTIONSECTIONPARALLEL:
            return "sectionsectionparallel";
        case ReqType::ET_SECTIONSECTIONPERPENDICULAR:
            return "sectionsectionperpendicular";
        case ReqType::ET_SECTIONSECTIONANGLE:
            return "sectionsectionangle";
        case ReqType::ET_ARCCENTERONPERPENDICULAR:
            return "arccenteronperpendicular";
        case ReqType::COUNT:
            return "count";
        case ReqType::ERROR:
            return "error";
    }
    return "None";
}

inline ReqType requirement_from_string(const std::string& s) {
    if (s == "pointsectiondist") return ReqType::ET_POINTSECTIONDIST;
    if (s == "pointonsection") return ReqType::ET_POINTONSECTION;
    if (s == "pointpointdist") return ReqType::ET_POINTPOINTDIST;
    if (s == "pointonpoint") return ReqType::ET_POINTONPOINT;
    if (s == "sectioncircledist") return ReqType::ET_SECTIONCIRCLEDIST;
    if (s == "sectiononcircle") return ReqType::ET_SECTIONONCIRCLE;
    if (s == "sectionincircle") return ReqType::ET_SECTIONINCIRCLE;
    if (s == "sectionsectionparallel") return ReqType::ET_SECTIONSECTIONPARALLEL;
    if (s == "sectionsectionperpendicular") return ReqType::ET_SECTIONSECTIONPERPENDICULAR;
    if (s == "sectionsectionangle") return ReqType::ET_SECTIONSECTIONANGLE;
    if (s == "arccenteronperpendicular") return ReqType::ET_ARCCENTERONPERPENDICULAR;
    return ReqType::ERROR;
}

#endif  // ! OURPAINT_HEADERS_ENUMS_H_
