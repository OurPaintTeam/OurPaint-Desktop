#ifndef OURPAINT_HEADERS_ENUMS_H_
#define OURPAINT_HEADERS_ENUMS_H_

#include <string>
#include <cstdint>

enum class ObjType : uint8_t {
    ET_POINT, ET_SECTION, ET_CIRCLE, ET_ARC
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
    ET_HORIZONTAL,
    ET_VERTICAL,
    COUNT
};

struct ReqRule {
    ObjType first;
    ObjType second;
    bool symmetric;
    bool needsParam;
};

static constexpr ReqRule ReqRules[static_cast<std::size_t>(ReqType::COUNT)] = {
     /* ET_POINTSECTIONDIST            */ {ObjType::ET_POINT,   ObjType::ET_SECTION, true , true },
     /* ET_POINTONSECTION              */ {ObjType::ET_POINT,   ObjType::ET_SECTION, false, false},
     /* ET_POINTPOINTDIST              */ {ObjType::ET_POINT,   ObjType::ET_POINT,   true , true },
     /* ET_POINTONPOINT                */ {ObjType::ET_POINT,   ObjType::ET_POINT,   false, false},
     /* ET_SECTIONCIRCLEDIST           */ {ObjType::ET_SECTION, ObjType::ET_CIRCLE,  true , true },
     /* ET_SECTIONONCIRCLE             */ {ObjType::ET_SECTION, ObjType::ET_CIRCLE,  false, false},
     /* ET_SECTIONINCIRCLE             */ {ObjType::ET_SECTION, ObjType::ET_CIRCLE,  false, false},
     /* ET_SECTIONSECTIONPARALLEL      */ {ObjType::ET_SECTION, ObjType::ET_SECTION, true , false},
     /* ET_SECTIONSECTIONPERPENDICULAR */ {ObjType::ET_SECTION,ObjType::ET_SECTION, true , false},
     /* ET_SECTIONSECTIONANGLE         */ {ObjType::ET_SECTION, ObjType::ET_SECTION, false, true },
     /* ET_ARCCENTERONPERPENDICULAR    */ {ObjType::ET_POINT,     ObjType::ET_POINT, true, false},
     /* ET_HORIZONTAL                  */ {ObjType::ET_SECTION, ObjType::ET_SECTION, false, false},
     /* ET_VERTICAL                    */ {ObjType::ET_SECTION, ObjType::ET_SECTION, false, false}
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
  }
  return "None";
}
inline ObjType element_from_string(const std::string& s) {
  if (s == "point") return ObjType::ET_POINT;
  if (s == "section") return ObjType::ET_SECTION;
  if (s == "circle") return ObjType::ET_CIRCLE;
  if (s == "arc") return ObjType::ET_ARC;
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
}

#endif  // ! OURPAINT_HEADERS_ENUMS_H_
