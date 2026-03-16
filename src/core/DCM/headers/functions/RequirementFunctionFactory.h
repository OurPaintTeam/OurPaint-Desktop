#ifndef OURPAINTDCM_FUNCTION_REQUIREMENTFUNCTIONFACTORY_H
#define OURPAINTDCM_FUNCTION_REQUIREMENTFUNCTIONFACTORY_H

#include "RequirementFunction.h"
#include "Point2D.h"
#include "Line.h"
#include "Circle.h"
#include "Arc.h"
#include <memory>
#include <vector>

namespace OurPaintDCM::Function {

/**
 * @brief Factory for creating RequirementFunction objects from geometric figures.
 *
 * Extracts double* pointers from figures and constructs the appropriate
 * constraint functions. All created functions reference the original
 * data in GeometryStorage, so changes are reflected automatically.
 */
class RequirementFunctionFactory {
public:
    /// @brief Create point-line distance function.
    static std::shared_ptr<PointLineDistanceFunction> createPointLineDist(
        Figures::Point2D* point,
        Figures::Line<Figures::Point2D>* line,
        double distance
    );

    /// @brief Create point-on-line function.
    static std::shared_ptr<PointOnLineFunction> createPointOnLine(
        Figures::Point2D* point,
        Figures::Line<Figures::Point2D>* line
    );

    /// @brief Create point-point distance function.
    static std::shared_ptr<PointPointDistanceFunction> createPointPointDist(
        Figures::Point2D* p1,
        Figures::Point2D* p2,
        double distance
    );

    /// @brief Create point-on-point function.
    static std::shared_ptr<PointOnPointFunction> createPointOnPoint(
        Figures::Point2D* p1,
        Figures::Point2D* p2
    );

    /// @brief Create line-circle distance function.
    static std::shared_ptr<LineCircleDistanceFunction> createLineCircleDist(
        Figures::Line<Figures::Point2D>* line,
        Figures::Circle<Figures::Point2D>* circle,
        double distance
    );

    /// @brief Create line-on-circle function.
    static std::shared_ptr<LineOnCircleFunction> createLineOnCircle(
        Figures::Line<Figures::Point2D>* line,
        Figures::Circle<Figures::Point2D>* circle
    );

    /// @brief Create parallel-lines function.
    static std::shared_ptr<LineLineParallelFunction> createLineLineParallel(
        Figures::Line<Figures::Point2D>* l1,
        Figures::Line<Figures::Point2D>* l2
    );

    /// @brief Create perpendicular-lines function.
    static std::shared_ptr<LineLinePerpendicularFunction> createLineLinePerpendicular(
        Figures::Line<Figures::Point2D>* l1,
        Figures::Line<Figures::Point2D>* l2
    );

    /// @brief Create angle-between-lines function.
    static std::shared_ptr<LineLineAngleFunction> createLineLineAngle(
        Figures::Line<Figures::Point2D>* l1,
        Figures::Line<Figures::Point2D>* l2,
        double angle
    );

    /// @brief Create vertical-line function.
    static std::shared_ptr<VerticalFunction> createVertical(
        Figures::Line<Figures::Point2D>* line
    );

    /// @brief Create horizontal-line function.
    static std::shared_ptr<HorizontalFunction> createHorizontal(
        Figures::Line<Figures::Point2D>* line
    );

    /// @brief Create arc center perpendicular-bisector function.
    static std::shared_ptr<ArcCenterOnPerpendicularFunction> createArcCenterOnPerpendicular(
        Figures::Arc<Figures::Point2D>* arc
    );
};

}

#endif
