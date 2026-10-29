#ifndef OURPAINT_SNAP_LINE_QUERY_H
#define OURPAINT_SNAP_LINE_QUERY_H

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
#include <utility>

#include "geometry/LineAlgorithms.h"
#include "geometry/GeometryTypes.h"
#include "sketch/SketchTypes.h"

// This module owns enumeration of line features. SnapSystem only translates
// these geometric hits into ranked SnapCandidates and chooses the winner.
namespace snap::algorithms {

enum class LineFeature { Body, Start, End, Midpoint, CollinearExtension };

struct LineFeatureCandidate {
    geometry::Vec2 point;
    double distance = 0.0;
    LineFeature feature = LineFeature::Body;
    geometry::GeometryRef source;
    std::optional<geometry::Line2> guide;
};

inline bool usableLine(const geometry::Line2& line) {
    return geometry::detail::isFinite(line) && !geometry::detail::isDegenerate(line);
}

} // namespace snap::algorithms
#endif // OURPAINT_SNAP_LINE_QUERY_H
