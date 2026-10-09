#ifndef OURPAINT_APPLICATION_ACTION_REPORT_H_
#define OURPAINT_APPLICATION_ACTION_REPORT_H_

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

#include "../../core/sketch/SketchTypes.h"

enum class ActionKind { CreatePoint, CreateLine, CreateCircle, ApplyConstraint, Delete, Paste, Drag, SwitchBackend };
enum class ModelChange { Unchanged, Changed, PotentiallyChanged };
enum class ActionRejection { None, InvalidSelection, InvalidValue, Unsupported, InsufficientTargets, InvalidGeometry };

// Detached application outcome. IDs describe confirmed operations only; errors
// and solve diagnostics retain their Core meaning without UI text or objects.
struct ActionReport {
    ActionKind action;
    ModelChange change = ModelChange::Unchanged;
    size_t requestedEntities = 0;
    size_t requestedConstraints = 0;
    std::vector<core::sketch::EntityId> entityIds{};
    std::vector<core::sketch::ConstraintId> constraintIds{};
    ActionRejection rejection = ActionRejection::None;
    std::optional<core::sketch::SketchError> operationError = std::nullopt;
    std::optional<core::sketch::SketchError> solveError = std::nullopt;
    std::optional<core::sketch::SolveDiagnostics> solve = std::nullopt;
    std::optional<core::sketch::BackendKind> backend = std::nullopt;

    // Ordinary Sketch mutations promise no rollback for backend/numerical errors.
    // Read-only preparation and staged backend switching must explicitly retain
    // Unchanged on failure instead of using this helper.
    void mutationFailed(const core::sketch::SketchError& error) {
        operationError = error;
        if (error.code == core::sketch::ErrorCode::BackendFailure || error.code == core::sketch::ErrorCode::SolveFailure) {
            change = ModelChange::PotentiallyChanged;
        }
    }

    void recordSolve(const core::sketch::Result<core::sketch::SolveDiagnostics>& result) {
        if (result) {
            solve = result.value();
            solveError.reset();
        } else {
            solve.reset();
            solveError = result.error();
        }
    }

    static ActionReport creation(ActionKind action, const core::sketch::Result<core::sketch::EntityId>& result) {
        ActionReport report{action};
        report.requestedEntities = 1;
        if (result) {
            report.change = ModelChange::Changed;
            report.entityIds.push_back(result.value());
        } else {
            report.mutationFailed(result.error());
        }
        return report;
    }
};

struct ToolCancellation {
    bool handled = false;
    std::optional<ActionReport> report;
};

#endif
