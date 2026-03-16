#ifndef OURPAINTDCM_HEADERS_FIGURES_CIRCLE_H
#define OURPAINTDCM_HEADERS_FIGURES_CIRCLE_H

#include <type_traits>
#include "PointBase.h"
#include <numbers>
namespace OurPaintDCM::Figures
{

    /**
     * @brief Generic circle structure for 2D/3D points.
     *
     * Circle stores a pointer to the center point so that any
     * modification of the point coordinates is automatically reflected in the circle.
     *
     * @tparam PointT Type of point (e.g., Point2D, Point3D) derived from PointBase<N>.
     */
    template <typename PointT>
    struct Circle {
        static_assert(std::is_base_of_v<PointBase<PointT::dimension()>, PointT>,
                      "PointT class must derive from PointBase<N>");

        PointT* center; ///< Pointer to the center point.
        double radius; ///< Radius of the circle.
        /**
         * @brief Constructor for Circle.
         *
         * @param c Pointer to the center point.
         * @param r Radius of the circle (default: 10).
         */
        Circle(PointT* c, double r = 10.0) : center(c), radius(r) {}

        /**
         * @brief Get the area of the circle.
         *
         * @return Area of the circle as double.
         */
        double area() const {
            return std::numbers::pi * radius * radius;
        }

        /**
         * @brief Get the lenght of the circle.
         *
         * @return lenght of the circle as double.
         */
        double lenght() const{
            return 2 * std::numbers::pi * radius;
        }

        double* ptrRadius() { return &radius; }
    };
}
#endif //OURPAINTDCM_HEADERS_FIGURES_CIRCLE_H
