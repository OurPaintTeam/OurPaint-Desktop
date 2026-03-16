#ifndef OURPAINTDCM_HEADERS_FIGURES_LINE_H
#define OURPAINTDCM_HEADERS_FIGURES_LINE_H
#include "PointBase.h"
#include "cmath"

namespace OurPaintDCM::Figures{
    /**
        * @brief Standart segment class.
        *
        * The line does not copy points. Changing the points will automatically
        * update the line.
        *
        * @tparam PointT Type derived from PointBase<N>, e.g., Point2D, Point3D.
        */
    template <typename PointT>
    struct Line {
        static_assert(std::is_base_of_v<PointBase<PointT::dimension()>, PointT>,
                      "PointT class must derive from PointBase<N>");

        PointT* p1; ///< Pointer to first point
        PointT* p2;   ///< Pointer to second point

        /**
            * @brief Constructor from two point pointers.
            *
            * @param p1 Pointer to the fisrt point.
            * @param p2 Pointer to the second point.
            */
        Line(PointT* p1, PointT* p2)
            : p1(p1), p2(p2) {}

        /**
             * @brief Calculate the length of the line segment.
             *
             * @return The distance between p1 and p2 points as a double.
             */
        double length() const {
            double sum = 0.0;
            for (std::size_t i = 0; i < PointT::dimension(); ++i) {
                double diff = (*p2)[i] - (*p1)[i];
                sum += diff * diff;
            }
            return std::sqrt(sum);
        }
    };

}

#endif //OURPAINTDCM_HEADERS_FIGURES_LINE_H
