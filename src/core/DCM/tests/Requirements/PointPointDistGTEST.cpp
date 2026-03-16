#include <gtest/gtest.h>
#include "Requirements.h"
#include "Point2D.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(PointPointDist, ConstructorAndVariableCount) {
    Point2D p1{0.0, 0.0};
    Point2D p2{1.0, 0.0};
    double dist = 1.0;

    PointPointDist req(&p1, &p2, dist);
    ErrorFunction* ef = req.toFunction();

    ASSERT_EQ(ef->getVariables().size(), 4u);
    delete ef;
}

TEST(PointPointDist, CorrectDistanceZeroError) {
    Point2D p1{0.0, 0.0};
    Point2D p2{3.0, 4.0};
    double dist = 5.0;

    PointPointDist req(&p1, &p2, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}

TEST(PointPointDist, IncorrectDistanceNonZeroError) {
    Point2D p1{0.0, 0.0};
    Point2D p2{3.0, 4.0};
    double dist = 4.0;

    PointPointDist req(&p1, &p2, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 0.0);
    delete ef;
}

TEST(PointPointDist, ZeroDistance) {
    Point2D p1{1.0, 1.0};
    Point2D p2{1.0, 1.0};
    double dist = 0.0;

    PointPointDist req(&p1, &p2, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}

TEST(PointPointDist, HorizontalDistance) {
    Point2D p1{0.0, 0.0};
    Point2D p2{5.0, 0.0};
    double dist = 5.0;

    PointPointDist req(&p1, &p2, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}

TEST(PointPointDist, VerticalDistance) {
    Point2D p1{0.0, 0.0};
    Point2D p2{0.0, 3.0};
    double dist = 3.0;

    PointPointDist req(&p1, &p2, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}