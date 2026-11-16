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
    core::sketch::Vec2 cursor;
    ActionType action = ActionType::None;
    PrimitiveType primitive = PrimitiveType::None;
    DrawState drawState = DrawState::None;
    std::vector<core::sketch::Point2> anchors;
    std::vector<core::sketch::GeometryRef> forbiddenObjects;
};

struct SnapResult {
    core::sketch::Point2 point;
    SnapKind type = SnapKind::None;
    std::vector<core::sketch::GeometryRef> objects;
    std::vector<core::sketch::Line2> guideLines;
};


struct SnapCandidate {
    SnapResult result;
    double distanceSquared = 0.0;
    int priority = 0;
};

class SnapSystem {
private:
    const core::sketch::Sketch& sketch_;
    const Camera2D& camera_;

public:
    explicit SnapSystem(const core::sketch::Sketch& sketch,const Camera2D& camera);
    std::vector<SnapResult> getSnapCandidates(const SnapRequest& request) const;

private:
    std::vector<SnapResult> parserRequest(const SnapRequest& snapRequest) const;
    std::vector<SnapResult> snapForDrawing(const SnapRequest& request) const;

    std::optional<SnapCandidate> collectCoincident(const SnapRequest& request) const;
    std::optional<SnapCandidate> collectMidpoint(const SnapRequest& request) const;
    std::optional<SnapCandidate> collectIntersection(const SnapRequest& request) const;
    std::optional<SnapCandidate> collectExtendedIntersection(const SnapRequest& request) const;
    std::optional<SnapCandidate> collectVirtualIntersection(const SnapRequest& request) const;
};

}  // namespace snap

#endif  // OURPAINT_APP_SNAP_SYSTEM_H_
