#ifndef OURPAINT_CORE_SKETCH_H_
#define OURPAINT_CORE_SKETCH_H_

#include <unordered_map>
#include <memory>
#include <variant>

class EntityId;
class Vec2;
class ConstraintId;
class SketchConstraint;
class SolveResult;
class DragRequest;
class IdGenerator;
class ISketchSolver;

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

struct Arc2 {
    Vec2 center;
    Vec2 start;
    Vec2 end;
};

using SketchGeometry =
    std::variant<Point2, Line2, Circle2, Arc2>;

struct SketchEntity {
    EntityId id;
    SketchGeometry geometry;

    bool construction = false;
};

enum class SubElement {
    Whole,
    Start,
    End,
    Center
};

struct GeometryRef {
    EntityId entity;
    SubElement sub = SubElement::Whole;
};

enum class ConstraintType {
    Coincident,

    Horizontal,
    Vertical,

    Parallel,
    Perpendicular,
    Tangent,
    Equal,

    Distance,
    Length,
    Radius,
    Diameter,
    Angle,

    Fix
};

struct SketchConstraint {
    ConstraintId id;

    ConstraintType type;

    std::array<GeometryRef, 4> refs {};
    std::uint8_t refCount = 0;

    std::optional<double> value;
};

class Sketch {
public:
    EntityId addPoint(Vec2 p);
    EntityId addLine(Vec2 a, Vec2 b);
    EntityId addCircle(Vec2 center, double radius);
    EntityId addArc(Vec2 center, Vec2 start, Vec2 end);

    bool removeEntity(EntityId id);

    ConstraintId addConstraint(const SketchConstraint& constraint);
    bool removeConstraint(ConstraintId id);

    const SketchEntity* entity(EntityId id) const;
    const SketchConstraint* constraint(ConstraintId id) const;

    SolveResult solve();
    SolveResult drag(const DragRequest& request);

private:
    std::unordered_map<EntityId, SketchEntity> entities_;
    std::unordered_map<ConstraintId, SketchConstraint> constraints_;

    std::unique_ptr<ISketchSolver> solver_;

    IdGenerator<EntityId> entityIds_;
    IdGenerator<ConstraintId> constraintIds_;
};

#endif  // OURPAINT_CORE_SKETCH_H_
