#ifndef OURPAINT_HEADERS_ENUMS_H_
#define OURPAINT_HEADERS_ENUMS_H_

#include <cstdint>
#include <span>
#include <string>

namespace core {
enum class ObjType : std::uint8_t {
    ET_POINT,
    ET_LINE,
    ET_CIRCLE,
    ET_ARC,
    ET_CUBIC_BEZIER,
    ERROR
};

enum class ReqType : uint8_t {
    ET_POINTLINEDIST,
    ET_POINTONLINE,
    ET_POINTPOINTDIST,
    ET_POINTONPOINT,
    ET_LINECIRCLEDIST,
    ET_LINEONCIRCLE,
    ET_LINEINCIRCLE,
    ET_LINELINEPARALLEL,
    ET_LINELINEPERPENDICULAR,
    ET_LINELINEANGLE,
    ET_ARCCENTERONPERPENDICULAR,
    ET_FIXPOINT,
    ET_FIXLINE,
    ET_FIXCIRCLE,
    ET_HORIZONTAL,
    ET_VERTICAL,
    COUNT,
    ERROR
};

struct ReqRule {
    std::span<const ObjType> types;
    bool symmetric;
    bool needsParam;
};

constexpr ObjType P_S[] = {ObjType::ET_POINT, ObjType::ET_LINE};
constexpr ObjType P_P[] = {ObjType::ET_POINT, ObjType::ET_POINT};
constexpr ObjType S_C[] = {ObjType::ET_LINE, ObjType::ET_CIRCLE};
constexpr ObjType S_S[] = {ObjType::ET_LINE, ObjType::ET_LINE};
constexpr ObjType P_P_P[] = {ObjType::ET_POINT, ObjType::ET_POINT, ObjType::ET_POINT};

static constexpr ReqRule ReqRules[static_cast<std::size_t>(ReqType::COUNT)] = {
    {P_S, true, true},    // ET_POINTLINEDIST
    {P_S, false, false},  // ET_POINTONLINE
    {P_P, true, true},    // ET_POINTPOINTDIST
    {P_P, false, false},  // ET_POINTONPOINT
    {S_C, true, true},    // ET_LINECIRCLEDIST
    {S_C, false, false},  // ET_LINEONCIRCLE
    {S_C, false, false},  // ET_LINEINCIRCLE
    {S_S, true, false},   // ET_LINELINEPARALLEL
    {S_S, true, false},   // ET_LINELINEPERPENDICULAR
    {S_S, false, true},   // ET_LINELINEANGLE
    {P_P_P, true, false}  // ET_ARCCENTERONPERPENDICULAR
};

inline std::string to_string(ObjType el) {
    switch (el) {
        case ObjType::ET_POINT:
            return "point";
        case ObjType::ET_LINE:
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
    if (s == "section") return ObjType::ET_LINE;
    if (s == "circle") return ObjType::ET_CIRCLE;
    if (s == "arc") return ObjType::ET_ARC;
    return ObjType::ERROR;
}

inline std::string to_string(ReqType requirement) {
    switch (requirement) {
        case ReqType::ET_POINTLINEDIST:
            return "pointsectiondist";
        case ReqType::ET_POINTONLINE:
            return "pointonsection";
        case ReqType::ET_POINTPOINTDIST:
            return "pointpointdist";
        case ReqType::ET_POINTONPOINT:
            return "pointonpoint";
        case ReqType::ET_LINECIRCLEDIST:
            return "sectioncircledist";
        case ReqType::ET_LINEONCIRCLE:
            return "sectiononcircle";
        case ReqType::ET_LINEINCIRCLE:
            return "sectionincircle";
        case ReqType::ET_LINELINEPARALLEL:
            return "sectionsectionparallel";
        case ReqType::ET_LINELINEPERPENDICULAR:
            return "sectionsectionperpendicular";
        case ReqType::ET_LINELINEANGLE:
            return "sectionsectionangle";
        case ReqType::ET_ARCCENTERONPERPENDICULAR:
            return "arccenteronperpendicular";
        case ReqType::COUNT:
            return "count";
        case ReqType::ERROR:
            return "error";
        case ReqType::ET_FIXPOINT:
            break;
        case ReqType::ET_FIXLINE:
            break;
        case ReqType::ET_FIXCIRCLE:
            break;
    }
    return "None";
}

inline ReqType requirement_from_string(const std::string& s) {
    if (s == "pointsectiondist") return ReqType::ET_POINTLINEDIST;
    if (s == "pointonsection") return ReqType::ET_POINTONLINE;
    if (s == "pointpointdist") return ReqType::ET_POINTPOINTDIST;
    if (s == "pointonpoint") return ReqType::ET_POINTONPOINT;
    if (s == "sectioncircledist") return ReqType::ET_LINECIRCLEDIST;
    if (s == "sectiononcircle") return ReqType::ET_LINEONCIRCLE;
    if (s == "sectionincircle") return ReqType::ET_LINEINCIRCLE;
    if (s == "sectionsectionparallel") return ReqType::ET_LINELINEPARALLEL;
    if (s == "sectionsectionperpendicular") return ReqType::ET_LINELINEPERPENDICULAR;
    if (s == "sectionsectionangle") return ReqType::ET_LINELINEANGLE;
    if (s == "arccenteronperpendicular") return ReqType::ET_ARCCENTERONPERPENDICULAR;
    return ReqType::ERROR;
}
}

#endif  // ! OURPAINT_HEADERS_ENUMS_H_
