#ifndef OURPAINTDCM_HEADERS_FIGURES_POINTBASE_H
#define OURPAINTDCM_HEADERS_FIGURES_POINTBASE_H
#include <array>
#include <cstddef>
namespace OurPaintDCM::Figures{
	/**
		 * @brief Generic base class for N-dimensional points.
		 *
		 * This template represents a point in N-dimensional space. coordinats
		 * is stored as a double value.
		 * And this is serves as a base for specialized classes like Point2D, Point3D etc.
		 *
		 * @tparam N Number of dimensions. Must be greater than zero.
		 */
	template <std::size_t N>
	struct PointBase {
		static_assert(N > 0, "Dimension of point must be greater than zero");
		/**
     		* @brief Coordinates of the point.
    		*
     		* The array holds N double values.
    		* The values are initialized to zero by default.
   		  	*/
		std::array<double, N> coords{};
		/**
    	 	* @brief Default constructor.
    		*
     		* Initializes all coordinates to zero.
     		*/
		constexpr PointBase() = default;
		/**
    	 	* @brief Constructor with initialization values.
    		*
     		* Initializes the point with the given coordinates.
     		*
     		* @param values Array containing N coordinate values.
    		*/
		constexpr PointBase(const std::array<double, N>& values) : coords(values) {}
		/**
			 * @brief Access to coordinates by index with modify.
			 * @param index Index of the coordinates
			 * @return Reference to the coordinate
			 */
		constexpr double& operator[](std::size_t index) { return coords[index]; }
		/**
			 * @brief Access to coordinates by index without modify.
			 * @param index Index of the coordinates
			 * @return Const reference to the coordinate
			 */
		constexpr const double& operator[](std::size_t index) const { return coords[index]; }
		/**
        	 * @brief Get the number of dimensions.
             *
             * @return The number of dimensions as a constant expression.
             */
		static constexpr std::size_t dimension() { return N; }
	};
}
#endif //OURPAINTDCM_HEADERS_FIGURES_POINTBASE_H
