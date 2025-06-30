#ifndef OURPAINT_HEADERS_ENUMS_H_
#define OURPAINT_HEADERS_ENUMS_H_
#include <string>
enum Element { ET_POINT, ET_SECTION, ET_CIRCLE, ET_ARC };

enum Requirement {
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
  ET_ARCCENTERONPERPENDICULAR

};

inline std::string to_string(Element el) {
  switch (el) {
    case ET_POINT:
      return "point";
    case ET_SECTION:
      return "section";
    case ET_CIRCLE:
      return "circle";
    case ET_ARC:
      return "arc";
  }
  return "None";
}
inline Element element_from_string(const std::string& s) {
  if (s == "point") return ET_POINT;
  if (s == "section") return ET_SECTION;
  if (s == "circle") return ET_CIRCLE;
  if (s == "arc") return ET_ARC;
}
inline std::string to_string(Requirement requirement) {
  switch (requirement) {
    case ET_POINTSECTIONDIST:
      return "pointsectiondist";
    case ET_POINTONSECTION:
      return "pointonsection";
    case ET_POINTPOINTDIST:
      return "pointpointdist";
    case ET_POINTONPOINT:
      return "pointonpoint";
    case ET_SECTIONCIRCLEDIST:
      return "sectioncircledist";
    case ET_SECTIONONCIRCLE:
      return "sectiononcircle";
    case ET_SECTIONINCIRCLE:
      return "sectionincircle";
    case ET_SECTIONSECTIONPARALLEL:
      return "sectionsectionparallel";
    case ET_SECTIONSECTIONPERPENDICULAR:
      return "sectionsectionperpendicular";
    case ET_SECTIONSECTIONANGLE:
      return "sectionsectionangle";
  }
  return "None";
}
inline Requirement requirement_from_string(const std::string& s) {
  if (s == "pointsectiondist") return ET_POINTSECTIONDIST;
  if (s == "pointonsection") return ET_POINTONSECTION;
  if (s == "pointpointdist") return ET_POINTPOINTDIST;
  if (s == "pointonpoint") return ET_POINTONPOINT;
  if (s == "sectioncircledist") return ET_SECTIONCIRCLEDIST;
  if (s == "sectiononcircle") return ET_SECTIONONCIRCLE;
  if (s == "sectionincircle") return ET_SECTIONINCIRCLE;
  if (s == "sectionsectionparallel") return ET_SECTIONSECTIONPARALLEL;
  if (s == "sectionsectionperpendicular") return ET_SECTIONSECTIONPERPENDICULAR;
  if (s == "sectionsectionangle") return ET_SECTIONSECTIONANGLE;
}

#endif  // ! OURPAINT_HEADERS_ENUMS_H_
