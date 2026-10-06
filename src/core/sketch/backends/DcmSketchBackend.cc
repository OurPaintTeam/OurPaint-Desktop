#include <algorithm>
#include <cmath>
#include <exception>
#include <map>
#include <numbers>
#include <stdexcept>
#include <type_traits>

#include "DCMManager.h"
#include "ISketchBackend.h"

namespace core::sketch::detail {
namespace {
namespace dcm = OurPaintDCM;
namespace du = OurPaintDCM::Utils;

template <class T, class F>
Result<T> checked(F&& operation) {
    try {
        return Result<T>::success(operation());
    } catch (const std::exception& error) {
        return Result<T>::failure(ErrorCode::BackendFailure, error.what());
    }
}

template <class F>
Status checkedStatus(F&& operation) {
    try {
        operation();
        return Status::success();
    } catch (const std::exception& error) {
        return Status::failure(ErrorCode::BackendFailure, error.what());
    }
}

Vec2 subtract(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
double length(Vec2 v) { return std::hypot(v.x, v.y); }
bool near(double actual, double expected) { return std::isfinite(actual) && std::abs(actual - expected) <= 1e-6 * (1.0 + std::abs(expected)); }

// All live coordinates reside in this stable-address manager. Records contain
// only public identity, construction metadata, immutable constraint definitions,
// and private handle mappings. DCM's owned points never become public entities.
class DcmSketchBackend final : public ISketchBackend {
    struct EntityRecord {
        du::ID handle;
        EntityKind kind;
        bool construction;
        std::optional<du::ID> arcRequirement;
    };
    struct ConstraintRecord {
        ConstraintDefinition definition;
        std::vector<du::ID> requirements;
        std::optional<du::ID> anchor;
    };

public:
    BackendKind kind() const override { return BackendKind::Dcm; }

    Capabilities capabilities() const override {
        Capabilities result;
        result.entities.fill(true);
        for (auto type : {ConstraintType::Coincident, ConstraintType::Horizontal, ConstraintType::Vertical, ConstraintType::Parallel,
                          ConstraintType::Perpendicular, ConstraintType::Distance, ConstraintType::Length, ConstraintType::Angle, ConstraintType::Fix}) {
            result.constraints[static_cast<size_t>(type)] = true;
        }
        result.updateEntities = result.removeEntities = result.updateConstraints = result.removeConstraints = true;
        result.solve = result.drag = true;
        // DCM's rank diagnosis counts only variables in its function system,
        // omitting unconstrained geometry and eliminated aliases. It is not a
        // sound whole-sketch DOF/conflict/redundancy report.
        return result;
    }

    Status supportsConstraint(const ConstraintDefinition& definition) const override {
        if (!capabilities().supports(definition.type)) {
            return Status::failure(ErrorCode::Unsupported, "This constraint is not supported by the DCM adapter");
        }
        for (const auto& ref : definition.refs) {
            if (!entities_.contains(ref.entity)) {
                return Status::failure(ErrorCode::NotFound, "Constraint entity was not found");
            }
        }
        return Status::success();
    }

    Status addEntity(const SketchEntity& entity) override {
        if (entities_.contains(entity.id)) {
            return Status::failure(ErrorCode::InvalidArgument, "Entity ID already exists");
        }
        return checkedStatus([&] {
            const auto handle = manager_.addFigure(describe(entity.geometry));
            EntityRecord record{handle, entityKind(entity.geometry), entity.construction, std::nullopt};
            try {
                // DCM represents an arc as three unconstrained points. The
                // adapter installs its intrinsic circularity equation privately.
                if (std::holds_alternative<Arc2>(entity.geometry)) {
                    record.arcRequirement = manager_.addRequirement(du::RequirementDescriptor::arcCenterOnPerpendicular(handle));
                }
                entities_.emplace(entity.id, record);
            } catch (...) {
                manager_.removeFigure(handle, true);
                throw;
            }
        });
    }

    Status updateEntity(const SketchEntity& entity) override { return updateEntities(std::span(&entity, 1)); }

    Status updateEntities(std::span<const SketchEntity> entities) override {
        return checkedStatus([&] {
            std::vector<du::FigureUpdateDescriptor> updates;
            updates.reserve(entities.size());
            for (const auto& entity : entities) {
                const auto& record = entities_.at(entity.id);
                if (record.kind != entityKind(entity.geometry)) {
                    throw std::logic_error("Entity kind changed after validation");
                }
                const auto descriptor = describe(entity.geometry);
                du::FigureUpdateDescriptor update(record.handle, descriptor.type);
                update.coords = descriptor.coords;
                update.x = descriptor.x;
                update.y = descriptor.y;
                update.radius = descriptor.radius;
                updates.push_back(std::move(update));
            }
            // GLOBAL mode prevents numerical solving on ordinary mutations.
            // Native batching resolves fixed/coincident groups once. Input order
            // wins within unfixed groups (Start, End, Center within each curve).
            manager_.updateFigures(updates);
            for (const auto& entity : entities) {
                entities_.at(entity.id).construction = entity.construction;
            }
        });
    }

    Status updatePointGuesses(std::span<const PointElement> points) override {
        return checkedStatus([&] {
            std::vector<du::PointUpdateDescriptor> updates;
            updates.reserve(points.size());
            for (const auto& point : points) {
                updates.push_back({pointHandle(point.ref), point.position.x, point.position.y});
            }
            // Ordinary GLOBAL edits project hard groups but do not solve.
            manager_.updatePoints(updates);
        });
    }

    Status setConstruction(EntityId id, bool construction) override {
        const auto it = entities_.find(id);
        if (it == entities_.end()) {
            return Status::failure(ErrorCode::NotFound, "Entity was not found");
        }
        it->second.construction = construction;
        return Status::success();
    }

    Status clear() override {
        return checkedStatus([&] {
            manager_.clear();
            constraints_.clear();
            entities_.clear();
        });
    }

    Status removeEntity(EntityId id) override {
        const auto it = entities_.find(id);
        if (it == entities_.end()) {
            return Status::failure(ErrorCode::NotFound, "Entity was not found");
        }
        return checkedStatus([&] {
            for (auto constraint = constraints_.begin(); constraint != constraints_.end();) {
                const auto& refs = constraint->second.definition.refs;
                if (std::any_of(refs.begin(), refs.end(), [&](const auto& ref) { return ref.entity == id; })) {
                    removeHandles(constraint->second);
                    constraint = constraints_.erase(constraint);
                } else {
                    ++constraint;
                }
            }
            // Owned points are exclusive to this entity, so cascading here can
            // never remove another public entity. It also removes arc invariants.
            manager_.removeFigure(it->second.handle, true);
            entities_.erase(it);
        });
    }

    Status addConstraint(const SketchConstraint& constraint) override {
        if (constraints_.contains(constraint.id)) {
            return Status::failure(ErrorCode::InvalidArgument, "Constraint ID already exists");
        }
        const auto supported = supportsConstraint(constraint.definition);
        if (!supported) {
            return supported;
        }
        return checkedStatus([&] { constraints_.emplace(constraint.id, addHandles(constraint.definition)); });
    }

    Status updateConstraint(const SketchConstraint& constraint) override {
        const auto it = constraints_.find(constraint.id);
        if (it == constraints_.end()) {
            return Status::failure(ErrorCode::NotFound, "Constraint was not found");
        }
        const auto supported = supportsConstraint(constraint.definition);
        if (!supported) {
            return supported;
        }
        return checkedStatus([&] {
            const auto& previous = it->second.definition;
            const auto& next = constraint.definition;
            if (previous.type == next.type && previous.refs == next.refs && next.value) {
                manager_.updateRequirementParam(it->second.requirements.front(), dcmParameter(next));
                it->second.definition = next;
                return;
            }
            // DCM cannot retarget a requirement. Replace just its translated
            // records, preserving the public ID and the rest of the live model.
            auto replacement = addHandles(next);
            removeHandles(it->second);
            it->second = std::move(replacement);
        });
    }

    Status removeConstraint(ConstraintId id) override {
        const auto it = constraints_.find(id);
        if (it == constraints_.end()) {
            return Status::failure(ErrorCode::NotFound, "Constraint was not found");
        }
        return checkedStatus([&] {
            removeHandles(it->second);
            constraints_.erase(it);
        });
    }

    Result<SketchEntity> entity(EntityId id) const override {
        const auto it = entities_.find(id);
        if (it == entities_.end()) {
            return Result<SketchEntity>::failure(ErrorCode::NotFound, "Entity was not found");
        }
        return checked<SketchEntity>([&] { return readEntity(id, it->second); });
    }

    Result<SketchConstraint> constraint(ConstraintId id) const override {
        const auto it = constraints_.find(id);
        if (it == constraints_.end()) {
            return Result<SketchConstraint>::failure(ErrorCode::NotFound, "Constraint was not found");
        }
        return Result<SketchConstraint>::success({id, it->second.definition});
    }

    Result<std::vector<SketchEntity>> entities(std::optional<EntityKind> kind) const override {
        return checked<std::vector<SketchEntity>>([&] {
            std::vector<SketchEntity> result;
            if (!kind) {
                result.reserve(entities_.size());
            }
            for (const auto& [id, record] : entities_) {
                if (!kind || record.kind == *kind) {
                    result.push_back(readEntity(id, record));
                }
            }
            return result;
        });
    }

    Result<std::vector<PointElement>> pointElements(PointElementScope scope) const override {
        return checked<std::vector<PointElement>>([&] {
            std::vector<PointElement> result;
            for (const auto& [id, record] : entities_) {
                if (includesPoints(scope, record.kind)) {
                    appendPointElements(result, readEntity(id, record));
                }
            }
            return result;
        });
    }

    size_t entityCount() const override { return entities_.size(); }
    size_t entityCount(EntityKind kind) const override {
        return std::count_if(entities_.begin(), entities_.end(), [kind](const auto& entry) { return entry.second.kind == kind; });
    }
    size_t constraintCount() const override { return constraints_.size(); }

    Result<std::vector<SketchConstraint>> constraints() const override {
        return checked<std::vector<SketchConstraint>>([&] {
            std::vector<SketchConstraint> result;
            result.reserve(constraints_.size());
            for (const auto& [id, record] : constraints_) {
                result.push_back({id, record.definition});
            }
            return result;
        });
    }

    Result<SolveDiagnostics> solve() override {
        return checked<SolveDiagnostics>([&] {
            const bool converged = manager_.solve();
            SolveDiagnostics result;
            result.status = converged && constraintsSatisfied() ? SolveStatus::Converged : SolveStatus::Failed;
            return result;
        });
    }

    Result<SolveDiagnostics> dragPoints(std::span<const DragRequest> requests) override {
        return checked<SolveDiagnostics>([&] {
            std::vector<du::PointUpdateDescriptor> updates;
            updates.reserve(requests.size());
            for (const auto& request : requests) {
                updates.push_back({pointHandle(request.point), request.target.x, request.target.y});
            }
            // DCM discards the convergence result from its incremental drag
            // solve. Validate residuals directly instead of reporting success
            // merely because updatePoints returned. Restore ordinary edit mode
            // even if the backend throws, without running a second global solve.
            struct RestoreMode {
                dcm::DCMManager& manager;
                ~RestoreMode() { manager.setSolveMode(du::SolveMode::GLOBAL); }
            } restore{manager_};
            manager_.setSolveMode(du::SolveMode::DRAG);
            manager_.updatePoints(updates);
            SolveDiagnostics result;
            result.status = constraintsSatisfied() ? SolveStatus::Converged : SolveStatus::Failed;
            return result;
        });
    }

private:
    static du::FigureDescriptor describe(const SketchGeometry& geometry) {
        return std::visit(
            [](const auto& value) -> du::FigureDescriptor {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, Point2>) {
                    return du::FigureDescriptor::point(value.position.x, value.position.y);
                } else if constexpr (std::is_same_v<T, Line2>) {
                    return du::FigureDescriptor::line(value.start.x, value.start.y, value.end.x, value.end.y);
                } else if constexpr (std::is_same_v<T, Circle2>) {
                    return du::FigureDescriptor::circle(value.center.x, value.center.y, value.radius);
                } else {
                    return du::FigureDescriptor::arc(value.start.x, value.start.y, value.end.x, value.end.y, value.center.x, value.center.y);
                }
            },
            geometry);
    }

    du::FigureDescriptor figure(du::ID handle) const {
        const auto descriptor = manager_.getFigure(handle);
        if (!descriptor) {
            throw std::logic_error("DCM figure mapping is invalid");
        }
        return *descriptor;
    }

    SketchEntity readEntity(EntityId id, const EntityRecord& record) const {
        const auto descriptor = figure(record.handle);
        const auto& c = descriptor.coords;
        SketchGeometry geometry;
        switch (descriptor.type) {
            case du::FigureType::ET_POINT2D:
                geometry = Point2{{c.at(0), c.at(1)}};
                break;
            case du::FigureType::ET_LINE:
                geometry = Line2{{c.at(0), c.at(1)}, {c.at(2), c.at(3)}};
                break;
            case du::FigureType::ET_CIRCLE:
                geometry = Circle2{{c.at(0), c.at(1)}, descriptor.radius.value()};
                break;
            case du::FigureType::ET_ARC:
                geometry = Arc2{{c.at(4), c.at(5)}, {c.at(0), c.at(1)}, {c.at(2), c.at(3)}};
                break;
            default:
                throw std::logic_error("Unknown DCM geometry kind");
        }
        return {id, geometry, record.construction};
    }

    du::ID pointHandle(const GeometryRef& ref) const {
        const auto handle = entities_.at(ref.entity).handle;
        const auto descriptor = figure(handle);
        if (descriptor.type == du::FigureType::ET_POINT2D && ref.sub == SubElement::Whole) {
            return handle;
        }
        if (ref.sub == SubElement::Start) {
            return descriptor.pointIds.at(0);
        }
        if (ref.sub == SubElement::End) {
            return descriptor.pointIds.at(1);
        }
        if (ref.sub == SubElement::Center) {
            return descriptor.pointIds.at(descriptor.type == du::FigureType::ET_ARC ? 2 : 0);
        }
        throw std::logic_error("Invalid point reference after validation");
    }

    Vec2 point(const GeometryRef& ref) const {
        const auto descriptor = figure(pointHandle(ref));
        return {descriptor.coords.at(0), descriptor.coords.at(1)};
    }

    Vec2 lineVector(const GeometryRef& ref) const {
        const auto descriptor = figure(entities_.at(ref.entity).handle);
        return {descriptor.coords.at(2) - descriptor.coords.at(0), descriptor.coords.at(3) - descriptor.coords.at(1)};
    }

    static double dcmParameter(const ConstraintDefinition& definition) {
        // DCM's actual math pipeline takes degrees; its separate diagnostic
        // function layer uses radians. Keep that mismatch behind this adapter
        // and use our own residual checks in the public radians convention.
        return definition.value.value() * (definition.type == ConstraintType::Angle ? 180.0 / std::numbers::pi : 1.0);
    }

    ConstraintRecord addHandles(const ConstraintDefinition& definition) {
        ConstraintRecord record{definition, {}, std::nullopt};
        const auto whole = [&](size_t index) { return entities_.at(definition.refs.at(index).entity).handle; };
        const auto pointAt = [&](size_t index) { return pointHandle(definition.refs.at(index)); };
        const auto add = [&](const du::RequirementDescriptor& descriptor) { record.requirements.push_back(manager_.addRequirement(descriptor)); };
        try {
            switch (definition.type) {
                case ConstraintType::Coincident:
                    add(du::RequirementDescriptor::pointOnPoint(pointAt(0), pointAt(1)));
                    break;
                case ConstraintType::Horizontal:
                    add(du::RequirementDescriptor::horizontal(whole(0)));
                    break;
                case ConstraintType::Vertical:
                    add(du::RequirementDescriptor::vertical(whole(0)));
                    break;
                case ConstraintType::Parallel:
                    add(du::RequirementDescriptor::lineLineParallel(whole(0), whole(1)));
                    break;
                case ConstraintType::Perpendicular:
                    add(du::RequirementDescriptor::lineLinePerpendicular(whole(0), whole(1)));
                    break;
                case ConstraintType::Distance:
                    add(du::RequirementDescriptor::pointPointDist(pointAt(0), pointAt(1), dcmParameter(definition)));
                    break;
                case ConstraintType::Length: {
                    const auto descriptor = figure(whole(0));
                    add(du::RequirementDescriptor::pointPointDist(descriptor.pointIds.at(0), descriptor.pointIds.at(1), dcmParameter(definition)));
                    break;
                }
                case ConstraintType::Angle:
                    add(du::RequirementDescriptor::lineLineAngle(whole(0), whole(1), dcmParameter(definition)));
                    break;
                case ConstraintType::Fix: {
                    const auto target = definition.fixedPosition.value();
                    // A private fixed anchor preserves the explicit target even
                    // when imported geometry differs from it. Fixing the public
                    // point directly would silently recapture its current value.
                    record.anchor = manager_.addFigure(du::FigureDescriptor::point(target.x, target.y));
                    add(du::RequirementDescriptor::fixPoint(*record.anchor));
                    add(du::RequirementDescriptor::pointOnPoint(pointAt(0), *record.anchor));
                    break;
                }
                default:
                    throw std::logic_error("Unsupported constraint passed backend preflight");
            }
        } catch (...) {
            removeHandles(record);
            throw;
        }
        return record;
    }

    void removeHandles(const ConstraintRecord& record) {
        // Remove coincidence before destroying a fixed anchor; otherwise DCM's
        // cascade could invalidate a still-live translated requirement.
        for (auto it = record.requirements.rbegin(); it != record.requirements.rend(); ++it) {
            manager_.removeRequirement(*it);
        }
        if (record.anchor) {
            manager_.removeFigure(*record.anchor, true);
        }
    }

    bool constraintsSatisfied() const {
        // DCM can return true with no free variables even when constraints
        // conflict. These read-only checks also reject invalid solver iterates
        // and do not copy or synchronize a second live geometric model.
        for (const auto& [id, record] : entities_) {
            const auto geometry = readEntity(id, record).geometry;
            const bool valid = std::visit(
                [](const auto& value) {
                    const auto finite = [](Vec2 p) { return std::isfinite(p.x) && std::isfinite(p.y); };
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (std::is_same_v<T, Point2>)
                        return finite(value.position);
                    else if constexpr (std::is_same_v<T, Line2>) {
                        const double size = length(subtract(value.end, value.start));
                        return finite(value.start) && finite(value.end) && std::isfinite(size) && size > 0;
                    } else if constexpr (std::is_same_v<T, Circle2>)
                        return finite(value.center) && std::isfinite(value.radius) && value.radius > 0;
                    else {
                        const double radius = length(subtract(value.start, value.center));
                        return finite(value.center) && finite(value.start) && finite(value.end) && radius > 0 && length(subtract(value.start, value.end)) > 0 &&
                               near(length(subtract(value.end, value.center)), radius);
                    }
                },
                geometry);
            if (!valid) {
                return false;
            }
        }
        for (const auto& [id, record] : constraints_) {
            const auto& definition = record.definition;
            const auto& refs = definition.refs;
            switch (definition.type) {
                case ConstraintType::Coincident:
                    if (!near(length(subtract(point(refs[0]), point(refs[1]))), 0)) return false;
                    break;
                case ConstraintType::Distance:
                    if (!near(length(subtract(point(refs[0]), point(refs[1]))), *definition.value)) return false;
                    break;
                case ConstraintType::Fix:
                    if (!near(length(subtract(point(refs[0]), *definition.fixedPosition)), 0)) return false;
                    break;
                case ConstraintType::Horizontal:
                case ConstraintType::Vertical: {
                    // Normalize orientation residuals: shrinking a line toward
                    // zero must not satisfy contradictory H/V constraints.
                    const auto direction = lineVector(refs[0]);
                    const double component = definition.type == ConstraintType::Horizontal ? direction.y : direction.x;
                    if (!near(component / length(direction), 0)) return false;
                    break;
                }
                case ConstraintType::Length:
                    if (!near(length(lineVector(refs[0])), *definition.value)) return false;
                    break;
                case ConstraintType::Parallel:
                case ConstraintType::Perpendicular:
                case ConstraintType::Angle: {
                    const auto a = lineVector(refs[0]);
                    const auto b = lineVector(refs[1]);
                    const double scale = length(a) * length(b);
                    if (!(scale > 0) || !std::isfinite(scale)) return false;
                    const double dot = (a.x * b.x + a.y * b.y) / scale;
                    if (definition.type == ConstraintType::Parallel && !near((a.x * b.y - a.y * b.x) / scale, 0)) return false;
                    if (definition.type == ConstraintType::Perpendicular && !near(dot, 0)) return false;
                    if (definition.type == ConstraintType::Angle && !near(std::acos(std::clamp(dot, -1.0, 1.0)), *definition.value)) return false;
                    break;
                }
                default:
                    return false;
            }
        }
        return true;
    }

    dcm::DCMManager manager_;
    std::map<EntityId, EntityRecord> entities_;
    std::map<ConstraintId, ConstraintRecord> constraints_;
};
}  // namespace

Result<std::unique_ptr<ISketchBackend>> makeDcmSketchBackend() {
    return checked<std::unique_ptr<ISketchBackend>>([] { return std::make_unique<DcmSketchBackend>(); });
}
}  // namespace core::sketch::detail
