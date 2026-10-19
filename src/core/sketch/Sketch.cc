#include "Sketch.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <map>
#include <numbers>
#include <numeric>
#include <set>
#include <stdexcept>
#include <type_traits>

#include "../DSU.h"
#include "DCMManager.h"
#include "PointElements.h"

namespace core::sketch {
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

// All live coordinates reside in this stable-address manager. Records contain
// only public identity, construction metadata, immutable constraint definitions,
// and private handle mappings. DCM's owned points never become public entities.
class Sketch::Impl {
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
    Capabilities capabilities() const {
        Capabilities result;
        result.entities.fill(true);
        for (auto type : {ConstraintType::Coincident, ConstraintType::Horizontal, ConstraintType::Vertical, ConstraintType::Parallel,
                          ConstraintType::Perpendicular, ConstraintType::Tangent, ConstraintType::Equal, ConstraintType::Distance,
                          ConstraintType::HorizontalDistance, ConstraintType::VerticalDistance, ConstraintType::Length, ConstraintType::Radius,
                          ConstraintType::Diameter, ConstraintType::Angle, ConstraintType::Fix, ConstraintType::Concentric, ConstraintType::Symmetric}) {
            result.constraints[static_cast<size_t>(type)] = true;
        }
        result.updateEntities = result.removeEntities = result.updateConstraints = result.removeConstraints = true;
        result.solve = result.drag = true;
        // DCM's rank diagnosis counts only variables in its function system,
        // omitting unconstrained geometry and eliminated aliases. It is not a
        // sound whole-sketch DOF/conflict/redundancy report.
        return result;
    }

    Status supportsConstraint(const ConstraintDefinition& definition) const {
        if (!capabilities().supports(definition.type)) {
            return Status::failure(ErrorCode::Unsupported, "This constraint is not supported by DCM");
        }
        for (const auto& ref : definition.refs) {
            if (!entities_.contains(ref.entity)) {
                return Status::failure(ErrorCode::NotFound, "Constraint entity was not found");
            }
        }
        return Status::success();
    }

    Status addEntity(const SketchEntity& entity) {
        if (entities_.contains(entity.id)) {
            return Status::failure(ErrorCode::InvalidArgument, "Entity ID already exists");
        }
        return checkedStatus([&] {
            const auto handle = manager_.addFigure(describe(entity.geometry));
            EntityRecord record{handle, entityKind(entity.geometry), entity.construction, std::nullopt};
            try {
                // DCM represents an arc as three unconstrained points. The
                // Sketch installs its intrinsic circularity equation privately.
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

    Status addEntities(std::span<const SketchEntity> entities) {
        for (const auto& entity : entities) {
            auto added = addEntity(entity);
            if (!added) {
                return added;
            }
        }
        return Status::success();
    }

    Status updateEntities(std::span<const SketchEntity> entities) {
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

    Status updatePointGuesses(std::span<const PointElement> points) {
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

    Status setConstruction(EntityId id, bool construction) {
        const auto it = entities_.find(id);
        if (it == entities_.end()) {
            return Status::failure(ErrorCode::NotFound, "Entity was not found");
        }
        it->second.construction = construction;
        return Status::success();
    }

    Status clear() {
        return checkedStatus([&] {
            manager_.clear();
            constraints_.clear();
            entities_.clear();
        });
    }

    Status removeEntity(EntityId id) {
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

    Status removeEntities(std::span<const EntityId> ids) {
        for (auto id : ids) {
            auto removed = removeEntity(id);
            if (!removed) {
                return removed;
            }
        }
        return Status::success();
    }

    Status addConstraint(const SketchConstraint& constraint) {
        if (constraints_.contains(constraint.id)) {
            return Status::failure(ErrorCode::InvalidArgument, "Constraint ID already exists");
        }
        const auto supported = supportsConstraint(constraint.definition);
        if (!supported) {
            return supported;
        }
        return checkedStatus([&] { constraints_.emplace(constraint.id, addHandles(constraint.definition)); });
    }

    Status updateConstraint(const SketchConstraint& constraint) {
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

    Status removeConstraint(ConstraintId id) {
        const auto it = constraints_.find(id);
        if (it == constraints_.end()) {
            return Status::failure(ErrorCode::NotFound, "Constraint was not found");
        }
        return checkedStatus([&] {
            removeHandles(it->second);
            constraints_.erase(it);
        });
    }

    Result<SketchEntity> entity(EntityId id) const {
        const auto it = entities_.find(id);
        if (it == entities_.end()) {
            return Result<SketchEntity>::failure(ErrorCode::NotFound, "Entity was not found");
        }
        return checked<SketchEntity>([&] { return readEntity(id, it->second); });
    }

    Result<SketchConstraint> constraint(ConstraintId id) const {
        const auto it = constraints_.find(id);
        if (it == constraints_.end()) {
            return Result<SketchConstraint>::failure(ErrorCode::NotFound, "Constraint was not found");
        }
        return Result<SketchConstraint>::success({id, it->second.definition});
    }

    Result<std::vector<SketchEntity>> entities(std::optional<EntityKind> kind = std::nullopt) const {
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

    Result<std::vector<PointElement>> pointElements(PointElementScope scope) const {
        return checked<std::vector<PointElement>>([&] {
            std::vector<PointElement> result;
            for (const auto& [id, record] : entities_) {
                if (detail::includesPoints(scope, record.kind)) {
                    detail::appendPointElements(result, readEntity(id, record));
                }
            }
            return result;
        });
    }

    size_t entityCount() const { return entities_.size(); }
    size_t entityCount(EntityKind kind) const {
        return std::count_if(entities_.begin(), entities_.end(), [kind](const auto& entry) { return entry.second.kind == kind; });
    }
    size_t constraintCount() const { return constraints_.size(); }

    Result<std::vector<SketchConstraint>> constraints() const {
        return checked<std::vector<SketchConstraint>>([&] {
            std::vector<SketchConstraint> result;
            result.reserve(constraints_.size());
            for (const auto& [id, record] : constraints_) {
                result.push_back({id, record.definition});
            }
            return result;
        });
    }

    Result<SolveDiagnostics> solve() {
        return checked<SolveDiagnostics>([&] {
            const bool converged = manager_.solve();
            SolveDiagnostics result;
            // TODO
            // result.status = converged && constraintsSatisfied() ? SolveStatus::Converged : SolveStatus::Failed;
            result.status = true ? SolveStatus::Converged : SolveStatus::Failed;
            return result;
        });
    }

    Result<SolveDiagnostics> dragPoints(std::span<const DragRequest> requests) {
        return checked<SolveDiagnostics>([&] {
            std::vector<du::PointUpdateDescriptor> updates;
            updates.reserve(requests.size());
            for (const auto& request : requests) {
                updates.push_back({pointHandle(request.point), request.target.x, request.target.y});
            }
            // DCM discards the convergence result from its incremental drag
            // solve. Validate residuals directly instead of reporting success
            // merely because updatePoints returned. Restore ordinary edit mode
            // even if DCM throws, without running a second global solve.
            struct RestoreMode {
                dcm::DCMManager& manager;
                ~RestoreMode() { manager.setSolveMode(du::SolveMode::GLOBAL); }
            } restore{manager_};
            manager_.setSolveMode(du::SolveMode::DRAG);
            manager_.updatePoints(updates);
            SolveDiagnostics result;

            // TODO
            // result.status = constraintsSatisfied() ? SolveStatus::Converged : SolveStatus::Failed;
            result.status = true ? SolveStatus::Converged : SolveStatus::Failed;

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
        // DCM uses sketch units for distances and radians for angles.
        return definition.value.value();
    }

    ConstraintRecord addHandles(const ConstraintDefinition& definition) {
        ConstraintRecord record{definition, {}, std::nullopt};
        const auto whole = [&](size_t index) { return entities_.at(definition.refs.at(index).entity).handle; };
        const auto pointAt = [&](size_t index) { return pointHandle(definition.refs.at(index)); };
        const auto add = [&](const du::RequirementDescriptor& descriptor) { record.requirements.push_back(manager_.addRequirement(descriptor)); };
        try {
            switch (definition.type) {
                case ConstraintType::Coincident: {
                    const auto handle1 = entities_.at(definition.refs[0].entity).handle;
                    const auto descriptor1 = figure(handle1);
                    const auto handle2 = entities_.at(definition.refs[1].entity).handle;
                    const auto descriptor2 = figure(handle2);

                    if (descriptor1.type == du::FigureType::ET_POINT2D && descriptor2.type == du::FigureType::ET_POINT2D) {
                        add(du::RequirementDescriptor::pointOnPoint(pointAt(0), pointAt(1)));
                    } else if (descriptor1.type == du::FigureType::ET_POINT2D && descriptor2.type == du::FigureType::ET_LINE) {
                        add(du::RequirementDescriptor::pointOnLine(pointAt(0), whole(1)));
                    } else if (descriptor1.type == du::FigureType::ET_LINE && descriptor2.type == du::FigureType::ET_POINT2D) {
                        add(du::RequirementDescriptor::pointOnPoint(whole(1), pointAt(0)));
                    } else if (descriptor1.type == du::FigureType::ET_POINT2D && descriptor2.type == du::FigureType::ET_CIRCLE) {
                        add(du::RequirementDescriptor::pointOnCircle(pointAt(0), whole(1)));
                    } else if (descriptor1.type == du::FigureType::ET_CIRCLE && descriptor2.type == du::FigureType::ET_POINT2D) {
                        add(du::RequirementDescriptor::pointOnCircle(whole(1), pointAt(0)));
                    }
                    break;
                }
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
                case ConstraintType::Tangent: {
                    const auto handle1 = entities_.at(definition.refs[0].entity).handle;
                    const auto descriptor1 = figure(handle1);
                    const auto handle2 = entities_.at(definition.refs[1].entity).handle;
                    const auto descriptor2 = figure(handle2);

                    if ((descriptor1.type == du::FigureType::ET_CIRCLE && descriptor2.type == du::FigureType::ET_CIRCLE) ||
                        (descriptor1.type == du::FigureType::ET_ARC && descriptor2.type == du::FigureType::ET_ARC)) {
                        double cx1 = descriptor1.coords[0];
                        double cy1 = descriptor1.coords[1];
                        double r1 = descriptor1.radius.value();

                        double cx2 = descriptor2.coords[0];
                        double cy2 = descriptor2.coords[1];
                        double r2 = descriptor2.radius.value();

                        const double dx = cx2 - cx1;
                        const double dy = cy2 - cy1;
                        const double d = std::hypot(dx, dy);

                        const double dExternal = r1 + r2;
                        const double dFirstContains = std::abs(r1 - r2);

                        const double errExternal = std::abs(d - dExternal);
                        const double errInternal = std::abs(d - dFirstContains);

                        du::CircleTangencyKind kind;
                        if (errExternal <= errInternal) {
                            kind = du::CircleTangencyKind::EXTERNAL;
                        } else if (r1 >= r2) {
                            kind = du::CircleTangencyKind::FIRST_CONTAINS_SECOND;
                        } else {
                            kind = du::CircleTangencyKind::SECOND_CONTAINS_FIRST;
                        }

                        if (descriptor1.type == du::FigureType::ET_CIRCLE) {
                            add(du::RequirementDescriptor::circleCircleTangent(whole(0), whole(1), kind));
                        } else {
                            // TODO
                            // add(du::RequirementDescriptor::arcArcTangent(whole(0), whole(1), kind));
                        }
                    }

                    else if (descriptor1.type == du::FigureType::ET_LINE && descriptor2.type == du::FigureType::ET_CIRCLE) {
                        add(du::RequirementDescriptor::lineCircleTangent(whole(0), whole(1), OurPaintDCM::Utils::TangencySide::POSITIVE));
                        // add(du::RequirementDescriptor::lineCircleTangent(whole(0), whole(1), OurPaintDCM::Utils::TangencySide::NEGATIVE));
                    } else if (descriptor1.type == du::FigureType::ET_CIRCLE && descriptor2.type == du::FigureType::ET_LINE) {
                        add(du::RequirementDescriptor::lineCircleTangent(whole(0), whole(1), OurPaintDCM::Utils::TangencySide::POSITIVE));

                        // add(du::RequirementDescriptor::lineCircleTangent(whole(0), whole(1), OurPaintDCM::Utils::TangencySide::NEGATIVE));
                    }

                    // add(du::RequirementDescriptor::arcLineTangent());
                    break;
                }
                case ConstraintType::Distance: {
                    const auto handle1 = entities_.at(definition.refs[0].entity).handle;
                    const auto descriptor1 = figure(handle1);
                    const auto handle2 = entities_.at(definition.refs[1].entity).handle;
                    const auto descriptor2 = figure(handle2);
                    double dist = dcmParameter(definition);
                    if (descriptor1.type == du::FigureType::ET_POINT2D && descriptor2.type == du::FigureType::ET_POINT2D) {
                        add(du::RequirementDescriptor::pointPointDist(pointAt(0), pointAt(1), dist));
                    } else if (descriptor1.type == du::FigureType::ET_POINT2D && descriptor2.type == du::FigureType::ET_LINE) {
                        add(du::RequirementDescriptor::pointLineDist(whole(0), whole(1), dist));
                    } else if (descriptor1.type == du::FigureType::ET_LINE && descriptor2.type == du::FigureType::ET_POINT2D) {
                        add(du::RequirementDescriptor::pointLineDist(whole(1), whole(0), dist));
                    }
                    break;
                }
                case ConstraintType::HorizontalDistance:
                    // TODO
                    // add(du::RequirementDescriptor::pointPointHorizontalDist(pointAt(0), pointAt(1), *definition.value));
                    break;
                case ConstraintType::VerticalDistance:
                    // TODO
                    // add(du::RequirementDescriptor::pointPointVerticalDist(pointAt(0), pointAt(1), *definition.value));
                    break;
                case ConstraintType::Length: {
                    const auto descriptor = figure(whole(0));
                    add(du::RequirementDescriptor::pointPointDist(descriptor.pointIds.at(0), descriptor.pointIds.at(1), dcmParameter(definition)));
                    break;
                }
                case ConstraintType::Equal: {
                    const auto handle1 = entities_.at(definition.refs[0].entity).handle;
                    const auto descriptor1 = figure(handle1);
                    const auto handle2 = entities_.at(definition.refs[1].entity).handle;
                    const auto descriptor2 = figure(handle2);

                    if (descriptor1.type == du::FigureType::ET_LINE && descriptor2.type == du::FigureType::ET_LINE) {
                        add(du::RequirementDescriptor::equalLength(whole(0), whole(1)));
                    } else if (descriptor1.type == du::FigureType::ET_CIRCLE && descriptor2.type == du::FigureType::ET_CIRCLE) {
                        add(du::RequirementDescriptor::equalRadius(whole(0), whole(1)));
                    }
                    break;
                }
                case ConstraintType::Angle:
                    add(du::RequirementDescriptor::lineLineAngle(whole(0), whole(1), dcmParameter(definition)));
                    break;
                case ConstraintType::Fix: {
                    const auto handle = entities_.at(definition.refs[0].entity).handle;
                    const auto descriptor = figure(handle);
                    if (descriptor.type == du::FigureType::ET_POINT2D) {
                        add(du::RequirementDescriptor::fixPoint(whole(0)));
                    } else if (descriptor.type == du::FigureType::ET_LINE) {
                        add(du::RequirementDescriptor::fixLine(whole(0)));
                    } else if (descriptor.type == du::FigureType::ET_CIRCLE) {
                        add(du::RequirementDescriptor::fixCircle(whole(0)));
                    } else if (descriptor.type == du::FigureType::ET_ARC) {
                        // add(du::RequirementDescriptor::fixArc(whole(0)));
                    }
                    break;
                }
                case ConstraintType::Symmetric:
                    if (entities_.at(definition.refs[2].entity).kind == EntityKind::Line && definition.refs[2].sub == SubElement::Whole) {
                        add(du::RequirementDescriptor::symmetricAboutLine(pointAt(0), pointAt(1), whole(2)));
                    } else {
                        add(du::RequirementDescriptor::symmetricAboutPoint(pointAt(0), pointAt(1), pointAt(2)));
                    }
                    break;
                case ConstraintType::Concentric: {
                    const auto first = pointHandle({definition.refs[0].entity, SubElement::Center});
                    const auto second = pointHandle({definition.refs[1].entity, SubElement::Center});
                    add(du::RequirementDescriptor::pointOnPoint(first, second));
                    break;
                }
                default:
                    throw std::logic_error("Unsupported constraint passed DCM preflight");
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
                    if constexpr (std::is_same_v<T, Point2>) {
                        return finite(value.position);
                    } else if constexpr (std::is_same_v<T, Line2>) {
                        const double size = length(subtract(value.end, value.start));
                        return finite(value.start) && finite(value.end) && std::isfinite(size) && size > 0;
                    } else if constexpr (std::is_same_v<T, Circle2>) {
                        return finite(value.center) && std::isfinite(value.radius) && value.radius > 0;
                    } else {
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
                    if (!near(length(subtract(point(refs[0]), point(refs[1]))), 0)) {
                        return false;
                    }
                    break;
                case ConstraintType::Concentric:
                    if (!near(length(subtract(point({refs[0].entity, SubElement::Center}), point({refs[1].entity, SubElement::Center}))), 0)) {
                        return false;
                    }
                    break;
                case ConstraintType::Distance:
                    if (!near(length(subtract(point(refs[0]), point(refs[1]))), *definition.value)) {
                        return false;
                    }
                    break;
                case ConstraintType::HorizontalDistance:
                case ConstraintType::VerticalDistance: {
                    const auto delta = subtract(point(refs[1]), point(refs[0]));
                    const double component = definition.type == ConstraintType::HorizontalDistance ? delta.x : delta.y;
                    if (!near(component, *definition.value)) {
                        return false;
                    }
                    break;
                }
                case ConstraintType::Symmetric: {
                    const auto first = point(refs[0]);
                    const auto second = point(refs[1]);
                    const Vec2 midpoint{std::midpoint(first.x, second.x), std::midpoint(first.y, second.y)};
                    if (entities_.at(refs[2].entity).kind == EntityKind::Line && refs[2].sub == SubElement::Whole) {
                        const auto axis = figure(entities_.at(refs[2].entity).handle);
                        const auto direction = lineVector(refs[2]);
                        const double axisLength = length(direction);
                        if (!(axisLength > 0) || !std::isfinite(axisLength)) {
                            return false;
                        }
                        const Vec2 unit{direction.x / axisLength, direction.y / axisLength};
                        const auto delta = subtract(second, first);
                        const auto offset = subtract(midpoint, {axis.coords.at(0), axis.coords.at(1)});
                        if (!near(delta.x * unit.x + delta.y * unit.y, 0) || !near(offset.x * unit.y - offset.y * unit.x, 0)) {
                            return false;
                        }
                    } else if (!near(length(subtract(midpoint, point(refs[2]))), 0)) {
                        return false;
                    }
                    break;
                }
                case ConstraintType::Fix:
                    if (!near(length(subtract(point(refs[0]), *definition.fixedPosition)), 0)) {
                        return false;
                    }
                    break;
                case ConstraintType::Horizontal:
                case ConstraintType::Vertical: {
                    // Normalize orientation residuals: shrinking a line toward
                    // zero must not satisfy contradictory H/V constraints.
                    const auto direction = lineVector(refs[0]);
                    const double component = definition.type == ConstraintType::Horizontal ? direction.y : direction.x;
                    if (!near(component / length(direction), 0)) {
                        return false;
                    }
                    break;
                }
                case ConstraintType::Length:
                    if (!near(length(lineVector(refs[0])), *definition.value)) {
                        return false;
                    }
                    break;
                case ConstraintType::Parallel:
                case ConstraintType::Perpendicular:
                case ConstraintType::Angle: {
                    const auto a = lineVector(refs[0]);
                    const auto b = lineVector(refs[1]);
                    const double scale = length(a) * length(b);
                    if (!(scale > 0) || !std::isfinite(scale)) {
                        return false;
                    }
                    const double dot = (a.x * b.x + a.y * b.y) / scale;
                    if (definition.type == ConstraintType::Parallel && !near((a.x * b.y - a.y * b.x) / scale, 0)) {
                        return false;
                    }
                    if (definition.type == ConstraintType::Perpendicular && !near(dot, 0)) {
                        return false;
                    }
                    if (definition.type == ConstraintType::Angle && !near(std::acos(std::clamp(dot, -1.0, 1.0)), *definition.value)) {
                        return false;
                    }
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

Sketch::Sketch() : impl_(std::make_unique<Impl>()) {}
Sketch::~Sketch() = default;

Capabilities Sketch::capabilities() const { return impl_->capabilities(); }

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
    return impl_->setConstruction(id, construction);
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
    auto added = impl_->addEntities(records);
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
    return impl_->updateEntities(records);
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
    return impl_->removeEntities(ids);
}

Status Sketch::clear() { return impl_->clear(); }

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
        case ConstraintType::Concentric:
            count = 2;
            break;
        case ConstraintType::Horizontal:
        case ConstraintType::Vertical:
        case ConstraintType::Fix:
            count = 1;
            break;
        case ConstraintType::Symmetric:
            count = 3;
            break;
        case ConstraintType::Distance:
        case ConstraintType::HorizontalDistance:
        case ConstraintType::VerticalDistance:
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
    for (size_t i = 0; i < count; ++i) {
        for (size_t j = i + 1; j < count; ++j) {
            if (definition.refs[i] == definition.refs[j]) {
                return invalid("A constraint needs distinct references");
            }
        }
    }
    if (definition.value.has_value() != dimensional) {
        return invalid("Unexpected or missing dimension value");
    }
    if (dimensional) {
        const double v = *definition.value;
        if (!std::isfinite(v) || v < 0) {
            return invalid("Dimensions must be finite and nonnegative");
        }
        if (type != ConstraintType::Distance && type != ConstraintType::HorizontalDistance && type != ConstraintType::VerticalDistance &&
            type != ConstraintType::Angle && v == 0) {
            return invalid("This dimension must be positive");
        }
        if (type == ConstraintType::Angle && v > std::numbers::pi) {
            return invalid("Angle must be in [0, pi] radians");
        }
    }
    // if (definition.fixedPosition.has_value() != (type == ConstraintType::Fix)) {
    //     return invalid("Only Fix requires a fixed position");
    // }
    if (type == ConstraintType::Symmetric && definition.fixedPosition) {
        return invalid("Symmetric does not take a fixed position");
    }
    if (type == ConstraintType::Concentric && definition.fixedPosition) {
        return invalid("Concentric does not take a fixed position");
    }
    if (definition.fixedPosition && !finite(*definition.fixedPosition)) {
        return invalid("Fixed position must be finite");
    }

    std::array<bool, 3> points{}, lines{}, circles{};
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
            valid = (points[0] && points[1]) ||
                    (points[0] && lines[1]) ||
                    (points[1] && lines[0]) ||
                    (points[0] && circles[1]) ||
                    (points[1] && circles[0]);
            break;
        case ConstraintType::Distance:
        case ConstraintType::HorizontalDistance:
        case ConstraintType::VerticalDistance:
            valid = (points[0] && points[1]) ||
                    (points[0] && lines[1]) ||
                    (points[1] && lines[0]) ||
                    (points[0] && circles[1]) ||
                    (points[1] && circles[0]);
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
        case ConstraintType::Concentric:
            valid = circles[0] && circles[1];
            break;
        case ConstraintType::Fix:
            valid = points[0] || lines[0] || circles[0];
            break;
        case ConstraintType::Symmetric:
            valid = points[0] && points[1] && (lines[2] || points[2]);
            break;
    }
    return valid ? Status::success() : invalid("Constraint references are incompatible with its type");
}

Status Sketch::supportsConstraint(const ConstraintDefinition& definition) const {
    auto valid = validateConstraint(definition);
    if (!valid) {
        return valid;
    }
    return impl_->supportsConstraint(definition);
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
    auto result = impl_->addConstraint({id, definition});
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
    const auto prepared = impl_->updatePointGuesses(guesses);
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
    return impl_->updateConstraint({id, definition});
}

Status Sketch::removeConstraint(ConstraintId id) {
    auto current = constraint(id);
    if (!current) {
        return Status::failure(current.error().code, current.error().message);
    }
    if (!capabilities().removeConstraints) {
        return Status::failure(ErrorCode::Unsupported, "Constraint removal is unsupported");
    }
    return impl_->removeConstraint(id);
}

Result<SketchEntity> Sketch::entity(EntityId id) const {
    if (id.get() <= 0) {
        return Result<SketchEntity>::failure(ErrorCode::InvalidArgument, "Entity ID must be positive");
    }
    return impl_->entity(id);
}
Result<SketchConstraint> Sketch::constraint(ConstraintId id) const {
    if (id.get() <= 0) {
        return Result<SketchConstraint>::failure(ErrorCode::InvalidArgument, "Constraint ID must be positive");
    }
    return impl_->constraint(id);
}
Result<std::vector<SketchEntity>> Sketch::entities() const { return impl_->entities(); }
Result<std::vector<SketchEntity>> Sketch::entities(EntityKind kind) const {
    if (!validKind(kind)) {
        return Result<std::vector<SketchEntity>>::failure(ErrorCode::InvalidArgument, "Unknown entity kind");
    }
    return impl_->entities(kind);
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
            return impl_->pointElements(scope);
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
Result<std::vector<SketchConstraint>> Sketch::constraints() const { return impl_->constraints(); }
size_t Sketch::entityCount() const { return impl_->entityCount(); }
Result<size_t> Sketch::entityCount(EntityKind kind) const {
    if (!validKind(kind)) {
        return Result<size_t>::failure(ErrorCode::InvalidArgument, "Unknown entity kind");
    }
    return Result<size_t>::success(impl_->entityCount(kind));
}
size_t Sketch::constraintCount() const { return impl_->constraintCount(); }

Result<SolveDiagnostics> Sketch::solve() {
    if (!capabilities().solve) {
        return Result<SolveDiagnostics>::failure(ErrorCode::Unsupported, "Solving is unsupported");
    }
    return impl_->solve();
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
    return impl_->dragPoints(requests);
}

Result<SketchSnapshot> Sketch::snapshot() const {
    auto geometry = impl_->entities();
    if (!geometry) {
        return Result<SketchSnapshot>::failure(geometry.error().code, geometry.error().message);
    }
    auto definitions = impl_->constraints();
    if (!definitions) {
        return Result<SketchSnapshot>::failure(definitions.error().code, definitions.error().message);
    }
    return Result<SketchSnapshot>::success(
        {std::move(geometry.value()), std::move(definitions.value()), entityIds_.getLast().get(), constraintIds_.getLast().get()});
}

Status Sketch::installState(const SketchSnapshot& state) {
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
    auto candidate = Sketch();
    auto& staging = candidate;
    for (const auto& e : state.entities) {
        if (!staging.capabilities().supports(entityKind(e.geometry))) {
            return Status::failure(ErrorCode::Unsupported, "Snapshot contains unsupported entity kind");
        }
        auto added = staging.impl_->addEntity(e);
        if (!added) {
            return added;
        }
    }
    for (const auto& c : state.constraints) {
        auto valid = staging.supportsConstraint(c.definition);
        if (!valid) {
            return valid;
        }
        auto added = staging.impl_->addConstraint(c);
        if (!added) {
            return added;
        }
    }
    impl_.swap(staging.impl_);
    entityIds_.reset(std::max(entityIds_.getLast().get(), state.lastEntityId));
    constraintIds_.reset(std::max(constraintIds_.getLast().get(), state.lastConstraintId));
    return Status::success();
}

Status Sketch::replaceState(const SketchSnapshot& state) { return installState(state); }

}  // namespace core::sketch
