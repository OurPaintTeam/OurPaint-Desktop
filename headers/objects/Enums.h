#ifndef OURPAINT_HEADERS_ENUMS_H_
#define OURPAINT_HEADERS_ENUMS_H_
#include <string>
enum Element {
    ET_POINT, ET_SECTION, ET_CIRCLE, ET_ARC
};

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


#endif // ! OURPAINT_HEADERS_ENUMS_H_
