#ifndef OURPAINT_CORE_SKETCH_POINT_ELEMENTS_H_
#define OURPAINT_CORE_SKETCH_POINT_ELEMENTS_H_

#include "SketchTypes.h"

namespace core::sketch::detail {

inline bool pointLike(const SketchGeometry& geometry, GeometryRef ref) {
    if (ref.sub == SubElement::Control) {
        const auto* curve = std::get_if<NurbsCurve2>(&geometry);
        return curve && ref.index < curve->controlPoints.size();
    }
    if (ref.index != 0) {
        return false;
    }
    switch (entityKind(geometry)) {
        case EntityKind::Point:
            return ref.sub == SubElement::Whole;
        case EntityKind::Line:
            return ref.sub == SubElement::Start || ref.sub == SubElement::End;
        case EntityKind::Circle:
            return ref.sub == SubElement::Center;
        case EntityKind::Arc:
            return ref.sub == SubElement::Start || ref.sub == SubElement::End || ref.sub == SubElement::Center;
        case EntityKind::Nurbs:
            return false;
    }
    return false;
}

inline bool validGeometryRef(const SketchGeometry& geometry, GeometryRef ref) {
    return (ref.sub == SubElement::Whole && ref.index == 0) || pointLike(geometry, ref);
}

// Callers validate pointLike first. Keep extraction in Core, shared by queries.
inline Vec2 geometryPointPosition(const SketchGeometry& geometry, GeometryRef ref) {
    switch (entityKind(geometry)) {
        case EntityKind::Point:
            return std::get<Point2>(geometry).position;
        case EntityKind::Line: {
            const auto& line = std::get<Line2>(geometry);
            return ref.sub == SubElement::Start ? line.start : line.end;
        }
        case EntityKind::Circle:
            return std::get<Circle2>(geometry).center;
        case EntityKind::Arc: {
            const auto& arc = std::get<Arc2>(geometry);
            return ref.sub == SubElement::Start ? arc.start : ref.sub == SubElement::End ? arc.end : arc.center;
        }
        case EntityKind::Nurbs:
            return std::get<NurbsCurve2>(geometry).controlPoints.at(ref.index).position;
    }
    return {};
}

inline bool includesPoints(PointElementScope scope, EntityKind kind) {
    return scope == PointElementScope::All ||
           (kind == EntityKind::Point ? scope == PointElementScope::StandalonePoints : scope == PointElementScope::CurveSubElements);
}

inline void appendPointElements(std::vector<PointElement>& result, const SketchEntity& entity) {
    if (const auto* curve = std::get_if<NurbsCurve2>(&entity.geometry)) {
        for (size_t i = 0; i < curve->controlPoints.size(); ++i) {
            result.push_back({{entity.id, SubElement::Control, i}, curve->controlPoints[i].position});
        }
        return;
    }
    for (auto sub : {SubElement::Whole, SubElement::Start, SubElement::End, SubElement::Center}) {
        const GeometryRef ref{entity.id, sub};
        if (pointLike(entity.geometry, ref)) {
            result.push_back({ref, geometryPointPosition(entity.geometry, ref)});
        }
    }
}

}  // namespace core::sketch::detail
#endif
