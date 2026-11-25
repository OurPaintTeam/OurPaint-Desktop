#ifndef OURPAINT_APPLICATION_NURBS_POLYLINE_H_
#define OURPAINT_APPLICATION_NURBS_POLYLINE_H_

#include <algorithm>
#include <cmath>
#include <vector>

#include "SketchTypes.h"
#include "tinynurbs/include/tinynurbs/core/evaluate.h"

namespace app {

// Picking and drawing use the same sampled curve, including both ends of its knot domain.
inline std::vector<core::sketch::Vec2> sampleNurbsPolyline(const core::sketch::NurbsCurve2& curve) {
    const auto count = curve.controlPoints.size();
    if (curve.degree < 1 || count <= static_cast<size_t>(curve.degree) || curve.weights.size() != count ||
        curve.knots.size() != count + curve.degree + 1 || !std::is_sorted(curve.knots.begin(), curve.knots.end())) {
        return {};
    }
    const double first = curve.knots[curve.degree];
    const double last = curve.knots[count];
    if (!(first < last)) {
        return {};
    }

    // Precompute homogeneous controls once for all samples of this planar rational curve.
    tinynurbs::Curve3d homogeneous;
    homogeneous.degree = static_cast<unsigned>(curve.degree);
    homogeneous.knots = curve.knots;
    homogeneous.control_points.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const auto& p = curve.controlPoints[i].position;
        const double w = curve.weights[i];
        homogeneous.control_points.emplace_back(p.x * w, p.y * w, w);
    }

    const size_t segments = 128 * count;
    std::vector<core::sketch::Vec2> points;
    points.reserve(segments + 1);
    for (size_t i = 0; i <= segments; ++i) {
        const double u = std::lerp(first, last, static_cast<double>(i) / static_cast<double>(segments));
        const auto p = tinynurbs::util::homogenousToCartesian(tinynurbs::curvePoint(homogeneous, u));
        if (!std::isfinite(p.x) || !std::isfinite(p.y)) {
            return {};
        }
        points.push_back({p.x, p.y});
    }
    return points;
}

}  // namespace app

#endif  // OURPAINT_APPLICATION_NURBS_POLYLINE_H_
