#ifndef OURPAINT_CORE_SKETCH_TYPES_H_
#define OURPAINT_CORE_SKETCH_TYPES_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../objects/ID.h"

namespace core::sketch {

// Typed wrappers reuse Core's ID value semantics. Zero is invalid. IDs are local
// to a Sketch lineage, never reused, and survive snapshot replacement.
template <class Tag>
class SketchId {
public:
    explicit SketchId(int64_t value = 0) : value_(value) {}
    int64_t get() const { return value_.get(); }
    friend bool operator==(SketchId a, SketchId b) { return a.value_ == b.value_; }
    friend bool operator<(SketchId a, SketchId b) { return a.value_ < b.value_; }

private:
    core::ID value_;
};
struct EntityTag;
struct ConstraintTag;
using EntityId = SketchId<EntityTag>;
using ConstraintId = SketchId<ConstraintTag>;

// Domain/capability errors are values. Convergence and backend failures differ.
enum class ErrorCode {
    Unsupported,
    InvalidArgument,
    NotFound,
    SolveFailure,
    BackendFailure
};
struct SketchError {
    ErrorCode code;
    std::string message;
};
template <class T>
class Result {
public:
    static Result success(T value) { return Result(std::move(value)); }
    static Result failure(ErrorCode code, std::string message) { return Result(SketchError{code, std::move(message)}); }
    explicit operator bool() const { return std::holds_alternative<T>(data_); }
    T& value() { return std::get<T>(data_); }
    const T& value() const { return std::get<T>(data_); }
    const SketchError& error() const { return std::get<SketchError>(data_); }

private:
    explicit Result(T value) : data_(std::move(value)) {}
    explicit Result(SketchError error) : data_(std::move(error)) {}
    std::variant<T, SketchError> data_;
};
template <>
class Result<void> {
public:
    static Result success() { return Result(std::nullopt); }
    static Result failure(ErrorCode code, std::string message) { return Result(SketchError{code, std::move(message)}); }
    explicit operator bool() const { return !error_; }
    const SketchError& error() const { return error_.value(); }

private:
    explicit Result(std::optional<SketchError> error) : error_(std::move(error)) {}
    std::optional<SketchError> error_;
};
using Status = Result<void>;

// Coordinates/lengths use one caller-selected consistent unit; angles use radians.
struct Vec2 {
    double x = 0;
    double y = 0;
    bool operator==(const Vec2&) const = default;
};
struct Point2 {
    Vec2 position;
};
struct Line2 {
    Vec2 start;
    Vec2 end;
};
struct Circle2 {
    Vec2 center;
    double radius;
};
// Counterclockwise start->end sweep around center, strictly between zero and one
// full turn. Creation/update require equal positive radii; an unsolved query may
// contain unequal radii until the backend satisfies the intrinsic arc equation.
struct Arc2 {
    Vec2 center;
    Vec2 start;
    Vec2 end;
};
using SketchGeometry = std::variant<Point2, Line2, Circle2, Arc2>;
enum class EntityKind {
    Point,
    Line,
    Circle,
    Arc
};
inline EntityKind entityKind(const SketchGeometry& geometry) {
    return static_cast<EntityKind>(geometry.index());
}

// Detached query value. Owned solver points are not public entities.
// Construction affects future consumers only, not constraint equations.
struct SketchEntity {
    EntityId id;
    SketchGeometry geometry;
    bool construction = false;
};
// Inputs own neutral values; batch calls borrow only the span for their duration.
struct EntityCreation {
    SketchGeometry geometry;
    bool construction = false;
};
struct EntityUpdate {
    EntityId id;
    SketchGeometry geometry;
    // Omission preserves the current construction flag.
    std::optional<bool> construction = std::nullopt;
};
enum class SubElement {
    Whole, Start, End, Center
};
struct GeometryRef {
    EntityId entity;
    SubElement sub = SubElement::Whole;
    bool operator==(const GeometryRef&) const = default;
};
struct PointElement {
    GeometryRef ref;
    Vec2 position;
};
enum class PointElementScope { StandalonePoints, CurveSubElements, All };
// Midpoint prepares a symmetric initial guess; Preserve uses ordinary insertion.
enum class CoincidentPlacement { Preserve, Midpoint };
enum class ConstraintType {
    Coincident,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Tangent,
    Equal,
    Distance,
    HorizontalDistance,
    VerticalDistance,
    Length,
    Radius,
    Diameter,
    Angle,
    Fix,
    Concentric,
    Symmetric,
    Midpoint
};

// Positive/negative sides refer to the left/right of Line Start -> End.
// Containment follows the order of the two circular references.
enum class TangencyBranch { PositiveSide, NegativeSide, External, FirstContainsSecond, SecondContainsFirst };

// Coincident/Distance use two point-like refs: Point Whole, Line Start/End,
// Circle Center, Arc Start/End/Center. Horizontal/Vertical/Length use one whole
// line. Parallel/Perpendicular/Angle use two whole lines. Equal uses two lines or
// two circular curves; Radius/Diameter use one circular curve. Tangent uses a
// line and circular curve or two circular curves, on their full supports.
// Angle is unsigned [0, pi] between directed lines. Length/Radius/Diameter are
// positive; Distance is nonnegative. Only dimensional kinds take value.
// HorizontalDistance/VerticalDistance use two point-like refs and constrain
// the second coordinate minus the first along X/Y to a nonnegative value.
// Fix uses one point-like ref and explicit fixedPosition, preserving its target
// across edits, failed solves and snapshot replacement.
// Symmetric uses three distinct refs: two point-like refs followed by a whole
// line axis or a point-like center. It takes neither value nor fixedPosition.
// Concentric uses two distinct whole Circle/Arc refs in any combination,
// equating their centers. It takes neither value nor fixedPosition.
// Midpoint uses a point-like ref followed by a whole finite line segment,
// constraining the point to (start + end) / 2. No value or fixedPosition.
// Curve midpoint measures are not implied by this segment-only relation.
struct ConstraintDefinition {
    ConstraintType type = ConstraintType::Coincident;
    std::vector<GeometryRef> refs;
    std::optional<double> value;
    std::optional<Vec2> fixedPosition;
    // Tangent only. Omission selects the nearest admissible branch on creation;
    // stored/query/snapshot definitions retain the resolved branch explicitly.
    // An update with unchanged refs and no branch preserves the previous one.
    std::optional<TangencyBranch> tangencyBranch = std::nullopt;
};
struct SketchConstraint {
    ConstraintId id;
    ConstraintDefinition definition;
};

// Type support is coarse; supportsConstraint checks the actual ref combination.
// Unsupported operations never mutate the model.
struct Capabilities {
    std::array<bool, 4> entities{};
    std::array<bool, 18> constraints{};
    bool updateEntities = false;
    bool removeEntities = false;
    bool updateConstraints = false;
    bool removeConstraints = false;
    bool solve = false;
    bool drag = false;
    bool degreesOfFreedom = false;
    bool conflictingConstraints = false;
    bool redundantConstraints = false;
    bool supports(EntityKind kind) const {
        const auto i = static_cast<size_t>(kind);
        return i < entities.size() && entities[i];
    }
    bool supports(ConstraintType type) const {
        const auto i = static_cast<size_t>(type);
        return i < constraints.size() && constraints[i];
    }
};
enum class SolveStatus { Converged, Failed };
// Missing diagnostics mean unavailable, not zero/an empty diagnosis. A failed
// solve may expose its current iterate; inspect status before treating geometry
// as solved. Constraint IDs always refer to the public model.
struct SolveDiagnostics {
    SolveStatus status = SolveStatus::Converged;
    std::optional<int> degreesOfFreedom;
    std::optional<std::vector<ConstraintId>> conflictingConstraints;
    std::optional<std::vector<ConstraintId>> redundantConstraints;
};
// One synchronous best-effort drag step of a point-like ref. Target is a solver
// preference, not a persistent constraint or a guaranteed resulting coordinate.
struct DragRequest {
    GeometryRef point;
    Vec2 target;
};
enum class TangencyDragPolicy { Preserve, SwitchLineSide };
struct DragOptions {
    TangencyDragPolicy tangency = TangencyDragPolicy::Preserve;
    // Sketch units. A requested center must cross this distance beyond the line
    // before changing side. Ambiguous/degenerate proposed lines retain the branch.
    double sideSwitchTolerance = 0;
};
// Detached O(N) transfer value, never a second live model. Contains unsolved
// current geometry, explicit targets and ID watermarks, but no solver caches.
struct SketchSnapshot {
    std::vector<SketchEntity> entities;
    std::vector<SketchConstraint> constraints;
    int64_t lastEntityId = 0;
    int64_t lastConstraintId = 0;
};
}  // namespace core::sketch

namespace std {
template <>
struct hash<core::sketch::EntityId> {
    std::size_t operator()(const core::sketch::EntityId &id) const {
        return hash<int64_t>()(id.get());
    }
};
}


#endif
