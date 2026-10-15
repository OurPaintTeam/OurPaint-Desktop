#ifndef OURPAINT_APPLICATION_SNAP_FEEDBACK_H_
#define OURPAINT_APPLICATION_SNAP_FEEDBACK_H_

#include <vector>

#include "../../core/sketch/SketchTypes.h"

namespace app {

// Presentation symbols, independent of the snap engine's candidate types.
enum class SnapGlyph {
    Point,
    Endpoint,
    Midpoint,
    Center,
    Intersection,
    Direction
};

enum class SnapGuideExtent {
    Segment,
    InfiniteLine
};

struct SnapGuide {
    // Two finite sketch-local points define the segment or supporting line.
    core::sketch::Line2 line;
    SnapGuideExtent extent = SnapGuideExtent::Segment;
};

// Owning, transient data for one viewport. Publishing it never edits Sketch or
// selection, and does not imply that a persistent constraint has been created.
// The producer must replace or clear it when its input/model context changes.
struct SnapFeedback {
    core::sketch::Vec2 position;
    SnapGlyph glyph = SnapGlyph::Point;
    // Whole highlights a curve; Start/End/Center highlight only that point site.
    std::vector<core::sketch::GeometryRef> sources;
    std::vector<SnapGuide> guides;
};

}  // namespace app

#endif  // OURPAINT_APPLICATION_SNAP_FEEDBACK_H_
