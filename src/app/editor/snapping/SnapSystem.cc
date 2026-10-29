#include "SnapSystem.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <utility>
#include <vector>

#include "Algorithms.h"
#include "Camera2D.h"
#include "sketch/Sketch.h"
#include "sketch/SketchTypes.h"
#include "snapping/algorithms/SnapEntityQuery.h"
#include "snapping/algorithms/SnapTangencyQuery.h"

namespace snap {
namespace {
using namespace core::sketch;

using geometry::Circle2;
using geometry::GeometryRef;
using geometry::Line2;
using geometry::Point2;
using geometry::SubElement;
using geometry::Vec2;

using geometry::distance;
using geometry::detail::add;
using geometry::detail::closestPointOnCircle;
using geometry::detail::closestPointOnSegment;
using geometry::detail::cross;
using geometry::detail::distanceSquared;
using geometry::detail::dot;
using geometry::detail::isFinite;
using geometry::detail::lengthSquared;
using geometry::detail::midpoint;
using geometry::detail::multiply;
using geometry::detail::subtract;
}

SnapSystem::SnapSystem(const core::sketch::Sketch& sketch, const Camera2D& camera) : sketch_(sketch), camera_(camera) {}

SnapResult SnapSystem::getSnapCandidate(const SnapRequest& request) const { return  SnapResult(); }

}  // namespace snap