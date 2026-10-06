#ifndef OURPAINT_APPLICATION_CONSTRAINT_ACTIONS_H_
#define OURPAINT_APPLICATION_CONSTRAINT_ACTIONS_H_

#include <optional>
#include <span>

#include "../../core/sketch/SketchTypes.h"
#include "ConstraintRequest.h"

namespace core::sketch {
class Sketch;
}

struct ConstraintPreparation {
    enum class State { Ready, NeedsMoreInput, InvalidSelection, Unsupported };

    State state = State::InvalidSelection;
    std::optional<core::sketch::ConstraintDefinition> definition;
};

class ConstraintActions {
public:
    explicit ConstraintActions(core::sketch::Sketch& sketch);

    ConstraintPreparation prepare(const ConstraintRequest& request, std::span<const core::sketch::GeometryRef> refs) const;
    bool apply(const core::sketch::ConstraintDefinition& definition);

private:
    core::sketch::Sketch& sketch_;
};

#endif
