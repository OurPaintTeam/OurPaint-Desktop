#ifndef OURPAINT_APPLICATION_CONSTRAINT_PRESENTATION_STATE_H_
#define OURPAINT_APPLICATION_CONSTRAINT_PRESENTATION_STATE_H_

#include <map>
#include <variant>

#include "SketchTypes.h"

namespace app {

struct LinearLabelPlacement {
    // Fraction from the first anchor to the second for Length/Distance;
    // sketch units from the midpoint along the fixed axis for H/V dimensions.
    double along;
    double normalOffset;
};

struct PolarLabelPlacement {
    // Radians from the reference direction. Distance in sketch units from the
    // rim for Radius/Diameter or from the vertex for Angle.
    double angle;
    double radialOffset;
};

using ConstraintLabelPlacement = std::variant<LinearLabelPlacement, PolarLabelPlacement>;

// View-owned presentation input, separate from solver definitions and derived layout.
// Dimension layout consumes these anchors without modifying them. CursorTool
// edits them during dimension dragging. Persistence and history remain future work.
struct ConstraintPresentationState {
    std::map<core::sketch::ConstraintId, ConstraintLabelPlacement> labelPlacements;
};

}  // namespace app

#endif  // OURPAINT_APPLICATION_CONSTRAINT_PRESENTATION_STATE_H_
