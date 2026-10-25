#ifndef OURPAINT_ALGORITHMS_H
#define OURPAINT_ALGORITHMS_H

// Compatibility umbrella header. Prefer including the specific module in new code.
#include "geometry/GeometryTypes.h"
#include "geometry/GeometryMath.h"
#include "geometry/LineAlgorithms.h"
#include "geometry/CircleAlgorithms.h"
#include "geometry/ArcAlgorithms.h"
#include "geometry/relations/Tangency.h"
#include "geometry/relations/Parallelism.h"
#include "geometry/relations/Perpendicularity.h"
#include "geometry/relations/AxisRelations.h"
#include "geometry/intersections/IntersectionTypes.h"
#include "geometry/intersections/LineLineIntersection.h"
#include "geometry/intersections/LineCircleIntersection.h"
#include "geometry/intersections/CircleCircleIntersection.h"
#include "geometry/intersections/ArcIntersection.h"
#include "geometry/intersections/IntersectionDispatcher.h"
#include "snapping/algorithms/SnapQueryTypes.h"
#include "snapping/algorithms/SnapEntityQuery.h"
#include "snapping/algorithms/SnapLineQuery.h"
#include "snapping/algorithms/SnapTypeQuery.h"
#include "snapping/algorithms/SnapQuery.h"
#include "snapping/algorithms/SnapIntersectionQuery.h"
#include "snapping/algorithms/SnapDirectionQuery.h"
#include "snapping/algorithms/SnapTangencyQuery.h"

#endif // OURPAINT_ALGORITHMS_H
