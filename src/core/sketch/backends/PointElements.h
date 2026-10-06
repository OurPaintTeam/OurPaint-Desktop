#ifndef PARAMSCAD_CORE_SKETCH_BACKENDS_POINT_ELEMENTS_H_
#define PARAMSCAD_CORE_SKETCH_BACKENDS_POINT_ELEMENTS_H_

#include "../SketchTypes.h"

namespace core::sketch::detail {

inline bool pointLike(EntityKind kind, SubElement sub) {
    switch (kind) {
        case EntityKind::Point:
            return sub == SubElement::Whole;
        case EntityKind::Line:
            return sub == SubElement::Start || sub == SubElement::End;
        case EntityKind::Circle:
            return sub == SubElement::Center;
        case EntityKind::Arc:
            return sub == SubElement::Start || sub == SubElement::End || sub == SubElement::Center;
    }
    return false;
}

// Callers validate pointLike first. Keep extraction in Core, shared by queries.
inline Vec2 geometryPointPosition(const SketchGeometry& geometry, SubElement sub) {
    switch (entityKind(geometry)) {
        case EntityKind::Point:
            return std::get<Point2>(geometry).position;
        case EntityKind::Line: {
            const auto& line = std::get<Line2>(geometry);
            return sub == SubElement::Start ? line.start : line.end;
        }
        case EntityKind::Circle:
            return std::get<Circle2>(geometry).center;
        case EntityKind::Arc: {
            const auto& arc = std::get<Arc2>(geometry);
            return sub == SubElement::Start ? arc.start : sub == SubElement::End ? arc.end : arc.center;
        }
    }
    return {};
}

inline bool includesPoints(PointElementScope scope, EntityKind kind) {
    return scope == PointElementScope::All ||
           (kind == EntityKind::Point ? scope == PointElementScope::StandalonePoints : scope == PointElementScope::CurveSubElements);
}

inline void appendPointElements(std::vector<PointElement>& result, const SketchEntity& entity) {
    for (auto sub : {SubElement::Whole, SubElement::Start, SubElement::End, SubElement::Center}) {
        if (pointLike(entityKind(entity.geometry), sub)) {
            result.push_back({{entity.id, sub}, geometryPointPosition(entity.geometry, sub)});
        }
    }
}

}  // namespace core::sketch::detail
#endif
