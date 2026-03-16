#include <gtest/gtest.h>
#include "Requirements.h"
#include "Point2D.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(PointOnPoint, ConstructorAndVariableCount) {
    Point2D p1{0.0, 0.0};
    Point2D p2{0.0, 0.0};

    PointOnPoint req(&p1, &p2);
    ErrorFunction* ef = req.toFunction();

    ASSERT_EQ(ef->getVariables().size(), 4u);
    delete ef;
}

TEST(PointOnPoint, SamePositionZeroError) {
    Point2D p1{1.0, 2.0};
    Point2D p2{1.0, 2.0};

    PointOnPoint req(&p1, &p2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}

TEST(PointOnPoint, DifferentPositionNonZeroError) {
    Point2D p1{1.0, 2.0};
    Point2D p2{1.0, 3.0};

    PointOnPoint req(&p1, &p2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 0.0);
    delete ef;
}

TEST(PointOnPoint, DifferentXCoordinate) {
    Point2D p1{1.0, 2.0};
    Point2D p2{2.0, 2.0};

    PointOnPoint req(&p1, &p2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 0.0);
    delete ef;
}

TEST(PointOnPoint, DifferentYCoordinate) {
    Point2D p1{1.0, 2.0};
    Point2D p2{1.0, 3.0};

    PointOnPoint req(&p1, &p2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 0.0);
    delete ef;
}

TEST(PointOnPoint, BothDifferentCoordinates) {
    Point2D p1{1.0, 2.0};
    Point2D p2{3.0, 4.0};

    PointOnPoint req(&p1, &p2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 0.0);
    delete ef;
}