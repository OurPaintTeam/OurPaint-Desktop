#ifndef OURPAINT_APPLICATION_CONSTRAINT_REQUEST_H_
#define OURPAINT_APPLICATION_CONSTRAINT_REQUEST_H_

#include <optional>

enum class ConstraintAction {
    Coincident,
    Horizontal,
    Vertical,
    Parallel,
    Perpendicular,
    Tangent,
    Equal,
    Fix,
    Dimension,
    HVDistance,
    Angle,
    // Select two whole circles or arcs, in either order.
    Concentric,
    Collinear,
    // Select a point-like element and a whole line in either order.
    Midpoint,
    // Select two points and an axis line in any order, or three points with the center last.
    Symmetric,
    Unsupported
};

struct ConstraintRequest {
    ConstraintAction action = ConstraintAction::Coincident;
    // Lengths use sketch units; angles use radians.
    std::optional<double> value;
};

#endif
