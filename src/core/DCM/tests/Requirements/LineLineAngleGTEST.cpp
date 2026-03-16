#include <gtest/gtest.h>
#include "Requirements.h"
#include "Line.h"
#include "ErrorFunction.h"
#include <cmath>

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(LineLineAngleTest, RightAngle) {
    Point2D l1_start{0.0, 0.0};
    Point2D l1_end{1.0, 0.0};
    Line line1{&l1_start, &l1_end};

    Point2D l2_start{0.0, 0.0};
    Point2D l2_end{0.0, 1.0};
    Line line2{&l2_start, &l2_end};

    double angle = 90; // 90 degrees
    LineLineAngle req(&line1, &line2, angle);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);

    delete ef;
}

TEST(LineLineAngleTest, FortyFiveDegrees) {
    Point2D l1_start{0.0, 0.0};
    Point2D l1_end{1.0, 0.0};
    Line line1{&l1_start, &l1_end};

    Point2D l2_start{0.0, 0.0};
    Point2D l2_end{1.0, 1.0};
    Line line2{&l2_start, &l2_end};

    double angle = 45; // 45 degrees
    LineLineAngle req(&line1, &line2, angle);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);

    delete ef;
}

TEST(LineLineAngleTest, IncorrectAngle) {
    Point2D l1_start{0.0, 0.0};
    Point2D l1_end{1.0, 0.0};
    Line line1{&l1_start, &l1_end};

    Point2D l2_start{0.0, 0.0};
    Point2D l2_end{0.0, 1.0};
    Line line2{&l2_start, &l2_end};

    double angle = 45; // 45 degrees, but lines are actually 90 degrees
    LineLineAngle req(&line1, &line2, angle);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}
