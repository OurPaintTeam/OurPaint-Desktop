#ifndef OURPAINTDCM_HEADERS_DCMMANAGER_H
#define OURPAINTDCM_HEADERS_DCMMANAGER_H

#include "GeometryStorage.h"
#include "RequirementSystem.h"
#include "RequirementDescriptor.h"
#include "FigureDescriptor.h"
#include "Graph.h"
#include "Function.h"
#include "LSMFORLMTask.h"
#include "LSMTask.h"
#include "LMWithSparse.h"
#include "GradientOptimizer.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <memory>
#include <cmath>

namespace OurPaintDCM {

using ComponentID = std::size_t;
using ComponentGraph = Graph<Utils::ID, Utils::ID, UndirectedPolicy, WeightedPolicy>;

/**
 * @brief Main manager class for the DCM (Dynamic Constraint Manager) system.
 *
 * Provides unified CRUD operations for geometric figures and requirements.
 * Manages connected components of the constraint graph.
 */
class DCMManager {
public:
    /**
     * @brief Default constructor.
     */
    DCMManager();

    ~DCMManager() = default;

    DCMManager(const DCMManager&) = delete;
    DCMManager& operator=(const DCMManager&) = delete;
    DCMManager(DCMManager&&) noexcept = default;
    DCMManager& operator=(DCMManager&&) noexcept = default;

    /**
     * @brief Add a new geometric figure using descriptor.
     * @param descriptor FigureDescriptor containing figure data.
     * @return ID of the created figure.
     * @throws std::invalid_argument if descriptor validation fails.
     */
    Utils::ID addFigure(const Utils::FigureDescriptor& descriptor);

    /**
     * @brief Remove a figure by ID.
     * @param figureId ID of the figure to remove.
     * @param forceCascade If true, removes dependent figures and requirements.
     * @throws std::runtime_error if figure not found or has dependencies.
     */
    void removeFigure(Utils::ID figureId, bool forceCascade = false);

    /**
     * @brief Update point coordinates.
     * @param descriptor PointUpdateDescriptor with new values.
     * @throws std::runtime_error if point not found.
     */
    void updatePoint(const Utils::PointUpdateDescriptor& descriptor);

    /**
     * @brief Update circle radius.
     * @param descriptor CircleUpdateDescriptor with new radius.
     * @throws std::runtime_error if circle not found.
     */
    void updateCircle(const Utils::CircleUpdateDescriptor& descriptor);

    /**
     * @brief Get figure descriptor with current state.
     * @param figureId ID of the figure.
     * @return Optional containing FigureDescriptor or nullopt if not found.
     */
    std::optional<Utils::FigureDescriptor> getFigure(Utils::ID figureId) const;

    /**
     * @brief Check if figure exists.
     * @param figureId ID to check.
     * @return true if figure exists.
     */
    bool hasFigure(Utils::ID figureId) const noexcept;

    /**
     * @brief Get all figure descriptors.
     * @return Vector of FigureDescriptors for all figures.
     */
    std::vector<Utils::FigureDescriptor> getAllFigures() const;

    /**
     * @brief Add a new requirement using descriptor.
     * @param descriptor RequirementDescriptor containing requirement data.
     * @return ID of the created requirement.
     * @throws std::invalid_argument if descriptor validation fails.
     */
    Utils::ID addRequirement(const Utils::RequirementDescriptor& descriptor);

    /**
     * @brief Remove a requirement by ID.
     * @param reqId ID of the requirement to remove.
     * @throws std::runtime_error if requirement not found.
     */
    void removeRequirement(Utils::ID reqId);

    /**
     * @brief Update requirement parameter value.
     * @param reqId ID of the requirement.
     * @param newParam New parameter value.
     * @throws std::runtime_error if requirement not found or has no parameter.
     */
    void updateRequirementParam(Utils::ID reqId, double newParam);

    /**
     * @brief Get requirement descriptor by ID.
     * @param reqId ID of the requirement.
     * @return Optional containing RequirementDescriptor or nullopt if not found.
     */
    std::optional<Utils::RequirementDescriptor> getRequirement(Utils::ID reqId) const noexcept;

    /**
     * @brief Check if requirement exists.
     * @param reqId ID to check.
     * @return true if requirement exists.
     */
    bool hasRequirement(Utils::ID reqId) const noexcept;

    /**
     * @brief Get all requirement descriptors.
     * @return Vector of RequirementDescriptors for all requirements.
     */
    std::vector<Utils::RequirementDescriptor> getAllRequirements() const;

    /**
     * @brief Get total number of connected components.
     * @return Number of connected components.
     */
    std::size_t getComponentCount() const noexcept;

    /**
     * @brief Get component ID for a given figure.
     * @param figureId ID of the figure.
     * @return Optional containing component ID or nullopt if figure not found.
     */
    std::optional<ComponentID> getComponentForFigure(Utils::ID figureId) const noexcept;

    /**
     * @brief Get all figure IDs in a component.
     * @param componentId ID of the component.
     * @return Vector of figure IDs in the component.
     */
    std::vector<Utils::ID> getFiguresInComponent(ComponentID componentId) const;

    /**
     * @brief Get all requirement IDs in a component.
     * @param componentId ID of the component.
     * @return Vector of requirement IDs affecting the component.
     */
    std::vector<Utils::ID> getRequirementsInComponent(ComponentID componentId) const;

    /**
     * @brief Get all connected components.
     * @return Vector of vectors containing figure IDs for each component.
     */
    std::vector<std::vector<Utils::ID>> getAllComponents() const;

    /**
     * @brief Get read-only access to GeometryStorage.
     * @return Const reference to internal GeometryStorage.
     */
    const Figures::GeometryStorage& getStorage() const noexcept;

    /**
     * @brief Get read-only access to RequirementSystem.
     * @return Const reference to internal RequirementSystem.
     */
    const System::RequirementSystem& getRequirementSystem() const noexcept;

    /**
     * @brief Get mutable access to GeometryStorage.
     * @return Reference to internal GeometryStorage.
     */
    Figures::GeometryStorage& storage() noexcept;

    /**
     * @brief Get mutable access to RequirementSystem.
     * @return Reference to internal RequirementSystem.
     */
    System::RequirementSystem& requirementSystem() noexcept;

    /**
     * @brief Get total figure count.
     * @return Number of figures.
     */
    std::size_t figureCount() const noexcept;

    /**
     * @brief Get total requirement count.
     * @return Number of requirements.
     */
    std::size_t requirementCount() const noexcept;

    /**
     * @brief Clear all figures, requirements, and components.
     */
    void clear();

    /**
     * @brief Switch current solving mode.
     * @param mode New solving mode.
     */
    void setSolveMode(Utils::SolveMode mode) noexcept;

    /// @brief Get current solving mode.
    Utils::SolveMode getSolveMode() const noexcept;

    /**
     * @brief Solve the constraint system according to the current mode.
     *
     * GLOBAL — solves all requirements at once (Levenberg-Marquardt).
     * LOCAL  — solves only the specified component (Levenberg-Marquardt).
     * DRAG   — lightweight gradient descent, intended to be called from updatePoint/updateCircle.
     *
     * @param componentId Component to solve (used only in LOCAL mode).
     * @return true if the solver converged.
     */
    bool solve(std::optional<ComponentID> componentId = std::nullopt);

private:
    Figures::GeometryStorage _storage;
    System::RequirementSystem _reqSystem;

    std::unordered_map<Utils::ID, Utils::FigureDescriptor> _figureRecords;
    std::unordered_map<Utils::ID, Utils::RequirementDescriptor> _requirementRecords;
    std::unordered_map<Utils::ID, ComponentID> _figureToComponent;
    std::vector<std::unordered_set<Utils::ID>> _components;
    ComponentID _nextComponentId = 0;
    std::size_t _activeComponentCount = 0;
    Utils::SolveMode _solveMode = Utils::SolveMode::GLOBAL;

    std::unique_ptr<System::RequirementSystem> buildSubsystem(ComponentID componentId) const;

    void rebuildRequirementSystem();
    void rebuildComponents();
    void mergeComponents(const std::vector<Utils::ID>& figureIds);
    void splitComponentsAfterRemoval();
    ComponentID createNewComponent();
    void addFigureToComponent(Utils::ID figureId, ComponentID componentId);
    void removeFigureFromComponent(Utils::ID figureId);
    std::vector<Utils::ID> getRequirementsForFigure(Utils::ID figureId) const;
};

} // namespace OurPaintDCM

#endif // OURPAINTDCM_HEADERS_DCMMANAGER_H
