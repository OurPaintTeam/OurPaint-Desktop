#include "DimensionTool.h"

#include <utility>

#include "../../viewport/OverlayModel.h"
#include "../../viewport/picking/Cpu2dPicker.h"
#include "../ConstraintActions.h"

DimensionTool::DimensionTool(ConstraintActions& actions, Cpu2dPicker& picker, OverlayModel& overlay) : actions_(actions), picker_(picker), overlay_(overlay) {}

void DimensionTool::begin(const ConstraintRequest& request, std::span<const core::sketch::GeometryRef> initialRefs) {
    request_ = request;
    refs_.assign(initialRefs.begin(), initialRefs.end());
    overlay_.constraintRefs_ = refs_;
}

void DimensionTool::onMouseMove(const input::MouseMoveEvent& e) { (void)e; }

void DimensionTool::onMouseButton(const input::MouseButtonEvent& e) {
    if (e.button != input::MouseButton::Left || e.action != input::MouseButtonAction::Press) {
        return;
    }

    const auto pick = request_.action == ConstraintAction::Angle ? picker_.pickLineAtScreenLogical(e.x, e.y) : picker_.pickAtScreenLogical(e.x, e.y);
    if (!pick) {
        return;
    }

    handleRef(pick->ref);
}

void DimensionTool::onKey(const input::KeyEvent& e) { (void)e; }

bool DimensionTool::cancel() {
    const bool handled = !refs_.empty();
    resetInputs();
    return handled;
}

void DimensionTool::handleRef(const core::sketch::GeometryRef& ref) {
    auto candidateRefs = refs_;
    candidateRefs.push_back(ref);
    const auto preparation = actions_.prepare(request_, candidateRefs);

    switch (preparation.state) {
        case ConstraintPreparation::State::NeedsMoreInput:
            refs_ = std::move(candidateRefs);
            overlay_.constraintRefs_ = refs_;
            return;
        case ConstraintPreparation::State::Ready:
            if (!preparation.definition || !actions_.apply(*preparation.definition)) {
                return;
            }
            resetInputs();
            return;
        case ConstraintPreparation::State::InvalidSelection:
        case ConstraintPreparation::State::Unsupported:
            return;
    }
}

void DimensionTool::resetInputs() {
    refs_.clear();
    overlay_.constraintRefs_.clear();
}
