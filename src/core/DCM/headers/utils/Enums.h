#ifndef HEADERS_UTILS_ENUM_H
#define HEADERS_UTILS_ENUM_H
#include <cstdint>
#include <string_view>

namespace OurPaintDCM::Utils {
/**
 * @brief Enum which illustrates every requirement type
 * Each value describes a geometric constraint that can be applied
 * to points, sections (lines), circles, or arcs in the system.
 * These constraints are used by solvers to maintain relationships
 * between geometric objects.
 */
enum class RequirementType: uint8_t {
    /// Fixed distance between point and section.
    ET_POINTLINEDIST,

    /// Point must lie on a section (distance == 0).
    ET_POINTONLINE,

    /// Fixed distance between two points.
    ET_POINTPOINTDIST,

    /// Two points must coincide(distance == 0).
    ET_POINTONPOINT,

    /// Fixed distance between section and circle.
    ET_LINECIRCLEDIST,

    /// Section endpoints must be on a circle.
    ET_LINEONCIRCLE,

    /// Section lies entirely inside a circle.
    ET_LINEINCIRCLE,

    /// Two sections must be parallel.
    ET_LINELINEPARALLEL,

    /// Two sections must be perpendicular.
    ET_LINELINEPERPENDICULAR,

    /// Fixed angle between two sections.
    ET_LINELINEANGLE,

    /// Section must be vertical (along Y-axis).
    ET_VERTICAL,

    /// Section must be horizontal (along X-axis).
    ET_HORIZONTAL,

    /// Default constraints for every arc, for making 3 points working like a arc
    ET_ARCCENTERONPERPENDICULAR
};

/**
 * @brief Enum for all geometric figure types.
 */
enum class FigureType : uint8_t {
    /// 2D point object.
    ET_POINT2D,

    /// Line or line segment.
    ET_LINE,

    /// Circle object.
    ET_CIRCLE,

    /// Circular arc object.
    ET_ARC
};
/**
 * @brief Different modes of constraint solving.
 */
enum class SolveMode {
    GLOBAL, ///< Solve the entire system
    LOCAL,  ///< Solve only affected subgraphs/subsystems
    DRAG    ///< Interactive drag mode (tracking free DoF)
};
enum class SystemStatus {
    WELL_CONSTRAINED,
    UNDER_CONSTRAINED,
    OVER_CONSTRAINED,
    SINGULAR_SYSTEM,
    EMPTY,
    UNKNOWN
};

}
#endif //HEADERS_UTILS_ENUM_H