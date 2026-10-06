#include "Sketch.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <map>
#include <numbers>
#include <numeric>
#include <set>
#include <type_traits>

#include "../DSU.h"
#include "backends/ISketchBackend.h"

namespace core::sketch {
namespace {

bool finite(Vec2 p) { return std::isfinite(p.x) && std::isfinite(p.y); }
double distance(Vec2 a, Vec2 b) { return std::hypot(a.x - b.x, a.y - b.y); }

Status invalid(const char* message) { return Status::failure(ErrorCode::InvalidArgument, message); }

Status validateGeometry(const SketchGeometry& geometry, bool transferring = false) {
    const bool valid = std::visit(
        [transferring](const auto& g) {
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
                return finite(g.center) && finite(g.start) && finite(g.end) && std::isfinite(a) && std::isfinite(b) && a > 0 && b > 0 && g.start != g.end &&
                       (transferring || std::abs(a - b) <= 1e-9 * std::max(a, b));
            }
        },
        geometry);
    return valid ? Status::success() : invalid("Geometry must be finite and nondegenerate; arc endpoints must have equal positive radii");
}

using detail::pointLike;

bool validKind(EntityKind kind) { return static_cast<size_t>(kind) < 4; }

}  // namespace

Sketch::Sketch(std::unique_ptr<detail::ISketchBackend> backend) : backend_(std::move(backend)) {}
Sketch::~Sketch() = default;

BackendKind Sketch::defaultBackend() {
#if OURPAINT_SKETCH_DEFAULT_SOLVESPACE
    return BackendKind::SolveSpace;
#else
    return BackendKind::Dcm;
#endif
}

std::vector<BackendKind> Sketch::availableBackends() {
    std::vector<BackendKind> result;
#if OURPAINT_SKETCH_WITH_DCM
    result.push_back(BackendKind::Dcm);
#endif
#if OURPAINT_SKETCH_WITH_SOLVESPACE
    result.push_back(BackendKind::SolveSpace);
#endif
    return result;
}

Result<std::unique_ptr<Sketch>> Sketch::create(BackendKind kind) {
    using Creation = Result<std::unique_ptr<Sketch>>;
    try {
        auto backend = Result<std::unique_ptr<detail::ISketchBackend>>::failure(ErrorCode::Unsupported, "Backend is not available in this build");
        switch (kind) {
            case BackendKind::Dcm:
#if OURPAINT_SKETCH_WITH_DCM
                backend = detail::makeDcmSketchBackend();
#endif
                break;
            case BackendKind::SolveSpace:
#if OURPAINT_SKETCH_WITH_SOLVESPACE
                backend = detail::makeSolveSpaceSketchBackend();
#endif
                break;
            default:
                return Creation::failure(ErrorCode::InvalidArgument, "Unknown sketch backend");
        }
        if (!backend) {
            return Creation::failure(backend.error().code, backend.error().message);
        }
        return Creation::success(std::unique_ptr<Sketch>(new Sketch(std::move(backend.value()))));
    } catch (const std::exception& error) {
        return Creation::failure(ErrorCode::BackendFailure, error.what());
    }
}

BackendKind Sketch::backendKind() const { return backend_->kind(); }
Capabilities Sketch::capabilities() const { return backend_->capabilities(); }

Result<EntityId> Sketch::addEntity(const SketchGeometry& geometry, bool construction) {
    const EntityCreation input{geometry, construction};
    auto result = addEntities(std::span(&input, 1));
    if (!result) {
        return Result<EntityId>::failure(result.error().code, result.error().message);
    }
    return Result<EntityId>::success(result.value().front());
}

Status Sketch::updateEntity(EntityId id, const SketchGeometry& geometry, std::optional<bool> construction) {
    const EntityUpdate input{id, geometry, construction};
    return updateEntities(std::span(&input, 1));
}

Status Sketch::updatePoint(EntityId id, Point2 point) {
    const EntityUpdate input{id, point};
    return updateEntities(std::span(&input, 1));
}

Status Sketch::updateLine(EntityId id, Line2 line) {
    const EntityUpdate input{id, line};
    return updateEntities(std::span(&input, 1));
}

Status Sketch::updateCircle(EntityId id, Circle2 circle) {
    const EntityUpdate input{id, circle};
    return updateEntities(std::span(&input, 1));
}

Status Sketch::updateArc(EntityId id, Arc2 arc) {
    const EntityUpdate input{id, arc};
    return updateEntities(std::span(&input, 1));
}

Status Sketch::updateCircleRadius(EntityId id, double radius) {
    auto result = entity(id);
    if (!result) {
        return Status::failure(result.error().code, result.error().message);
    }

    auto* circle = std::get_if<Circle2>(&result.value().geometry);
    if (!circle) {
        return Status::failure(ErrorCode::InvalidArgument, "Entity is not a circle");
    }

    circle->radius = radius;
    return updateEntity(id, *circle);
}

Status Sketch::moveEntities(std::span<const EntityId> ids, Vec2 offset) {
    if (ids.empty()) {
        return Status::success();
    }
    if (!finite(offset)) {
        return invalid("Movement offset must be finite");
    }
    const auto translate = [offset](Vec2& p) {
        p.x += offset.x;
        p.y += offset.y;
    };
    std::vector<EntityUpdate> updates;
    updates.reserve(ids.size());
    for (auto id : ids) {
        auto current = entity(id);
        if (!current) {
            return Status::failure(current.error().code, current.error().message);
        }
        auto geometry = std::move(current.value().geometry);
        std::visit(
            [&translate](auto& g) {
                using T = std::decay_t<decltype(g)>;
                if constexpr (std::is_same_v<T, Point2>) {
                    translate(g.position);
                } else if constexpr (std::is_same_v<T, Line2>) {
                    translate(g.start);
                    translate(g.end);
                } else if constexpr (std::is_same_v<T, Circle2>) {
                    translate(g.center);
                } else {
                    translate(g.center);
                    translate(g.start);
                    translate(g.end);
                }
            },
            geometry);
        updates.push_back({id, std::move(geometry), std::nullopt});
    }
    return updateEntities(updates);
}

Status Sketch::setConstruction(EntityId id, bool construction) {
    if (id.get() <= 0) {
        return invalid("Entity ID must be positive");
    }
    if (!capabilities().updateEntities) {
        return Status::failure(ErrorCode::Unsupported, "Entity updates are unsupported");
    }
    return backend_->setConstruction(id, construction);
}

Status Sketch::removeEntity(EntityId id) { return removeEntities(std::span(&id, 1)); }

Result<std::vector<EntityId>> Sketch::addEntities(std::span<const EntityCreation> input) {
    using Added = Result<std::vector<EntityId>>;
    if (input.empty()) {
        return Added::success({});
    }
    const auto support = capabilities();
    for (const auto& item : input) {
        auto valid = validateGeometry(item.geometry);
        if (!valid) {
            return Added::failure(valid.error().code, valid.error().message);
        }
        if (!support.supports(entityKind(item.geometry))) {
            return Added::failure(ErrorCode::Unsupported, "Entity kind is unsupported");
        }
    }
    const auto remaining = std::numeric_limits<int64_t>::max() - entityIds_.getLast().get();
    if (input.size() > static_cast<uint64_t>(remaining)) {
        return Added::failure(ErrorCode::BackendFailure, "Entity ID space exhausted");
    }
    std::vector<EntityId> ids;
    std::vector<SketchEntity> records;
    ids.reserve(input.size());
    records.reserve(input.size());
    for (const auto& item : input) {
        const EntityId id(entityIds_.generate().get());
        ids.push_back(id);
        records.push_back({id, item.geometry, item.construction});
    }
    auto added = backend_->addEntities(records);
    if (!added) {
        return Added::failure(added.error().code, added.error().message);
    }
    return Added::success(std::move(ids));
}

Status Sketch::updateEntities(std::span<const EntityUpdate> input) {
    if (input.empty()) {
        return Status::success();
    }
    std::set<EntityId> targets;
    std::vector<SketchEntity> records;
    records.reserve(input.size());
    for (const auto& item : input) {
        if (!targets.insert(item.id).second) {
            return invalid("Duplicate update target ID");
        }
        auto valid = validateGeometry(item.geometry);
        if (!valid) {
            return valid;
        }
        auto current = entity(item.id);
        if (!current) {
            return Status::failure(current.error().code, current.error().message);
        }
        if (entityKind(current.value().geometry) != entityKind(item.geometry)) {
            return invalid("An update cannot change entity kind");
        }
        records.push_back({item.id, item.geometry, item.construction.value_or(current.value().construction)});
    }
    if (!capabilities().updateEntities) {
        return Status::failure(ErrorCode::Unsupported, "Entity updates are unsupported");
    }
    return backend_->updateEntities(records);
}

Status Sketch::removeEntities(std::span<const EntityId> ids) {
    if (ids.empty()) {
        return Status::success();
    }
    std::set<EntityId> targets;
    for (auto id : ids) {
        if (!targets.insert(id).second) {
            return invalid("Duplicate removal target ID");
        }
        auto current = entity(id);
        if (!current) {
            return Status::failure(current.error().code, current.error().message);
        }
    }
    if (!capabilities().removeEntities) {
        return Status::failure(ErrorCode::Unsupported, "Entity removal is unsupported");
    }
    return backend_->removeEntities(ids);
}

Status Sketch::clear() { return backend_->clear(); }

Status Sketch::validateConstraint(const ConstraintDefinition& definition) const {
    const auto type = definition.type;
    size_t count = 0;
    bool dimensional = false;
    switch (type) {
        case ConstraintType::Coincident:
        case ConstraintType::Parallel:
        case ConstraintType::Perpendicular:
        case ConstraintType::Tangent:
        case ConstraintType::Equal:
            count = 2;
            break;
        case ConstraintType::Horizontal:
        case ConstraintType::Vertical:
        case ConstraintType::Fix:
            count = 1;
            break;
        case ConstraintType::Distance:
        case ConstraintType::Angle:
            count = 2;
            dimensional = true;
            break;
        case ConstraintType::Length:
        case ConstraintType::Radius:
        case ConstraintType::Diameter:
            count = 1;
            dimensional = true;
            break;
        default:
            return invalid("Unknown constraint type");
    }
    if (definition.refs.size() != count) {
        return invalid("Wrong number of constraint references");
    }
    if (count == 2 && definition.refs[0] == definition.refs[1]) {
        return invalid("A constraint needs distinct references");
    }
    if (definition.value.has_value() != dimensional) {
        return invalid("Unexpected or missing dimension value");
    }
    if (dimensional) {
        const double v = *definition.value;
        if (!std::isfinite(v) || v < 0) {
            return invalid("Dimensions must be finite and nonnegative");
        }
        if (type != ConstraintType::Distance && type != ConstraintType::Angle && v == 0) {
            return invalid("This dimension must be positive");
        }
        if (type == ConstraintType::Angle && v > std::numbers::pi) {
            return invalid("Angle must be in [0, pi] radians");
        }
    }
    if (definition.fixedPosition.has_value() != (type == ConstraintType::Fix)) {
        return invalid("Only Fix requires a fixed position");
    }
    if (definition.fixedPosition && !finite(*definition.fixedPosition)) {
        return invalid("Fixed position must be finite");
    }

    std::array<bool, 2> points{}, lines{}, circles{};
    for (size_t i = 0; i < count; ++i) {
        const auto& ref = definition.refs[i];
        auto target = entity(ref.entity);
        if (!target) {
            return Status::failure(target.error().code, target.error().message);
        }
        const auto kind = entityKind(target.value().geometry);
        points[i] = pointLike(kind, ref.sub);
        lines[i] = kind == EntityKind::Line && ref.sub == SubElement::Whole;
        circles[i] = (kind == EntityKind::Circle || kind == EntityKind::Arc) && ref.sub == SubElement::Whole;
    }
    bool valid = false;
    switch (type) {
        case ConstraintType::Coincident:
        case ConstraintType::Distance:
            valid = points[0] && points[1];
            break;
        case ConstraintType::Horizontal:
        case ConstraintType::Vertical:
        case ConstraintType::Length:
            valid = lines[0];
            break;
        case ConstraintType::Parallel:
        case ConstraintType::Perpendicular:
        case ConstraintType::Angle:
            valid = lines[0] && lines[1];
            break;
        case ConstraintType::Tangent:
            valid = (lines[0] && circles[1]) || (circles[0] && lines[1]) || (circles[0] && circles[1]);
            break;
        case ConstraintType::Equal:
            valid = (lines[0] && lines[1]) || (circles[0] && circles[1]);
            break;
        case ConstraintType::Radius:
        case ConstraintType::Diameter:
            valid = circles[0];
            break;
        case ConstraintType::Fix:
            valid = points[0];
            break;
    }
    return valid ? Status::success() : invalid("Constraint references are incompatible with its type");
}

Status Sketch::supportsConstraint(const ConstraintDefinition& definition) const {
    auto valid = validateConstraint(definition);
    if (!valid) {
        return valid;
    }
    return backend_->supportsConstraint(definition);
}

Result<ConstraintId> Sketch::addConstraint(const ConstraintDefinition& definition) {
    auto valid = supportsConstraint(definition);
    if (!valid) {
        return Result<ConstraintId>::failure(valid.error().code, valid.error().message);
    }
    if (constraintIds_.getLast().get() == std::numeric_limits<int64_t>::max()) {
        return Result<ConstraintId>::failure(ErrorCode::BackendFailure, "Constraint ID space exhausted");
    }
    const ConstraintId id(constraintIds_.generate().get());
    auto result = backend_->addConstraint({id, definition});
    if (!result) {
        return Result<ConstraintId>::failure(result.error().code, result.error().message);
    }
    return Result<ConstraintId>::success(id);
}

Result<ConstraintId> Sketch::addCoincident(GeometryRef a, GeometryRef b, CoincidentPlacement placement) {
    using Added = Result<ConstraintId>;
    const ConstraintDefinition definition{ConstraintType::Coincident, {a, b}, std::nullopt, std::nullopt};
    if (placement == CoincidentPlacement::Preserve) {
        return addConstraint(definition);
    }
    if (placement != CoincidentPlacement::Midpoint) {
        return Added::failure(ErrorCode::InvalidArgument, "Unknown coincidence placement");
    }
    const auto valid = supportsConstraint(definition);
    if (!valid) {
        return Added::failure(valid.error().code, valid.error().message);
    }
    if (constraintIds_.getLast().get() == std::numeric_limits<int64_t>::max()) {
        return Added::failure(ErrorCode::BackendFailure, "Constraint ID space exhausted");
    }
    if (!capabilities().updateEntities) {
        return Added::failure(ErrorCode::Unsupported, "Coincidence placement requires point editing");
    }
    const auto first = pointPosition(a);
    const auto second = pointPosition(b);
    if (!first || !second) {
        const auto& error = !first ? first.error() : second.error();
        return Added::failure(error.code, error.message);
    }
    if (!finite(first.value()) || !finite(second.value())) {
        return Added::failure(ErrorCode::InvalidArgument, "Coincidence placement requires finite point positions");
    }
    const auto existing = constraints();
    if (!existing) {
        return Added::failure(existing.error().code, existing.error().message);
    }

    // Use public references to include transitive groups without solver handles.
    using RefKey = std::pair<EntityId, SubElement>;
    std::map<RefKey, size_t> indices;
    DSU<size_t> groups;
    const auto index = [&](GeometryRef ref) {
        const auto [it, inserted] = indices.emplace(RefKey{ref.entity, ref.sub}, indices.size());
        if (inserted) {
            groups.makeSet(it->second);
        }
        return it->second;
    };
    const size_t firstIndex = index(a);
    const size_t secondIndex = index(b);
    for (const auto& constraint : existing.value()) {
        const auto& current = constraint.definition;
        if (current.type == ConstraintType::Coincident) {
            groups.unite(index(current.refs[0]), index(current.refs[1]));
        }
    }
    const size_t firstGroup = groups.find(firstIndex);
    const size_t secondGroup = groups.find(secondIndex);
    if (firstGroup == secondGroup) {
        return addConstraint(definition);
    }

    std::optional<Vec2> firstFixed;
    std::optional<Vec2> secondFixed;
    for (const auto& constraint : existing.value()) {
        const auto& current = constraint.definition;
        if (current.type != ConstraintType::Fix) {
            continue;
        }
        const size_t group = groups.find(index(current.refs[0]));
        if (group == firstGroup && !firstFixed) {
            firstFixed = current.fixedPosition;
        }
        if (group == secondGroup && !secondFixed) {
            secondFixed = current.fixedPosition;
        }
    }
    if (firstFixed && secondFixed) {
        // Preserve both anchors, leaving any conflict for explicit solving.
        return addConstraint(definition);
    }
    const Vec2 midpoint{std::midpoint(first.value().x, second.value().x), std::midpoint(first.value().y, second.value().y)};
    const Vec2 target = firstFixed ? *firstFixed : secondFixed ? *secondFixed : midpoint;
    std::vector<PointElement> guesses;
    for (const auto& [ref, i] : indices) {
        const size_t group = groups.find(i);
        if ((group == firstGroup && !firstFixed) || (group == secondGroup && !secondFixed)) {
            guesses.push_back({{ref.first, ref.second}, target});
        }
    }
    const auto prepared = backend_->updatePointGuesses(guesses);
    if (!prepared) {
        return Added::failure(prepared.error().code, prepared.error().message);
    }
    return addConstraint(definition);
}

Status Sketch::updateConstraint(ConstraintId id, const ConstraintDefinition& definition) {
    auto current = constraint(id);
    if (!current) {
        return Status::failure(current.error().code, current.error().message);
    }
    auto valid = supportsConstraint(definition);
    if (!valid) {
        return valid;
    }
    if (!capabilities().updateConstraints) {
        return Status::failure(ErrorCode::Unsupported, "Constraint updates are unsupported");
    }
    return backend_->updateConstraint({id, definition});
}

Status Sketch::removeConstraint(ConstraintId id) {
    auto current = constraint(id);
    if (!current) {
        return Status::failure(current.error().code, current.error().message);
    }
    if (!capabilities().removeConstraints) {
        return Status::failure(ErrorCode::Unsupported, "Constraint removal is unsupported");
    }
    return backend_->removeConstraint(id);
}

Result<SketchEntity> Sketch::entity(EntityId id) const {
    if (id.get() <= 0) {
        return Result<SketchEntity>::failure(ErrorCode::InvalidArgument, "Entity ID must be positive");
    }
    return backend_->entity(id);
}
Result<SketchConstraint> Sketch::constraint(ConstraintId id) const {
    if (id.get() <= 0) {
        return Result<SketchConstraint>::failure(ErrorCode::InvalidArgument, "Constraint ID must be positive");
    }
    return backend_->constraint(id);
}
Result<std::vector<SketchEntity>> Sketch::entities() const { return backend_->entities(); }
Result<std::vector<SketchEntity>> Sketch::entities(EntityKind kind) const {
    if (!validKind(kind)) {
        return Result<std::vector<SketchEntity>>::failure(ErrorCode::InvalidArgument, "Unknown entity kind");
    }
    return backend_->entities(kind);
}
Result<Vec2> Sketch::pointPosition(GeometryRef ref) const {
    auto current = entity(ref.entity);
    if (!current) {
        return Result<Vec2>::failure(current.error().code, current.error().message);
    }
    if (!pointLike(entityKind(current.value().geometry), ref.sub)) {
        return Result<Vec2>::failure(ErrorCode::InvalidArgument, "A point-like reference is required");
    }
    return Result<Vec2>::success(detail::geometryPointPosition(current.value().geometry, ref.sub));
}
Result<std::vector<PointElement>> Sketch::pointElements() const { return pointElements(PointElementScope::All); }
Result<std::vector<PointElement>> Sketch::pointElements(PointElementScope scope) const {
    switch (scope) {
        case PointElementScope::StandalonePoints:
        case PointElementScope::CurveSubElements:
        case PointElementScope::All:
            return backend_->pointElements(scope);
        default:
            return Result<std::vector<PointElement>>::failure(ErrorCode::InvalidArgument, "Unknown point-element scope");
    }
}
Result<std::vector<PointElement>> Sketch::pointElements(EntityId id) const {
    auto current = entity(id);
    if (!current) {
        return Result<std::vector<PointElement>>::failure(current.error().code, current.error().message);
    }
    std::vector<PointElement> result;
    detail::appendPointElements(result, current.value());
    return Result<std::vector<PointElement>>::success(std::move(result));
}
Result<std::vector<SketchConstraint>> Sketch::constraints() const { return backend_->constraints(); }
size_t Sketch::entityCount() const { return backend_->entityCount(); }
Result<size_t> Sketch::entityCount(EntityKind kind) const {
    if (!validKind(kind)) {
        return Result<size_t>::failure(ErrorCode::InvalidArgument, "Unknown entity kind");
    }
    return Result<size_t>::success(backend_->entityCount(kind));
}
size_t Sketch::constraintCount() const { return backend_->constraintCount(); }

Result<SolveDiagnostics> Sketch::solve() {
    if (!capabilities().solve) {
        return Result<SolveDiagnostics>::failure(ErrorCode::Unsupported, "Solving is unsupported");
    }
    return backend_->solve();
}

Result<SolveDiagnostics> Sketch::drag(const DragRequest& request) {
    return dragPoints(std::span<const DragRequest>(&request, 1));
}

Result<SolveDiagnostics> Sketch::dragPoints(std::span<const DragRequest> requests) {
    if (requests.empty()) {
        return Result<SolveDiagnostics>::success(SolveDiagnostics{});
    }
    std::set<std::pair<EntityId, SubElement>> points;
    for (const auto& request : requests) {
        if (!finite(request.target)) {
            return Result<SolveDiagnostics>::failure(ErrorCode::InvalidArgument, "Drag target must be finite");
        }
        auto target = entity(request.point.entity);
        if (!target) {
            return Result<SolveDiagnostics>::failure(target.error().code, target.error().message);
        }
        if (!pointLike(entityKind(target.value().geometry), request.point.sub)) {
            return Result<SolveDiagnostics>::failure(ErrorCode::InvalidArgument, "Drag requires a point-like reference");
        }
        if (!points.emplace(request.point.entity, request.point.sub).second) {
            return Result<SolveDiagnostics>::failure(ErrorCode::InvalidArgument, "Duplicate drag point");
        }
    }
    if (!capabilities().drag) {
        return Result<SolveDiagnostics>::failure(ErrorCode::Unsupported, "Dragging is unsupported");
    }
    return backend_->dragPoints(requests);
}

Result<SketchSnapshot> Sketch::snapshot() const {
    auto geometry = backend_->entities();
    if (!geometry) {
        return Result<SketchSnapshot>::failure(geometry.error().code, geometry.error().message);
    }
    auto definitions = backend_->constraints();
    if (!definitions) {
        return Result<SketchSnapshot>::failure(definitions.error().code, definitions.error().message);
    }
    return Result<SketchSnapshot>::success(
        {std::move(geometry.value()), std::move(definitions.value()), entityIds_.getLast().get(), constraintIds_.getLast().get()});
}

Status Sketch::installState(const SketchSnapshot& state, BackendKind kind) {
    if (state.lastEntityId < 0 || state.lastConstraintId < 0) {
        return invalid("Negative snapshot ID watermark");
    }
    std::set<EntityId> entityIds;
    std::set<ConstraintId> constraintIds;
    for (const auto& e : state.entities) {
        if (e.id.get() <= 0 || e.id.get() > state.lastEntityId || !entityIds.insert(e.id).second) {
            return invalid("Invalid/duplicate snapshot entity ID");
        }
        auto valid = validateGeometry(e.geometry, true);
        if (!valid) {
            return valid;
        }
    }
    for (const auto& c : state.constraints) {
        if (c.id.get() <= 0 || c.id.get() > state.lastConstraintId || !constraintIds.insert(c.id).second) {
            return invalid("Invalid/duplicate snapshot constraint ID");
        }
    }
    auto candidate = create(kind);
    if (!candidate) {
        return Status::failure(candidate.error().code, candidate.error().message);
    }
    auto& staging = *candidate.value();
    for (const auto& e : state.entities) {
        if (!staging.capabilities().supports(entityKind(e.geometry))) {
            return Status::failure(ErrorCode::Unsupported, "Snapshot contains unsupported entity kind");
        }
        auto added = staging.backend_->addEntity(e);
        if (!added) {
            return added;
        }
    }
    for (const auto& c : state.constraints) {
        auto valid = staging.supportsConstraint(c.definition);
        if (!valid) {
            return valid;
        }
        auto added = staging.backend_->addConstraint(c);
        if (!added) {
            return added;
        }
    }
    backend_.swap(staging.backend_);
    entityIds_.reset(std::max(entityIds_.getLast().get(), state.lastEntityId));
    constraintIds_.reset(std::max(constraintIds_.getLast().get(), state.lastConstraintId));
    return Status::success();
}

Status Sketch::replaceState(const SketchSnapshot& state) { return installState(state, backendKind()); }

Status Sketch::switchBackend(BackendKind kind) {
    if (kind == backendKind()) {
        return Status::success();
    }
    auto state = snapshot();
    if (!state) {
        return Status::failure(state.error().code, state.error().message);
    }
    return installState(state.value(), kind);
}

}  // namespace core::sketch
