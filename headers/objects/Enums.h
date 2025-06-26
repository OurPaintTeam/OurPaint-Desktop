#ifndef OURPAINT_HEADERS_ENUMS_H_
#define OURPAINT_HEADERS_ENUMS_H_

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
     /* ET_ARCCENTERONPERPENDICULAR    */ {ObjType::ET_POINT,     ObjType::ET_POINT, true, false}
};

#endif // ! OURPAINT_HEADERS_ENUMS_H_
