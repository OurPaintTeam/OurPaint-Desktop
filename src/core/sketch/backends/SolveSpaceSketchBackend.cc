#include <slvs.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <mutex>
#include <numbers>
#include <type_traits>

#include "ISketchBackend.h"

namespace core::sketch::detail {
namespace {

constexpr Slvs_hGroup kFixedGroup = 1;
constexpr Slvs_hGroup kSketchGroup = 2;
constexpr Slvs_hEntity kNormal = 2;
constexpr Slvs_hEntity kWorkplane = 3;

// libslvs has process-global scratch even for its caller-owned-array API. Only
// the solve call touches that scratch; separate Sketch instances otherwise own
// completely independent models. All libslvs use must respect this serialization.
std::mutex solveSpaceMutex;

template <class T>
class NativeRecords {
public:
    void add(T value) {
        indices_.emplace(value.h, values_.size());
        values_.push_back(value);
    }
    T& get(uint32_t handle) { return values_.at(indices_.at(handle)); }
    const T& get(uint32_t handle) const { return values_.at(indices_.at(handle)); }
    void remove(uint32_t handle) {
        const auto index = indices_.at(handle);
        if (index + 1 != values_.size()) {
            values_[index] = values_.back();
            indices_.at(values_[index].h) = index;
        }
        values_.pop_back();
        indices_.erase(handle);
    }
    std::vector<T>& values() { return values_; }

private:
    std::vector<T> values_;
    std::map<uint32_t, size_t> indices_;
};

// Persistent Slvs records are the sole coordinate source. These records are
// edited incrementally; public entities only retain identity/topology metadata.
class SolveSpaceSketchBackend final : public ISketchBackend {
public:
    SolveSpaceSketchBackend() {
        for (int i = 0; i != 7; ++i) addParam(i == 3 ? 1.0 : 0.0, kFixedGroup);
        nativeEntities_.add(Slvs_MakePoint3d(nextEntity_++, kFixedGroup, 1, 2, 3));
        nativeEntities_.add(Slvs_MakeNormal3d(nextEntity_++, kFixedGroup, 4, 5, 6, 7));
        nativeEntities_.add(Slvs_MakeWorkplane(nextEntity_++, kFixedGroup, 1, kNormal));
    }

    BackendKind kind() const override { return BackendKind::SolveSpace; }

    Capabilities capabilities() const override {
        Capabilities result;
        result.entities.fill(true);
        result.constraints.fill(true);
        result.constraints[static_cast<size_t>(ConstraintType::Tangent)] = false;
        result.updateEntities = result.removeEntities = result.updateConstraints = result.removeConstraints = true;
        result.solve = result.drag = result.degreesOfFreedom = result.conflictingConstraints = result.redundantConstraints = true;
        return result;
    }

    Status supportsConstraint(const ConstraintDefinition& definition) const override {
        if (!capabilities().supports(definition.type)) {
            return Status::failure(ErrorCode::Unsupported, "SolveSpace does not support whole-curve external tangency through this adapter.");
        }
        return Status::success();
    }

    Status addEntity(const SketchEntity& entity) override {
        if (entities_.contains(entity.id)) return Status::failure(ErrorCode::InvalidArgument, "Duplicate entity ID.");
        if (!canAllocate(6, 4, 0)) return exhausted();
        EntityRecord record{entityKind(entity.geometry), entity.construction};
        switch (record.kind) {
            case EntityKind::Point: {
                record.handle = addPoint(std::get<Point2>(entity.geometry).position, record.owned);
                break;
            }
            case EntityKind::Line: {
                const auto& line = std::get<Line2>(entity.geometry);
                const auto start = addPoint(line.start, record.owned);
                const auto end = addPoint(line.end, record.owned);
                record.handle = nextEntity_++;
                nativeEntities_.add(Slvs_MakeLineSegment(record.handle, kSketchGroup, kWorkplane, start, end));
                record.owned.entities.push_back(record.handle);
                break;
            }
            case EntityKind::Circle: {
                const auto& circle = std::get<Circle2>(entity.geometry);
                const auto center = addPoint(circle.center, record.owned);
                const auto radius = addParam(circle.radius);
                record.owned.params.push_back(radius);
                const auto distance = nextEntity_++;
                nativeEntities_.add(Slvs_MakeDistance(distance, kSketchGroup, kWorkplane, radius));
                record.owned.entities.push_back(distance);
                record.handle = nextEntity_++;
                nativeEntities_.add(Slvs_MakeCircle(record.handle, kSketchGroup, kWorkplane, center, kNormal, distance));
                record.owned.entities.push_back(record.handle);
                break;
            }
            case EntityKind::Arc: {
                const auto& arc = std::get<Arc2>(entity.geometry);
                const auto center = addPoint(arc.center, record.owned);
                const auto start = addPoint(arc.start, record.owned);
                const auto end = addPoint(arc.end, record.owned);
                record.handle = nextEntity_++;
                nativeEntities_.add(Slvs_MakeArcOfCircle(record.handle, kSketchGroup, kWorkplane, kNormal, center, start, end));
                record.owned.entities.push_back(record.handle);
                break;
            }
        }
        entities_.emplace(entity.id, std::move(record));
        return Status::success();
    }

    Status updateEntity(const SketchEntity& entity) override {
        auto found = entities_.find(entity.id);
        if (found == entities_.end()) return Status::failure(ErrorCode::NotFound, "Entity not found.");
        auto& record = found->second;
        if (record.kind != entityKind(entity.geometry)) return Status::failure(ErrorCode::InvalidArgument, "Entity kind cannot change.");
        const auto& native = nativeEntities_.get(record.handle);
        switch (record.kind) {
            case EntityKind::Point:
                setPoint(record.handle, std::get<Point2>(entity.geometry).position);
                break;
            case EntityKind::Line: {
                const auto& line = std::get<Line2>(entity.geometry);
                setPoint(native.point[0], line.start);
                setPoint(native.point[1], line.end);
                break;
            }
            case EntityKind::Circle: {
                const auto& circle = std::get<Circle2>(entity.geometry);
                setPoint(native.point[0], circle.center);
                params_.get(nativeEntities_.get(native.distance).param[0]).val = circle.radius;
                break;
            }
            case EntityKind::Arc: {
                const auto& arc = std::get<Arc2>(entity.geometry);
                setPoint(native.point[0], arc.center);
                setPoint(native.point[1], arc.start);
                setPoint(native.point[2], arc.end);
                break;
            }
        }
        record.construction = entity.construction;
        return Status::success();
    }

    Status updateEntities(std::span<const SketchEntity> entities) override {
        // Each public entity exclusively owns its parameters. Coincident and Fix
        // remain equations until an explicit solve, just as for single updates.
        for (const auto& entity : entities) {
            auto updated = updateEntity(entity);
            if (!updated) {
                return updated;
            }
        }
        return Status::success();
    }

    Status updatePointGuesses(std::span<const PointElement> points) override {
        for (const auto& point : points) {
            setPoint(pointHandle(point.ref), point.position);
        }
        return Status::success();
    }

    Status setConstruction(EntityId id, bool construction) override {
        const auto found = entities_.find(id);
        if (found == entities_.end()) {
            return Status::failure(ErrorCode::NotFound, "Entity not found.");
        }
        found->second.construction = construction;
        return Status::success();
    }

    Status clear() override {
        // Retain the private workplane/normal and handle watermarks. Remove all
        // public-owned points, parameters, anchors and constraint equations.
        while (!constraints_.empty()) {
            auto removed = removeConstraint(constraints_.begin()->first);
            if (!removed) {
                return removed;
            }
        }
        for (const auto& [id, record] : entities_) {
            removeOwned(record.owned);
        }
        entities_.clear();
        return Status::success();
    }

    Status removeEntity(EntityId id) override {
        const auto found = entities_.find(id);
        if (found == entities_.end()) return Status::failure(ErrorCode::NotFound, "Entity not found.");
        for (auto it = constraints_.begin(); it != constraints_.end();) {
            const auto& refs = it->second.definition.refs;
            if (std::any_of(refs.begin(), refs.end(), [id](GeometryRef ref) { return ref.entity == id; })) {
                auto remove = it++;
                removeConstraint(remove->first);
            } else {
                ++it;
            }
        }
        removeOwned(found->second.owned);
        entities_.erase(found);
        return Status::success();
    }

    Status addConstraint(const SketchConstraint& constraint) override {
        if (constraints_.contains(constraint.id)) return Status::failure(ErrorCode::InvalidArgument, "Duplicate constraint ID.");
        auto support = supportsConstraint(constraint.definition);
        if (!support) return support;
        if (!canAllocate(2, 1, 1)) return exhausted();
        ConstraintRecord record;
        record.definition = constraint.definition;
        record.handle = nextConstraint_++;
        nativeConstraints_.add(makeConstraint(record));
        constraints_.emplace(constraint.id, std::move(record));
        return Status::success();
    }

    Status updateConstraint(const SketchConstraint& constraint) override {
        const auto found = constraints_.find(constraint.id);
        if (found == constraints_.end()) return Status::failure(ErrorCode::NotFound, "Constraint not found.");
        auto support = supportsConstraint(constraint.definition);
        if (!support) return support;
        if (!canAllocate(2, 1, 0)) return exhausted();
        ConstraintRecord replacement;
        replacement.definition = constraint.definition;
        replacement.handle = found->second.handle;
        const auto native = makeConstraint(replacement);
        nativeConstraints_.get(replacement.handle) = native;
        removeOwned(found->second.owned);
        found->second = std::move(replacement);
        return Status::success();
    }

    Status removeConstraint(ConstraintId id) override {
        const auto found = constraints_.find(id);
        if (found == constraints_.end()) return Status::failure(ErrorCode::NotFound, "Constraint not found.");
        nativeConstraints_.remove(found->second.handle);
        removeOwned(found->second.owned);
        constraints_.erase(found);
        return Status::success();
    }

    Result<SketchEntity> entity(EntityId id) const override {
        const auto found = entities_.find(id);
        if (found == entities_.end()) {
            return Result<SketchEntity>::failure(ErrorCode::NotFound, "Entity not found.");
        }
        return Result<SketchEntity>::success(readEntity(id, found->second));
    }

    Result<SketchConstraint> constraint(ConstraintId id) const override {
        const auto found = constraints_.find(id);
        if (found == constraints_.end()) return Result<SketchConstraint>::failure(ErrorCode::NotFound, "Constraint not found.");
        return Result<SketchConstraint>::success({id, found->second.definition});
    }

    Result<std::vector<SketchEntity>> entities(std::optional<EntityKind> kind) const override {
        std::vector<SketchEntity> result;
        if (!kind) {
            result.reserve(entities_.size());
        }
        for (const auto& [id, record] : entities_) {
            if (!kind || record.kind == *kind) {
                result.push_back(readEntity(id, record));
            }
        }
        return Result<std::vector<SketchEntity>>::success(std::move(result));
    }

    Result<std::vector<PointElement>> pointElements(PointElementScope scope) const override {
        std::vector<PointElement> result;
        for (const auto& [id, record] : entities_) {
            if (includesPoints(scope, record.kind)) {
                appendPointElements(result, readEntity(id, record));
            }
        }
        return Result<std::vector<PointElement>>::success(std::move(result));
    }

    size_t entityCount() const override { return entities_.size(); }
    size_t entityCount(EntityKind kind) const override {
        return std::count_if(entities_.begin(), entities_.end(), [kind](const auto& entry) { return entry.second.kind == kind; });
    }
    size_t constraintCount() const override { return constraints_.size(); }

    Result<std::vector<SketchConstraint>> constraints() const override {
        std::vector<SketchConstraint> result;
        result.reserve(constraints_.size());
        for (const auto& [id, record] : constraints_) result.push_back({id, record.definition});
        return Result<std::vector<SketchConstraint>>::success(std::move(result));
    }

    Result<SolveDiagnostics> solve() override { return solveWithDrag({}); }

    Result<SolveDiagnostics> dragPoints(std::span<const DragRequest> requests) override {
        std::vector<Slvs_hParam> dragged;
        dragged.reserve(requests.size() * 2);
        for (const auto& request : requests) {
            const auto handle = pointHandle(request.point);
            const auto& native = nativeEntities_.get(handle);
            setPoint(handle, request.target);
            dragged.push_back(native.param[0]);
            dragged.push_back(native.param[1]);
        }
        return solveWithDrag(std::move(dragged));
    }

private:
    struct OwnedRecords {
        std::vector<Slvs_hEntity> entities;
        std::vector<Slvs_hParam> params;
    };
    struct EntityRecord {
        EntityKind kind;
        bool construction;
        Slvs_hEntity handle = 0;
        OwnedRecords owned;
    };
    struct ConstraintRecord {
        ConstraintDefinition definition;
        Slvs_hConstraint handle = 0;
        OwnedRecords owned;
    };

    SketchEntity readEntity(EntityId id, const EntityRecord& record) const {
        const auto& native = nativeEntities_.get(record.handle);
        SketchGeometry geometry;
        switch (record.kind) {
            case EntityKind::Point:
                geometry = Point2{point(record.handle)};
                break;
            case EntityKind::Line:
                geometry = Line2{point(native.point[0]), point(native.point[1])};
                break;
            case EntityKind::Circle:
                geometry = Circle2{point(native.point[0]), params_.get(nativeEntities_.get(native.distance).param[0]).val};
                break;
            case EntityKind::Arc:
                geometry = Arc2{point(native.point[0]), point(native.point[1]), point(native.point[2])};
                break;
        }
        return {id, std::move(geometry), record.construction};
    }

    bool canAllocate(uint32_t params, uint32_t entities, uint32_t constraints) const {
        // Leave room for libslvs' temporary, generated constraint parameters.
        constexpr auto limit = std::numeric_limits<uint32_t>::max() / 2;
        return nextParam_ < limit - params && nextEntity_ < limit - entities && nextConstraint_ < limit - constraints;
    }
    static Status exhausted() { return Status::failure(ErrorCode::BackendFailure, "SolveSpace handle capacity exhausted."); }

    Slvs_hParam addParam(double value, Slvs_hGroup group = kSketchGroup) {
        const auto handle = nextParam_++;
        params_.add(Slvs_MakeParam(handle, group, value));
        return handle;
    }

    Slvs_hEntity addPoint(Vec2 position, OwnedRecords& owned, Slvs_hGroup group = kSketchGroup) {
        const auto u = addParam(position.x, group);
        const auto v = addParam(position.y, group);
        const auto handle = nextEntity_++;
        nativeEntities_.add(Slvs_MakePoint2d(handle, group, kWorkplane, u, v));
        owned.params.push_back(u);
        owned.params.push_back(v);
        owned.entities.push_back(handle);
        return handle;
    }

    void removeOwned(const OwnedRecords& owned) {
        for (auto handle : owned.entities) nativeEntities_.remove(handle);
        for (auto handle : owned.params) params_.remove(handle);
    }

    Vec2 point(Slvs_hEntity handle) const {
        const auto& native = nativeEntities_.get(handle);
        return {params_.get(native.param[0]).val, params_.get(native.param[1]).val};
    }

    void setPoint(Slvs_hEntity handle, Vec2 position) {
        const auto& native = nativeEntities_.get(handle);
        params_.get(native.param[0]).val = position.x;
        params_.get(native.param[1]).val = position.y;
    }

    Slvs_hEntity pointHandle(GeometryRef ref) const {
        const auto& record = entities_.at(ref.entity);
        const auto& native = nativeEntities_.get(record.handle);
        if (record.kind == EntityKind::Point) return record.handle;
        if (ref.sub == SubElement::Center) return native.point[0];
        if (record.kind == EntityKind::Line) return native.point[ref.sub == SubElement::Start ? 0 : 1];
        return native.point[ref.sub == SubElement::Start ? 1 : 2];
    }

    Slvs_Constraint makeConstraint(ConstraintRecord& record) {
        const auto& definition = record.definition;
        Slvs_Constraint result{};
        result.h = record.handle;
        result.group = kSketchGroup;
        result.wrkpl = kWorkplane;
        result.valA = definition.value.value_or(0.0);
        const auto first = entities_.at(definition.refs[0].entity).handle;
        const auto second = definition.refs.size() > 1 ? entities_.at(definition.refs[1].entity).handle : 0;
        switch (definition.type) {
            case ConstraintType::Coincident:
                result.type = SLVS_C_POINTS_COINCIDENT;
                result.ptA = pointHandle(definition.refs[0]);
                result.ptB = pointHandle(definition.refs[1]);
                break;
            case ConstraintType::Horizontal:
            case ConstraintType::Vertical:
                result.type = definition.type == ConstraintType::Horizontal ? SLVS_C_HORIZONTAL : SLVS_C_VERTICAL;
                result.entityA = first;
                break;
            case ConstraintType::Parallel:
            case ConstraintType::Perpendicular:
                result.type = definition.type == ConstraintType::Parallel ? SLVS_C_PARALLEL : SLVS_C_PERPENDICULAR;
                result.entityA = first;
                result.entityB = second;
                break;
            case ConstraintType::Equal:
                result.type = entities_.at(definition.refs[0].entity).kind == EntityKind::Line ? SLVS_C_EQUAL_LENGTH_LINES : SLVS_C_EQUAL_RADIUS;
                result.entityA = first;
                result.entityB = second;
                break;
            case ConstraintType::Distance:
                result.type = SLVS_C_PT_PT_DISTANCE;
                result.ptA = pointHandle(definition.refs[0]);
                result.ptB = pointHandle(definition.refs[1]);
                // The native length equation is singular at zero; coincidence
                // expresses precisely the zero-distance domain constraint.
                if (result.valA == 0.0) result.type = SLVS_C_POINTS_COINCIDENT;
                break;
            case ConstraintType::Length:
                result.type = SLVS_C_PT_PT_DISTANCE;
                result.ptA = nativeEntities_.get(first).point[0];
                result.ptB = nativeEntities_.get(first).point[1];
                break;
            case ConstraintType::Radius:
            case ConstraintType::Diameter:
                result.type = SLVS_C_DIAMETER;
                result.entityA = first;
                if (definition.type == ConstraintType::Radius) result.valA *= 2.0;
                break;
            case ConstraintType::Angle:
                result.type = SLVS_C_ANGLE;
                result.entityA = first;
                result.entityB = second;
                result.valA *= 180.0 / std::numbers::pi;
                break;
            case ConstraintType::Fix:
                result.type = SLVS_C_POINTS_COINCIDENT;
                result.ptA = pointHandle(definition.refs[0]);
                result.ptB = addPoint(definition.fixedPosition.value(), record.owned, kFixedGroup);
                break;
            case ConstraintType::Tangent:
                // supportsConstraint rejects this before any mutation.
                break;
        }
        return result;
    }

    Result<SolveDiagnostics> solveWithDrag(std::vector<Slvs_hParam> dragged) {
        auto& params = params_.values();
        auto& entities = nativeEntities_.values();
        auto& constraints = nativeConstraints_.values();
        constexpr auto limit = static_cast<size_t>(std::numeric_limits<int>::max());
        if (params.size() > limit || entities.size() > limit || constraints.size() > limit || dragged.size() > limit) {
            return Result<SolveDiagnostics>::failure(ErrorCode::BackendFailure, "SolveSpace array capacity exhausted.");
        }
        std::vector<Slvs_hConstraint> failed(constraints.size());
        Slvs_System system{};
        // Native solving can return before writing DOF (single-equation failure).
        system.dof = -1;
        system.param = params.data();
        system.params = static_cast<int>(params.size());
        system.entity = entities.data();
        system.entities = static_cast<int>(entities.size());
        system.constraint = constraints.data();
        system.constraints = static_cast<int>(constraints.size());
        system.dragged = dragged.data();
        system.ndragged = static_cast<int>(dragged.size());
        system.calculateFaileds = 1;
        system.failed = failed.data();
        system.faileds = static_cast<int>(failed.size());
        {
            std::lock_guard lock(solveSpaceMutex);
            // The public array API necessarily recreates temporary solver scratch
            // on each explicit solve. Its global stateful alternative has no
            // per-instance context, deletion, or constraint update API. Keeping
            // our persistent arrays avoids rebuilding the model for mutations,
            // supports independent sketches, and requires no SolveSpace fork.
            Slvs_Solve(&system, kSketchGroup);
        }
        SolveDiagnostics result;
        result.status = system.result == SLVS_RESULT_OKAY || system.result == SLVS_RESULT_REDUNDANT_OKAY ? SolveStatus::Converged : SolveStatus::Failed;
        // Native equations can be satisfied by a collapsed line/arc or invalid
        // radius. Such an iterate is not a valid solution to the Sketch model.
        if (result.status == SolveStatus::Converged && !geometryValid()) result.status = SolveStatus::Failed;
        if (system.result != SLVS_RESULT_TOO_MANY_UNKNOWNS && system.dof >= 0) result.degreesOfFreedom = system.dof;
        // A failure to converge only yields unsatisfied equations, not a proof
        // of conflicting constraints. Leave conflict diagnostics unavailable in
        // that case rather than mislabeling those handles as contradictions.
        if (system.result == SLVS_RESULT_OKAY || system.result == SLVS_RESULT_REDUNDANT_OKAY || system.result == SLVS_RESULT_INCONSISTENT) {
            result.conflictingConstraints.emplace();
            if (system.result != SLVS_RESULT_INCONSISTENT) result.redundantConstraints.emplace();
        }
        if (!result.conflictingConstraints) return Result<SolveDiagnostics>::success(std::move(result));
        auto& diagnosed = system.result == SLVS_RESULT_REDUNDANT_OKAY ? *result.redundantConstraints : *result.conflictingConstraints;
        const auto count = std::min(failed.size(), static_cast<size_t>(std::max(system.faileds, 0)));
        for (size_t i = 0; i != count; ++i) {
            const auto found = std::find_if(constraints_.begin(), constraints_.end(), [&](const auto& entry) { return entry.second.handle == failed[i]; });
            if (found != constraints_.end()) diagnosed.push_back(found->first);
        }
        return Result<SolveDiagnostics>::success(std::move(result));
    }

    bool geometryValid() const {
        for (const auto& [id, record] : entities_) {
            const auto geometry = entity(id).value().geometry;
            const bool valid = std::visit(
                [](const auto& g) {
                    const auto finite = [](Vec2 p) { return std::isfinite(p.x) && std::isfinite(p.y); };
                    const auto distance = [](Vec2 a, Vec2 b) { return std::hypot(a.x - b.x, a.y - b.y); };
                    using T = std::decay_t<decltype(g)>;
                    if constexpr (std::is_same_v<T, Point2>) {
                        return finite(g.position);
                    } else if constexpr (std::is_same_v<T, Line2>) {
                        const double length = distance(g.start, g.end);
                        return finite(g.start) && finite(g.end) && std::isfinite(length) && length > 0;
                    } else if constexpr (std::is_same_v<T, Circle2>) {
                        return finite(g.center) && std::isfinite(g.radius) && g.radius > 0;
                    } else {
                        const double a = distance(g.center, g.start);
                        const double b = distance(g.center, g.end);
                        return finite(g.center) && finite(g.start) && finite(g.end) && std::isfinite(a) && std::isfinite(b) && a > 0 && b > 0 &&
                               g.start != g.end && std::abs(a - b) <= 1e-6 * (1 + std::max(a, b));
                    }
                },
                geometry);
            if (!valid) return false;
        }
        return true;
    }

    NativeRecords<Slvs_Param> params_;
    NativeRecords<Slvs_Entity> nativeEntities_;
    NativeRecords<Slvs_Constraint> nativeConstraints_;
    std::map<EntityId, EntityRecord> entities_;
    std::map<ConstraintId, ConstraintRecord> constraints_;
    Slvs_hParam nextParam_ = 1;
    Slvs_hEntity nextEntity_ = 1;
    Slvs_hConstraint nextConstraint_ = 1;
};

}  // namespace

Result<std::unique_ptr<ISketchBackend>> makeSolveSpaceSketchBackend() {
    return Result<std::unique_ptr<ISketchBackend>>::success(std::make_unique<SolveSpaceSketchBackend>());
}

}  // namespace core::sketch::detail
