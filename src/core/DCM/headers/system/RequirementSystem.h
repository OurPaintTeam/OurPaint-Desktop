#ifndef OURPAINTDCM_HEADERS_SYSTEM_REQUIREMENTSYSTEM_H
#define OURPAINTDCM_HEADERS_SYSTEM_REQUIREMENTSYSTEM_H

#include "RequirementFunctionSystem.h"
#include "RequirementFunctionFactory.h"
#include "GeometryStorage.h"
#include "Enums.h"
#include "IDGenerator.h"
#include "utils/RequirementDescriptor.h"

namespace OurPaintDCM::System {
/**
 * @brief High-level requirement system bound to GeometryStorage.
 *
 * Adds geometric constraints by IDs and uses RequirementFunctionSystem
 * directly for evaluation and diagnostics.
 *
 * ## Usage Example
 * @code
 * RequirementSystem system(&storage);
 *
 * // Method 1: Using unified addRequirement with descriptor
 * auto desc = RequirementDescriptor::pointPointDist(p1Id, p2Id, 100.0);
 * Utils::ID reqId = system.addRequirement(desc);
 *
 * // Method 2: Using specialized methods (still supported)
 * system.addPointOnLine(pointId, lineId);
 * @endcode
 */
class RequirementSystem : public RequirementFunctionSystem {
    Figures::GeometryStorage* _storage;
    Utils::IDGenerator _reqIdGen;

    struct RequirementEntry {
        Utils::ID id;                            ///< Unique ID of this requirement
        Utils::RequirementType type;             ///< Type of the requirement
        std::vector<Utils::ID> objectIds;        ///< IDs of objects involved
        std::optional<double> param;             ///< Optional parameter (distance, angle)
    };

    std::vector<RequirementEntry> _requirements;

public:
    /**
     * @brief Construct system bound to a GeometryStorage.
     * @param storage Pointer to the storage (must outlive this object).
     */
    explicit RequirementSystem(Figures::GeometryStorage* storage);


    /**
     * @brief Add a requirement using a descriptor (unified high-level interface).
     *
     * This is the recommended way to add requirements. It provides a single
     * entry point for all requirement types and validates the descriptor
     * before adding.
     *
     * @param descriptor The requirement descriptor containing type, object IDs, and optional parameter.
     * @return ID of the created requirement.
     * @throws std::invalid_argument if descriptor validation fails.
     * @throws std::runtime_error if object IDs are not found in storage.
     *
     * ## Example
     * @code
     * // Using factory method
     * auto desc = RequirementDescriptor::pointPointDist(p1Id, p2Id, 50.0);
     * Utils::ID reqId = system.addRequirement(desc);
     *
     * // Using direct construction
     * RequirementDescriptor desc2;
     * desc2.type = RequirementType::ET_LINELINEPARALLEL;
     * desc2.objectIds = {line1Id, line2Id};
     * system.addRequirement(desc2);
     * @endcode
     */
    Utils::ID addRequirement(const Utils::RequirementDescriptor& descriptor);

    /**
     * @brief Get all stored requirement entries (read-only).
     * @return Const reference to the vector of requirement entries.
     */
    const std::vector<RequirementEntry>& getRequirements() const noexcept { return _requirements; }

    /**
     * @brief Get requirement entry by ID.
     * @param reqId The requirement ID.
     * @return Pointer to the entry, or nullptr if not found.
     */
    const RequirementEntry* getRequirement(Utils::ID reqId) const noexcept;

    /**
     * @brief Check if requirement with given ID exists.
     * @param reqId The requirement ID to check.
     * @return true if requirement exists, false otherwise.
     */
    bool hasRequirement(Utils::ID reqId) const noexcept;

    /**
     * @brief Get requirement type by ID.
     * @param reqId The requirement ID.
     * @return Optional containing the requirement type, or nullopt if not found.
     */
    std::optional<Utils::RequirementType> getRequirementType(Utils::ID reqId) const noexcept;

    /**
     * @brief Get object IDs involved in requirement.
     * @param reqId The requirement ID.
     * @return Optional containing vector of object IDs, or nullopt if not found.
     */
    std::optional<std::vector<Utils::ID>> getRequirementObjectIds(Utils::ID reqId) const noexcept;

    /**
     * @brief Get parameter value of requirement (distance, angle, etc.).
     * @param reqId The requirement ID.
     * @return Optional containing the parameter value, or nullopt if requirement not found or has no parameter.
     */
    std::optional<double> getRequirementParam(Utils::ID reqId) const noexcept;

    /**
     * @brief Get count of all requirements in the system.
     * @return Number of requirements.
     */
    std::size_t getRequirementCount() const noexcept { return _requirements.size(); }

    // ==================== Specialized Methods (Legacy) ====================

    /// @brief Add point-line distance constraint by IDs.
    void addPointLineDist(Utils::ID pointId, Utils::ID lineId, double dist);

    /// @brief Add point-on-line constraint by IDs.
    void addPointOnLine(Utils::ID pointId, Utils::ID lineId);

    /// @brief Add point-point distance constraint by IDs.
    void addPointPointDist(Utils::ID p1Id, Utils::ID p2Id, double dist);

    /// @brief Add point-on-point constraint by IDs.
    void addPointOnPoint(Utils::ID p1Id, Utils::ID p2Id);

    /// @brief Add line-circle distance constraint by IDs.
    void addLineCircleDist(Utils::ID lineId, Utils::ID circleId, double dist);

    /// @brief Add line-on-circle constraint by IDs.
    void addLineOnCircle(Utils::ID lineId, Utils::ID circleId);

    /// @brief Add parallel-lines constraint by IDs.
    void addLineLineParallel(Utils::ID l1Id, Utils::ID l2Id);

    /// @brief Add perpendicular-lines constraint by IDs.
    void addLineLinePerpendicular(Utils::ID l1Id, Utils::ID l2Id);

    /// @brief Add angle-between-lines constraint by IDs.
    void addLineLineAngle(Utils::ID l1Id, Utils::ID l2Id, double angle);

    /// @brief Add vertical-line constraint by ID.
    void addVertical(Utils::ID lineId);

    /// @brief Add horizontal-line constraint by ID.
    void addHorizontal(Utils::ID lineId);

    /// @brief Add arc-center perpendicular-bisector constraint by ID.
    void addArcCenterOnPerpendicular(Utils::ID arcId);

    /**
     * @brief Clear all functions, variables, and requirement metadata.
     * 
     * Overrides base class clear() to also clear the requirement entries.
     */
    void clear();

    Figures::ObjectGraph buildDependencyGraph() const;
};

}

#endif
