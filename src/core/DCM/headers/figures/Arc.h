#ifndef HEADERS_FIGURES_ARC_H
#define HEADERS_FIGURES_ARC_H

#include <type_traits>
#include "PointBase.h"

namespace OurPaintDCM::Figures {
/**
 * @brief Geometric figure: Arc.
 *
 * This class represents an arc defined by two endpoints and a center point.
 * The arc is a segment of a circle lying between the points \f$p1\f$ and \f$p2\f$
 * around the center \f$p_center\f$.
 *
 * @tparam PointT Type of points used, must inherit from PointBase.
 */
template <typename PointT>
struct Arc {
    static_assert(std::is_base_of_v<PointBase<PointT::dimension()>, PointT>,
                  "PointT class must derive from PointBase<N>");

    PointT* p1;       ///< Pointer to the first endpoint of the arc.
    PointT* p2;       ///< Pointer to the second endpoint of the arc.
    PointT* p_center; ///< Pointer to the center point of the arc.

    /**
     * @brief Construct a new Arc object.
     *
     * @param p1 Pointer to the first endpoint.
     * @param p2 Pointer to the second endpoint.
     * @param p_center Pointer to the center of the arc.
     */
    Arc(PointT* p1, PointT* p2, PointT* p_center)
        : p1(p1),
          p2(p2),
          p_center(p_center) {
    }
};

}
#endif //HEADERS_FIGURES_ARC_H