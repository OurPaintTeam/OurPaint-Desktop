#include "ConstraintTool.h"

#include <utility>

#include "../../viewport/OverlayModel.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "../ConstraintActions.h"

ConstraintTool::ConstraintTool(ConstraintActions& actions, Cpu2dPicker& picker, OverlayModel& overlay)
    : actions_(actions), picker_(picker), overlay_(overlay) {}

void ConstraintTool::begin(const ConstraintRequest& request, std::span<const core::sketch::GeometryRef> initialRefs) {
    request_ = request;
    refs_.assign(initialRefs.begin(), initialRefs.end());
    overlay_.constraintRefs_ = refs_;
}

std::optional<ActionReport> ConstraintTool::onMouseMove(const input::MouseMoveEvent& e) {
    (void)e;
    return std::nullopt;
}

std::optional<ActionReport> ConstraintTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button != input::MouseButton::Left || e.action != input::MouseButtonAction::Press) {
        return std::nullopt;
    }

    std::optional<PickResult> pick;
    switch (request_.action) {
        case ConstraintAction::Horizontal:
        case ConstraintAction::Vertical:
        case ConstraintAction::Parallel:
        case ConstraintAction::Perpendicular:
            pick = picker_.pickLineAtScreenLogical(e.x, e.y);
            break;
        case ConstraintAction::Coincident:
        case ConstraintAction::Fix:
            pick = picker_.pickPointAtScreenLogical(e.x, e.y);
            break;
        case ConstraintAction::Equal:
        case ConstraintAction::Tangent:
            pick = picker_.pickCurveAtScreenLogical(e.x, e.y);
            break;
        default:
            pick = picker_.pickAtScreenLogical(e.x, e.y);
            break;
    }
    if (!pick) {
        return std::nullopt;
    }

    return handleRef(pick->ref);
}

std::optional<ActionReport> ConstraintTool::onKey(const input::KeyEvent& e) {
    (void)e;
    return std::nullopt;
}

ToolCancellation ConstraintTool::cancel() {
    const bool handled = !refs_.empty();
    resetInputs();
    return {handled, std::nullopt};
}

std::optional<ActionReport> ConstraintTool::handleRef(const core::sketch::GeometryRef& ref) {
    auto candidateRefs = refs_;
    candidateRefs.push_back(ref);
    const auto preparation = actions_.prepare(request_, candidateRefs);

    switch (preparation.state) {
        case ConstraintPreparation::State::NeedsMoreInput:
            refs_ = std::move(candidateRefs);
            overlay_.constraintRefs_ = refs_;
            return std::nullopt;
        case ConstraintPreparation::State::Ready: {
            if (!preparation.definition) {
                return ConstraintActions::rejectedReport(preparation);
            }
            auto report = actions_.apply(*preparation.definition);
            // A failed solve does not undo insertion. Unknown partial edits must
            // also release pending input so a retry cannot insert it twice.
            if (report.change != ModelChange::Unchanged) {
                resetInputs();
            }
            return report;
        }
        case ConstraintPreparation::State::InvalidSelection:
        case ConstraintPreparation::State::Unsupported:
            return ConstraintActions::rejectedReport(preparation);
    }
    return std::nullopt;
}

void ConstraintTool::resetInputs() {
    refs_.clear();
    overlay_.constraintRefs_.clear();
}
