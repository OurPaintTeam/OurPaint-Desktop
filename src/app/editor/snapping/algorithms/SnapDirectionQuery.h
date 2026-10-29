#ifndef OURPAINT_SNAP_DIRECTION_QUERY_H
#define OURPAINT_SNAP_DIRECTION_QUERY_H

#include <algorithm>
#include <cmath>
#include <numbers>
#include <optional>
#include <span>
#include <variant>
#include <vector>

#include "geometry/GeometryMath.h"

namespace geometry {

enum class DirectionSnapKind { AxisXParallel, AxisYParallel, AxisXPerpendicular, AxisYPerpendicular, ParallelToLine, PerpendicularToLine, AngleToLine };

}  // namespace geometry

#endif  // OURPAINT_SNAP_DIRECTION_QUERY_H
