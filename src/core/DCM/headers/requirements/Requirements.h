#ifndef OURPAINTDCM_HEADERS_REQUIREMENTS_REQUIREMENTS_H
#define OURPAINTDCM_HEADERS_REQUIREMENTS_REQUIREMENTS_H
#include "Enums.h"
#include "Point2D.h"
#include "Line.h"
#include "Circle.h"
#include "ErrorFunction.h"

namespace OurPaintDCM::Requirements {
/**
* @brief Base class for all geometric requirements (constraints).
*
* A requirement represents a geometric constraint between objects
* (points, lines, circles, etc.). Each requirement can be converted
* into a mathematical function (equation) that must be satisfied
* by the solver.
*/
class Requirement {
protected:
    Utils::RequirementType type; ///< Type of the requirement (distance, angle, coincidence, etc.)
public:
    /**
    * @brief Construct a new Requirement.
    * @param t The type of this requirement.
    */
    explicit Requirement(Utils::RequirementType t)
        : type(t) {
    }

    /// Virtual destructor.
    virtual ~Requirement() = default;
    /**
     * @brief Get the type of this requirement.
     * @return The requirement type.
     */
    Utils::RequirementType getType() const noexcept { return type; }
    virtual double getParam() const noexcept {
        return 0.0;
    }
    /**
     * @brief Convert this requirement into a mathematical function.
     *
     * Each requirement generates one function
     * (equations) that the global solver must satisfy.
     *
     * @return Pointer to a Function object representing this requirement.
     */
    virtual ErrorFunction* toFunction() = 0;
};

/**
 * @brief Requirement: fixed distance between point and sections.
 *
 * This requirement enforces that the Euclidean distance
 * between point and section is equal to the specified value.
 */
class PointLineDist final : public Requirement {
    Figures::Point2D*                _p;    ///< pointer to point.
    Figures::Line<Figures::Point2D>* _l;    ///< pointer to line.
    double                           _dist; ///< distance between point and line
public:
    /**
     * @brief Construct a new PointSectionDist requirement.
     * @param p Point.
     * @param l Line.
     * @param dist Fixed distance value between the point and line.
     */
    PointLineDist(Figures::Point2D*                p,
                  Figures::Line<Figures::Point2D>* l,
                  double                           dist);
    /**
      * @brief Build the function for the point section distance constraint.
      *
      * The equation is derived from the general line equation through
      * segment endpoints A(xₐ, yₐ), B(xᵦ, yᵦ):
      *
      * \f[
      * \begin{aligned}
      * Ax + By + C &= 0 \\
      * A &= y_B - y_A \\
      * B &= x_A - x_B \\
      * C &= x_B y_A - x_A y_B
      * \end{aligned}
      * \f]
      *
      * For a point P(x, y), the signed distance to line AB is:
      *
      * \f[
      * F = \frac{A x_P + B y_P + C}{\sqrt{A^2 + B^2}}
      * \f]
      *
      * The requirement enforces:
      *
      * \f[ F - d = 0 \f]
      *
      * where \f$ d \f$ is the fixed distance value.
      *
      * @return Pointer to function object tree.
      */
    ErrorFunction* toFunction() override;
    double getParam() const noexcept override {
        return _dist;
    }
};

/**
 * @brief Requirement: put the point to the section.
 *
 * This requirement enforces that the Euclidean distance
 * between point and section is equal to 0.
 */
class PointOnLine final : public Requirement {
    Figures::Point2D*                _p; ///< pointer to point.
    Figures::Line<Figures::Point2D>* _l; ///< pointer to line.
public:
    /**
     * @brief Construct a new PointOnLine requirement.
     * @param p Point.
     * @param l Line.
     */
    PointOnLine(Figures::Point2D* p, Figures::Line<Figures::Point2D>* l);
    /**
     * @brief Simular to PointLineDist.
     * @return Pointer to function object tree.
     */
    ErrorFunction* toFunction() override;
};

/**
 * @brief Constraint: Fixed distance between two points.
 *
 * This requirement enforces a specific Euclidean distance between two points
 * in 2D space. The constraint maintains the exact distance value regardless
 * of the points' positions.
 */
class PointPointDist final : public Requirement {
    Figures::Point2D* _p1;   ///< pointer to first point.
    Figures::Point2D* _p2;   ///< pointer to second point.
    double            _dist; ///< distance between two points.

public:
    /**
     * @brief Construct a new Point-Point Distance constraint.
     *
     * @param p1 First point
     * @param p2 Second point
     * @param dist Distance between 2 points
     */
    PointPointDist(Figures::Point2D* p1, Figures::Point2D* p2, double dist);
    /**
     * @brief Convert the constraint to a function for solving.
     *
     * Creates a function tree representing the constraint equation:
     * \f[
     * (x_2 - x_1)^2 + (y_2 - y_1)^2 - d^2 = 0
     * \f]
     * where \f$d\f$ is the required distance.
     *
     * @return ErrorFunction* Pointer to the constraint function
     */
    ErrorFunction* toFunction() override;
    double getParam() const noexcept override {
        return _dist;
    }
};

/**
 * @brief Constraint: Two points must coincide.
 *
 * This requirement enforces that two points have identical coordinates,
 * effectively making them reference the same geometric location.
 */
class PointOnPoint final : public Requirement {
    Figures::Point2D* _p1;
    Figures::Point2D* _p2;

public:
    /** @brief Construct a new PointOnPoint requirement
     *
     *  @param p1 First point
     *  @param p2 second point
     */
    PointOnPoint(Figures::Point2D* p1, Figures::Point2D* p2);
    /**
     * @brief Convert the constraint to a function for solving.
     * This constraint use redefinition. We use two name for
     * one point for more flexability.
     * \f[
     * p_1 == p_2
     * \f]
     * @return ErrorFunction* Pointer to the constraint function
     */
    ErrorFunction* toFunction() override;
};

/**
 * @brief Constraint: Distance between a line and a circle.
 *
 * This requirement enforces that the distance between a line
 * and a circle equals a given fixed value. The distance is
 * measured from the closest point on the line to the circle's
 * center, minus the circle's radius.
 */
class LineCircleDist final : public Requirement {
    Figures::Line<Figures::Point2D>*   _l;   ///< Pointer to the line.
    Figures::Circle<Figures::Point2D>* _c; ///< Pointer to the circle.
    double                             _dist;   ///< Desired distance between line and circle.
public:
    /**
     * @brief Construct a new LineCircleDist requirement.
     *
     * @param line Pointer to the line.
     * @param circle Pointer to the circle.
     * @param dist Fixed distance value between the line and circle.
     */
    LineCircleDist(Figures::Line<Figures::Point2D>* line, Figures::Circle<Figures::Point2D>* circle, double dist);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The distance between line L\f$(Ax + By + C = 0)\f$ and circle
     * with center \f$(x_c, y_c)\f$and radius \f$r\f$ is:
     * \f[
     * F = \frac{|A x_c + B y_c + C|}{\sqrt{A^2 + B^2}} - r - d = 0
     * \f]
     *
     * where \f$d\f$ is the desired fixed distance.
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
    double getParam() const noexcept override {
        return _dist;
    }
};
/**
 * @brief Constraint: Line lies on a circle.
 *
 * This requirement enforces that a line lies on a circle.
 * The constraint ensures that the line intersects the circle
 * in such a way that all points on the line satisfy the
 * circle's equation.
 */
class LineOnCircle final : public Requirement { //!NOT TESTED
    Figures::Line<Figures::Point2D>* _l; ///< Pointer to the line.
    Figures::Circle<Figures::Point2D>* _c; ///< Pointer to the circle.
public:
    /**
     * @brief Construct a new LineOnCircle requirement.
     *
     * @param line Pointer to the line.
     * @param circle Pointer to the circle.
     */
    LineOnCircle(Figures::Line<Figures::Point2D>* line, Figures::Circle<Figures::Point2D>* circle);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The constraint ensures that the line satisfies the circle's equation:
     * \f[
     * F = (x - x_c)^2 + (y - y_c)^2 - r^2 = 0
     * \f]
     *
     * where \f$(x_c, y_c)\f$ is the circle's center and \f$r\f$ is its radius.
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
};
/**
 * @brief Constraint: Line lies entirely inside a circle.
 *
 * This requirement enforces that a line lies completely within a circle.
 * The constraint ensures that all points of the line are at a distance
 * from the circle's center less than or equal to the circle's radius.
 */
class LineInCircle final : public Requirement {//!NOT TESTED
    Figures::Line<Figures::Point2D>* _l;   ///< Pointer to the line.
    Figures::Circle<Figures::Point2D>* _c; ///< Pointer to the circle.
public:
    /**
     * @brief Construct a new LineInCircle requirement.
     *
     * @param line Pointer to the line.
     * @param circle Pointer to the circle.
     */
    LineInCircle(Figures::Line<Figures::Point2D>* line, Figures::Circle<Figures::Point2D>* circle);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The constraint ensures that both endpoints of the line satisfy:
     * \f[
     * F = (x - x_c)^2 + (y - y_c)^2 - r^2 \le 0
     * \f]
     *
     * where \f$(x_c, y_c)\f$ is the circle's center and \f$r\f$ is its radius.
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
};
/**
 * @brief Constraint: Two lines are parallel.
 *
 * This requirement enforces that two lines remain parallel.
 * The constraint ensures that the slopes of the lines are equal,
 * so they never intersect (unless they are coincident).
 */
class LineLineParallel final : public Requirement {
    Figures::Line<Figures::Point2D>* _l1; ///< Pointer to the first line.
    Figures::Line<Figures::Point2D>* _l2; ///< Pointer to the second line.
public:
    /**
     * @brief Construct a new LineLineParallel requirement.
     *
     * @param l1 Pointer to the first line.
     * @param l2 Pointer to the second line.
     */
    LineLineParallel(Figures::Line<Figures::Point2D>* l1, Figures::Line<Figures::Point2D>* l2);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The constraint ensures that the lines have the same slope:
     * \f[
     * F = (y_2^{end} - y_2^{start}) / (x_2^{end} - x_2^{start})
     *   - (y_1^{end} - y_1^{start}) / (x_1^{end} - x_1^{start}) = 0
     * \f]
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
};
/**
 * @brief Constraint: Two lines are perpendicular.
 *
 * This requirement enforces that two lines intersect at a right angle.
 * The constraint ensures that the dot product of their direction vectors is zero.
 */
class LineLinePerpendicular final : public Requirement {
    Figures::Line<Figures::Point2D>* _l1; ///< Pointer to the first line.
    Figures::Line<Figures::Point2D>* _l2; ///< Pointer to the second line.
public:
    /**
     * @brief Construct a new LineLinePerpendicular requirement.
     *
     * @param l1 Pointer to the first line.
     * @param l2 Pointer to the second line.
     */
    LineLinePerpendicular(Figures::Line<Figures::Point2D>* l1, Figures::Line<Figures::Point2D>* l2);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The constraint ensures perpendicularity using the dot product:
     * \f[
     * F = (x_1^{end} - x_1^{start}) \cdot (x_2^{end} - x_2^{start})
     *   + (y_1^{end} - y_1^{start}) \cdot (y_2^{end} - y_2^{start}) = 0
     * \f]
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
};
/**
 * @brief Constraint: Angle between two lines.
 *
 * This requirement enforces a specific angle between two lines.
 * The constraint ensures that the cosine of the angle between their
 * direction vectors matches the desired value.
 */
class LineLineAngle final : public Requirement {
    Figures::Line<Figures::Point2D>* _l1; ///< Pointer to the first line.
    Figures::Line<Figures::Point2D>* _l2; ///< Pointer to the second line.
    double _angle; ///< Angle between lines
public:
    /**
     * @brief Construct a new LineLineAngle requirement.
     *
     * @param l1 Pointer to the first line.
     * @param l2 Pointer to the second line.
     */
    LineLineAngle(Figures::Line<Figures::Point2D>* l1, Figures::Line<Figures::Point2D>* l2, double angle);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The constraint ensures the angle between the lines:
     * \f[
     * F = \frac{(x_1^{end} - x_1^{start})(x_2^{end} - x_2^{start})
     *      + (y_1^{end} - y_1^{start})(y_2^{end} - y_2^{start})}
     *      {\sqrt{(x_1^{end} - x_1^{start})^2 + (y_1^{end} - y_1^{start})^2} \,
     *       \sqrt{(x_2^{end} - x_2^{start})^2 + (y_2^{end} - y_2^{start})^2}}} - \cos(\theta) = 0
     * \f]
     *
     * where \f$\theta\f$ is the desired angle between the lines.
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
    double getParam() const noexcept override {
        return _angle;
    }
};

/**
 * @brief Constraint: Line is horizontal.
 *
 * This requirement enforces that a line is horizontal.
 * The constraint ensures that the y-coordinates of both endpoints
 * of the line are equal.
 */
class LineHorizontal final : public Requirement {
    Figures::Line<Figures::Point2D>* _l; ///< Pointer to the line.
public:
    /**
     * @brief Construct a new LineHorizontal requirement.
     *
     * @param l Pointer to the line.
     */
    LineHorizontal(Figures::Line<Figures::Point2D>* l);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The constraint ensures horizontal orientation:
     * \f[
     * F = y^{end} - y^{start} = 0
     * \f]
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
};
/**
 * @brief Constraint: Line is vertical.
 *
 * This requirement enforces that a line is vertical.
 * The constraint ensures that the x-coordinates of both endpoints
 * of the line are equal.
 */
class LineVertical final : public Requirement {
    Figures::Line<Figures::Point2D>* _l; ///< Pointer to the line.
public:
    /**
     * @brief Construct a new LineVertical requirement.
     *
     * @param l Pointer to the line.
     */
    LineVertical(Figures::Line<Figures::Point2D>* l);

    /**
     * @brief Convert the constraint to a function for solving.
     *
     * The constraint ensures vertical orientation:
     * \f[
     * F = x^{end} - x^{start} = 0
     * \f]
     *
     * @return ErrorFunction* Pointer to the constraint function.
     */
    ErrorFunction* toFunction() override;
};

}
#endif //OURPAINTDCM_HEADERS_REQUIREMENTS_REQUIREMENTS_H