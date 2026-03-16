#ifndef OURPAINTDCM_HEADERS_UTILS_REQUIREMENTDESCRIPTOR_H
#define OURPAINTDCM_HEADERS_UTILS_REQUIREMENTDESCRIPTOR_H

#include "Enums.h"
#include "ID.h"
#include <vector>
#include <optional>
#include <stdexcept>

namespace OurPaintDCM::Utils {

/**
 * @brief High-level descriptor for geometric requirements.
 *
 * Provides a unified way to describe any geometric constraint
 * using object IDs and an optional parameter value.
 * This allows adding requirements through a single interface
 * instead of multiple specialized methods.
 *
 * ## Usage Example
 * @code
 * // Distance between two points
 * RequirementDescriptor desc;
 * desc.type = RequirementType::ET_POINTPOINTDIST;
 * desc.objectIds = {pointId1, pointId2};
 * desc.param = 50.0;  // distance value
 * system.addRequirement(desc);
 *
 * // Or using the builder-style methods
 * auto desc = RequirementDescriptor::pointPointDist(p1, p2, 50.0);
 * system.addRequirement(desc);
 * @endcode
 */
struct RequirementDescriptor {
    std::optional<ID> id;              ///< ID of the requirement (set after creation)
    RequirementType type;              ///< Type of the requirement
    std::vector<ID> objectIds;         ///< IDs of objects involved
    std::optional<double> param;       ///< Optional parameter (distance, angle, etc.)

    /// @brief Default constructor
    RequirementDescriptor() = default;

    /**
     * @brief Full constructor
     * @param t Requirement type
     * @param ids Vector of object IDs
     * @param p Optional parameter value
     */
    RequirementDescriptor(RequirementType t,
                          std::vector<ID> ids,
                          std::optional<double> p = std::nullopt)
        : type(t), objectIds(std::move(ids)), param(p) {}

    // ==================== Factory methods ====================

    /// @brief Create point-line distance descriptor
    static RequirementDescriptor pointLineDist(ID pointId, ID lineId, double dist) {
        return {RequirementType::ET_POINTLINEDIST, {pointId, lineId}, dist};
    }

    /// @brief Create point-on-line descriptor
    static RequirementDescriptor pointOnLine(ID pointId, ID lineId) {
        return {RequirementType::ET_POINTONLINE, {pointId, lineId}};
    }

    /// @brief Create point-point distance descriptor
    static RequirementDescriptor pointPointDist(ID p1Id, ID p2Id, double dist) {
        return {RequirementType::ET_POINTPOINTDIST, {p1Id, p2Id}, dist};
    }

    /// @brief Create point-on-point descriptor
    static RequirementDescriptor pointOnPoint(ID p1Id, ID p2Id) {
        return {RequirementType::ET_POINTONPOINT, {p1Id, p2Id}};
    }

    /// @brief Create line-circle distance descriptor
    static RequirementDescriptor lineCircleDist(ID lineId, ID circleId, double dist) {
        return {RequirementType::ET_LINECIRCLEDIST, {lineId, circleId}, dist};
    }

    /// @brief Create line-on-circle descriptor
    static RequirementDescriptor lineOnCircle(ID lineId, ID circleId) {
        return {RequirementType::ET_LINEONCIRCLE, {lineId, circleId}};
    }

    /// @brief Create line-in-circle descriptor
    static RequirementDescriptor lineInCircle(ID lineId, ID circleId) {
        return {RequirementType::ET_LINEINCIRCLE, {lineId, circleId}};
    }

    /// @brief Create parallel lines descriptor
    static RequirementDescriptor lineLineParallel(ID l1Id, ID l2Id) {
        return {RequirementType::ET_LINELINEPARALLEL, {l1Id, l2Id}};
    }

    /// @brief Create perpendicular lines descriptor
    static RequirementDescriptor lineLinePerpendicular(ID l1Id, ID l2Id) {
        return {RequirementType::ET_LINELINEPERPENDICULAR, {l1Id, l2Id}};
    }

    /// @brief Create angle between lines descriptor
    static RequirementDescriptor lineLineAngle(ID l1Id, ID l2Id, double angle) {
        return {RequirementType::ET_LINELINEANGLE, {l1Id, l2Id}, angle};
    }

    /// @brief Create vertical line descriptor
    static RequirementDescriptor vertical(ID lineId) {
        return {RequirementType::ET_VERTICAL, {lineId}};
    }

    /// @brief Create horizontal line descriptor
    static RequirementDescriptor horizontal(ID lineId) {
        return {RequirementType::ET_HORIZONTAL, {lineId}};
    }

    /// @brief Create arc center on perpendicular bisector descriptor
    static RequirementDescriptor arcCenterOnPerpendicular(ID arcId) {
        return {RequirementType::ET_ARCCENTERONPERPENDICULAR, {arcId}};
    }

    // ==================== Validation ====================

    /**
     * @brief Validate that descriptor has correct number of objects for its type.
     * @return true if valid
     * @throws std::invalid_argument with description of the problem
     */
    bool validate() const {
        switch (type) {
            case RequirementType::ET_POINTLINEDIST:
            case RequirementType::ET_POINTONLINE:
            case RequirementType::ET_POINTPOINTDIST:
            case RequirementType::ET_POINTONPOINT:
            case RequirementType::ET_LINECIRCLEDIST:
            case RequirementType::ET_LINEONCIRCLE:
            case RequirementType::ET_LINEINCIRCLE:
            case RequirementType::ET_LINELINEPARALLEL:
            case RequirementType::ET_LINELINEPERPENDICULAR:
            case RequirementType::ET_LINELINEANGLE:
                if (objectIds.size() != 2) {
                    throw std::invalid_argument("Requirement type requires exactly 2 object IDs");
                }
                break;
            case RequirementType::ET_VERTICAL:
            case RequirementType::ET_HORIZONTAL:
            case RequirementType::ET_ARCCENTERONPERPENDICULAR:
                if (objectIds.size() != 1) {
                    throw std::invalid_argument("Requirement type requires exactly 1 object ID");
                }
                break;
        }

        // Check that param is provided for types that need it
        switch (type) {
            case RequirementType::ET_POINTLINEDIST:
            case RequirementType::ET_POINTPOINTDIST:
            case RequirementType::ET_LINECIRCLEDIST:
            case RequirementType::ET_LINELINEANGLE:
                if (!param.has_value()) {
                    throw std::invalid_argument("Requirement type requires a parameter value");
                }
                break;
            default:
                break;
        }

        return true;
    }
};

} // namespace OurPaintDCM::Utils

#endif // OURPAINTDCM_HEADERS_UTILS_REQUIREMENTDESCRIPTOR_H
