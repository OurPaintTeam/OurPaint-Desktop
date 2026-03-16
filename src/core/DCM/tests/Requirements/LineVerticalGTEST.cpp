#include <gtest/gtest.h>
#include "Requirements.h"
#include "Line.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(LineVerticalTest, VerticalLine) {
    Point2D start{2.0, 0.0};
    Point2D end{2.0, 3.0};
    Line line{&start, &end};

    LineVertical req(&line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);

    delete ef;
}

TEST(LineVerticalTest, NonVerticalLine) {
    Point2D start{0.0, 0.0};
    Point2D end{1.0, 2.0};
    Line line{&start, &end};

    LineVertical req(&line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}

TEST(LineVerticalTest, ChangeXCoordinates) {
    Point2D start{1.0, 0.0};
    Point2D end{1.0, 4.0};
    Line line{&start, &end};

    LineVertical req(&line);
    ErrorFunction* ef = req.toFunction();

    start[0] = 3.0;
    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}
