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

// =============================================================================
// SnapRequest — input for one snap query.
// Tool and draw state define which snaps are allowed.
// =============================================================================
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

// =============================================================================
// SnapResult — output of one snap query.
// Describes ONE resolved candidate: point, kind, and all geometry
// needed for rendering and for the tool.
// =============================================================================
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
};

// =============================================================================
// SnapRule — search rules for the current tool and state.
// =============================================================================
struct SnapRule {
    SnapMask mask = SnapMask::None;
    double maxRadius = 0.0;
    double regularRadius = 0.0;

    /// Distance band within which priority decides the winner.
    /// Candidates farther than this from each other are ranked by distance only.
    double priorityRadius = 0.0;

    /// Tighter screen-space limits for relation snaps that are easy to over-trigger.
    double collinearRadius = 0.0;
    double tangentRadius = 0.0;
    double endpointRadius = 0.0;
    double midpointRadius = 0.0;
};

// =============================================================================
// SnapCandidate — internal candidate before it becomes a public SnapResult.
// One query may produce several candidates; SnapSystem picks the best one.
// =============================================================================
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

    mutable std::optional<geometry::Vec2> directionSnapBaseAnchor_;
    mutable double directionSnapReferenceZoom_ = 1.0;

    // Small cursor movements within the same tool/state/anchor context cycle
    // through nearby snap alternatives instead of locking to the first result.
    mutable bool hasPreviousQuery_ = false;
    mutable ActionType previousAction_ = ActionType::Point;
    mutable DrawState previousDrawState_ = DrawState::Start;
    mutable core::sketch::Vec2 previousCursor_{};
    mutable std::vector<core::sketch::Vec2> previousAnchors_;
    mutable std::vector<core::sketch::GeometryRef> previousForbiddenObjects_;
    mutable std::size_t previousCycleIndex_ = 0;

public:
    explicit SnapSystem(const core::sketch::Sketch& sketch,const Camera2D& camera);
    SnapResult getSnapCandidate(const SnapRequest& request) const;

private:
    static SnapRule ruleFor(ActionType action, DrawState state);
    void  onAnchorPlaced(const geometry::Vec2& position);

    SnapResult parser(const SnapRequest& request) const;

    std::optional<SnapCandidate> findBestStandalonePoint(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestLineCandidate(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestCircleCandidate(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestIntersection(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestTangentCandidate(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestDirectionalCandidate(const SnapRequest& request, const SnapRule& rule) const;

    std::optional<SnapCandidate> findBestArcCandidate(const SnapRequest& request, const SnapRule& rule) const;

    static bool isAllowed(SnapKind kind, double distance, const SnapRule& rule);

    static bool isBetterCandidate(const SnapCandidate& candidate, const SnapCandidate& current, double priorityRadius);
};

}  // namespace snap

#endif  // OURPAINT_APP_SNAP_SYSTEM_H_
