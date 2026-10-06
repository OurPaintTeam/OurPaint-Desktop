#ifndef OURPAINT_CORE_SKETCH_H_
#define OURPAINT_CORE_SKETCH_H_

#include "SketchTypes.h"

namespace core::sketch {
namespace detail {
class ISketchBackend;
}

// Sketch is a self-contained parametric 2D geometric model operating in its own
// local 2D coordinate system. It provides the stable OurPaint API for creating,
// modifying, constraining, solving, manipulating, querying and diagnosing sketch
// geometry independently of the concrete sketch backend.
//
// Exactly one owned backend holds all current geometry/constraints. Sketch owns
// policy, identity allocation and backend lifetime only. It is neither a render
// scene nor UI/selection/history/document/3D placement state; a future SketchFeature
// can associate it with a support and placement transform.
// Calls on one instance must be externally serialized. Query results own their
// data and remain valid after edits/destruction. No borrowed solver pointers.
class Sketch {
public:
    static BackendKind defaultBackend();
    static std::vector<BackendKind> availableBackends();
    static Result<std::unique_ptr<Sketch>> create(BackendKind backend = defaultBackend());
    ~Sketch();
    Sketch(const Sketch&) = delete;
    Sketch& operator=(const Sketch&) = delete;

    BackendKind backendKind() const;
    Capabilities capabilities() const;
    // Common validation and exact backend support preflight without editing.
    Status supportsConstraint(const ConstraintDefinition& definition) const;

    Result<EntityId> addEntity(const SketchGeometry& geometry, bool construction = false);
    Result<EntityId> addPoint(Vec2 p) { return addEntity(Point2{p}); }
    Result<EntityId> addLine(Vec2 start, Vec2 end) { return addEntity(Line2{start, end}); }
    Result<EntityId> addCircle(Vec2 center, double radius) { return addEntity(Circle2{center, radius}); }
    Result<EntityId> addArc(Vec2 center, Vec2 start, Vec2 end) { return addEntity(Arc2{center, start, end}); }

    // Proposes an initial geometry guess of the same kind, preserving identity.
    // Hard fixed/coincident rules may project it. No numerical solve is requested.
    Status updateEntity(EntityId id, const SketchGeometry& geometry, std::optional<bool> construction = std::nullopt);
    Status updatePoint(EntityId id, Point2 point);
    Status updateLine(EntityId id, Line2 line);
    Status updateCircle(EntityId id, Circle2 circle);
    Status updateArc(EntityId id, Arc2 arc);
    Status updateCircleRadius(EntityId id, double radius);

    // Translates detached geometry proposals by a finite offset, preserving metadata.
    // Uses updateEntities validation/projection; no numerical solve or group dragging.
    Status moveEntities(std::span<const EntityId> ids, Vec2 offset);

    // Metadata-only edit: no geometry projection or numerical solve.
    Status setConstruction(EntityId id, bool construction);
    // Removes the entity and all constraints referencing any of its sub-elements.
    Status removeEntity(EntityId id);
    // All inputs are prevalidated before mutation; no rollback on BackendFailure.
    // Updates apply in input order and are initial guesses, not group dragging.
    // Empty batches succeed; created IDs follow input order and are never reused.
    Result<std::vector<EntityId>> addEntities(std::span<const EntityCreation> entities);
    Status updateEntities(std::span<const EntityUpdate> entities);
    Status removeEntities(std::span<const EntityId> ids);
    // Clears public and private model records in place, retaining backend and IDs.
    Status clear();
    Result<ConstraintId> addConstraint(const ConstraintDefinition& definition);
    // Prepare both coincidence groups before insertion, without numerical solving.
    // Midpoint moves free groups to the selected points' midpoint, or to a Fix
    // target when only one group is fixed. Fixed groups are not seeded. Other
    // equations/curve validity are enforced by a later explicit solve().
    // Preserve delegates to addConstraint. Preflight failures leave state intact;
    // BackendFailure can retain prepared coordinates and partial insertion.
    Result<ConstraintId> addCoincident(GeometryRef a, GeometryRef b, CoincidentPlacement placement = CoincidentPlacement::Midpoint);
    Status updateConstraint(ConstraintId id, const ConstraintDefinition& definition);
    Status removeConstraint(ConstraintId id);

    Result<SketchEntity> entity(EntityId id) const;
    Result<SketchConstraint> constraint(ConstraintId id) const;
    // Coarse queries cost one backend dispatch, not one dispatch per coordinate.
    Result<std::vector<SketchEntity>> entities() const;
    // Ascending public ID order. Backends scan metadata and copy matching values.
    Result<std::vector<SketchEntity>> entities(EntityKind kind) const;
    // points() contains standalone Point entities, not curve endpoints/centers.
    Result<std::vector<SketchEntity>> points() const { return entities(EntityKind::Point); }
    Result<std::vector<SketchEntity>> lines() const { return entities(EntityKind::Line); }
    Result<std::vector<SketchEntity>> circles() const { return entities(EntityKind::Circle); }
    Result<std::vector<SketchEntity>> arcs() const { return entities(EntityKind::Arc); }
    Result<Vec2> pointPosition(GeometryRef ref) const;
    Result<std::vector<PointElement>> pointElements() const;
    Result<std::vector<PointElement>> pointElements(PointElementScope scope) const;
    Result<std::vector<PointElement>> pointElements(EntityId entity) const;
    Result<std::vector<SketchConstraint>> constraints() const;
    // Public records only: O(1) totals, O(N) count by kind, no geometry copies.
    size_t entityCount() const;
    Result<size_t> entityCount(EntityKind kind) const;
    size_t constraintCount() const;

    // Mutations do not automatically solve. Several edits may precede one solve;
    // this is deferred solving, not an atomic transaction. InvalidArgument,
    // NotFound and Unsupported leave state unchanged. BackendFailure offers a
    // queryable model, not rollback. solve/drag do not change identities/topology;
    // failed convergence may retain an iterate, and is reported in diagnostics.
    Result<SolveDiagnostics> solve();
    Result<SolveDiagnostics> drag(const DragRequest& request);
    // Prevalidate all point targets, then solve them together. Empty input is a no-op.
    Result<SolveDiagnostics> dragPoints(std::span<const DragRequest> requests);

    Result<SketchSnapshot> snapshot() const;
    // Stages a fresh backend, then swaps only on complete success. No solve is
    // implicit. Recreation is limited to explicit state transfer. Import
    // retains this instance's ID high watermarks as well, preventing ID reuse.
    // Unsolved unequal arc radii can transfer; nonfinite/degenerate failed
    // iterates are rejected without replacing the current backend.
    Status replaceState(const SketchSnapshot& snapshot);
    Status switchBackend(BackendKind backend);

private:
    explicit Sketch(std::unique_ptr<detail::ISketchBackend> backend);
    Status validateConstraint(const ConstraintDefinition& definition) const;
    Status installState(const SketchSnapshot& snapshot, BackendKind backend);
    std::unique_ptr<detail::ISketchBackend> backend_;
    core::IDGenerator entityIds_;
    core::IDGenerator constraintIds_;
};
}  // namespace core::sketch
#endif
