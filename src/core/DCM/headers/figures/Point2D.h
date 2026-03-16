#ifndef OURPAINTDCM_HEADERS_FIGURES_POINT2D_H
#define OURPAINTDCM_HEADERS_FIGURES_POINT2D_H
#include "PointBase.h"
namespace OurPaintDCM::Figures{
    /**
        * @brief 2D point structure.
        *
        * Represents a point in 2D space with X and Y
        * Basic object for circles, arcs, sections etc.
        */
    struct Point2D : public PointBase<2> {
        /**
             * @brief Default constructor initializes coordinates to zero.
             */
        constexpr Point2D() = default;

        /**
             * @brief Constructor with explicit coordinates.
             *
             * @param x X coordinate.
             * @param y Y coordinate.
             */
        constexpr Point2D(double x, double y)
            : PointBase<2>({x, y}) {}

        /**
             * @brief Get X coordinate.
             * @return Reference to the X coordinate.
             */
        constexpr double& x() { return coords[0]; }
        constexpr const double& x() const { return coords[0]; }

        /**
             * @brief Get Y coordinate.
             * @return Reference to the Y coordinate.
             */
        constexpr double& y() { return coords[1]; }
        constexpr const double& y() const { return coords[1]; }

        double* ptrX() { return &coords[0]; }
        double* ptrY() { return &coords[1]; }
    };

}
#endif //OURPAINTDCM_HEADERS_FIGURES_POINT2D_H
