#ifndef OURPAINT_APPLICATION_CONSTRAINT_PRESENTATION_STATE_H_
#define OURPAINT_APPLICATION_CONSTRAINT_PRESENTATION_STATE_H_

#include <map>

#include "SketchTypes.h"

namespace app {

struct ConstraintLabelPlacement {
    // User-chosen label center in sketch coordinates, independent of zoom and DPI.
    core::sketch::Vec2 position;
};

// View-owned presentation input, separate from solver definitions and derived layout.
// Dimension layout consumes these anchors without modifying them. CursorTool
// edits them during dimension dragging. Persistence and history remain future work.
struct ConstraintPresentationState {
    std::map<core::sketch::ConstraintId, ConstraintLabelPlacement> labelPlacements;
};

}  // namespace app

#endif  // OURPAINT_APPLICATION_CONSTRAINT_PRESENTATION_STATE_H_
