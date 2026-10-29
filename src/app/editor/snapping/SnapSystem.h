#ifndef OURPAINT_APP_SNAP_SYSTEM_H_
#define OURPAINT_APP_SNAP_SYSTEM_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "SnapTypes.h"
#include "geometry/GeometryTypes.h"
#include "sketch/SketchTypes.h"

class Camera2D;
namespace core::sketch {
class Sketch;
}

namespace snap {

struct SnapRequest {
    /// Cursor position in sketch (world) coordinates.
    core::sketch::Vec2 cursor;

    /// Active tool.
    ActionType action;

    /// Current placement state.
    DrawState drawState = DrawState::Start;

    /// Points already committed by the tool (e.g. first point of a line).
    /// If empty — directional snaps are not activated.
    std::vector<core::sketch::Point2> anchors;

    std::vector<core::sketch::GeometryRef> forbiddenObjects;

};


/// Coincident + P/P
/// Coincident + H/V
/// Coincident + tangent
struct SnapRelation {
    SnapKind type = SnapKind::Coincident;
    double angle = 0.0;
    std::vector<core::sketch::GeometryRef> objects;
    std::vector<core::sketch::Line2> guideLines;
};

struct SnapResult {
    /// True if a candidate was found and accepted.
    bool snapped = false;

    /// Resolved point in sketch (world) coordinates.
    core::sketch::Point2 point;

    /// Type of the resolved candidate.
    SnapKind type = SnapKind::Coincident;

    /// Angle (radians) for directional snaps (Angle, Parallel, etc.).
    double angle = 0.0;

    /// Objects participating in the snap:
    /// both primitives for an intersection, the reference line for
    /// Parallel/Perpendicular, the snapped object for Body snaps, etc.
    std::vector<core::sketch::GeometryRef> objects;

    /// Auxiliary guide lines for rendering (infinite or finite).
    /// Used for Parallel, Perpendicular, Horizontal, Vertical, Angle,
    /// PerpendicularBisector, and for "almost parallel" hints.
    std::vector<core::sketch::Line2> guideLines;

    /// Auxiliary geometry drawn in a different style.
    /// Used when draw state > 0 to preview what the user is drawing.
    std::vector<core::sketch::SketchGeometry> visualObjectsTools;

    /// Additional simultaneous constraints at the same resolved point.
    /// E.g. Coincident + Parallel or Coincident + Perpendicular.
    std::vector<SnapRelation> relations;
};


struct SnapCandidate {
    /// Resolved candidate data.
    SnapResult result;

    /// Squared distance to the cursor, for ranking.
    /// Squared to avoid a sqrt during selection.
    double distanceSquared = 0.0;

    /// Priority of the snap kind (higher wins when distances are close).
    int priority = 0;
};

class SnapSystem {
private:
    const core::sketch::Sketch& sketch_;
    const Camera2D& camera_;

public:
    explicit SnapSystem(const core::sketch::Sketch& sketch,const Camera2D& camera);
    SnapResult getSnapCandidate(const SnapRequest& request) const;

private:

};

}  // namespace snap

#endif  // OURPAINT_APP_SNAP_SYSTEM_H_
