#ifndef OURPAINT_APPLICATION_CONSTRAINT_REQUEST_H_
#define OURPAINT_APPLICATION_CONSTRAINT_REQUEST_H_

#include <optional>

struct ConstraintRequest {
    core::sketch::ConstraintType type;
    std::optional<double> value = std::nullopt;     // Lengths use Sketch units; angles use radians.
};

#endif
