#ifndef OURPAINT_CORE_SKETCH_BACKENDS_ISKETCHBACKEND_H_
#define OURPAINT_CORE_SKETCH_BACKENDS_ISKETCHBACKEND_H_

#include "../SketchTypes.h"
#include "PointElements.h"

namespace core::sketch::detail {

// Internal implementation mechanism, never the API used by App/UI/Render. One
// stable-address instance exclusively owns the live model and solver resources.
// Adapters keep public-ID-to-handle maps/metadata, not duplicate coordinates.
// Sketch validates domain input; adapters validate exact support. Ordinary edits
// update affected records incrementally. Only explicit transfer recreates a backend.
class ISketchBackend {
public:
    virtual ~ISketchBackend() = default;
    virtual BackendKind kind() const = 0;
    virtual Capabilities capabilities() const = 0;
    virtual Status supportsConstraint(const ConstraintDefinition& definition) const = 0;

    // CRUD entity
    virtual Status addEntity(const SketchEntity& entity) = 0;
    virtual Status updateEntity(const SketchEntity& entity) = 0;
    virtual Status removeEntity(EntityId id) = 0;
    // Sketch prevalidates the complete batch. Default loops are safe for adds
    // and removals with exclusive owned sub-elements. No failure rollback.
    virtual Status addEntities(std::span<const SketchEntity> entities) {
        for (const auto& entity : entities) {
            auto added = addEntity(entity);
            if (!added) {
                return added;
            }
        }
        return Status::success();
    }
    virtual Status updateEntities(std::span<const SketchEntity> entities) = 0;
    // Prevalidated, finite point guesses for constraint preparation. No numerical
    // solve or metadata edits; intermediate curves can be noncircular/degenerate.
    virtual Status updatePointGuesses(std::span<const PointElement> points) = 0;
    virtual Status removeEntities(std::span<const EntityId> ids) {
        for (auto id : ids) {
            auto removed = removeEntity(id);
            if (!removed) {
                return removed;
            }
        }
        return Status::success();
    }
    virtual Status setConstruction(EntityId id, bool construction) = 0;
    virtual Status clear() = 0;
    virtual Result<SketchEntity> entity(EntityId id) const = 0;
    virtual Result<std::vector<SketchEntity>> entities(std::optional<EntityKind> kind = std::nullopt) const = 0;
    virtual Result<std::vector<PointElement>> pointElements(PointElementScope scope) const = 0;
    virtual size_t entityCount() const = 0;
    virtual size_t entityCount(EntityKind kind) const = 0;
    virtual size_t constraintCount() const = 0;

    // CRUD constraint
    virtual Status addConstraint(const SketchConstraint& constraint) = 0;
    virtual Status updateConstraint(const SketchConstraint& constraint) = 0;
    virtual Status removeConstraint(ConstraintId id) = 0;
    virtual Result<SketchConstraint> constraint(ConstraintId id) const = 0;
    virtual Result<std::vector<SketchConstraint>> constraints() const = 0;

    // Solver
    virtual Result<SolveDiagnostics> solve() = 0;
    virtual Result<SolveDiagnostics> dragPoints(std::span<const DragRequest> requests) = 0;
};

Result<std::unique_ptr<ISketchBackend>> makeDcmSketchBackend();
Result<std::unique_ptr<ISketchBackend>> makeSolveSpaceSketchBackend();

}  // namespace core::sketch::detail

#endif // ! OURPAINT_CORE_SKETCH_BACKENDS_ISKETCHBACKEND_H_
