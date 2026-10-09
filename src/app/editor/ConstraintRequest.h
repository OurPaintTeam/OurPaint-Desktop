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
    Angle,
    Unsupported
};

struct ConstraintRequest {
    ConstraintAction action = ConstraintAction::Coincident;
    // Lengths use sketch units; angles use radians.
    std::optional<double> value;
};

#endif
