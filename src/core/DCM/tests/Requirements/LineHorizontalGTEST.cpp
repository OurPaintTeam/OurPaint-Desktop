#include <gtest/gtest.h>
#include "Requirements.h"
#include "Line.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(LineHorizontalTest, HorizontalLine) {
    Point2D start{0.0, 1.0};
    Point2D end{2.0, 1.0};
    Line line{&start, &end};

    LineHorizontal req(&line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);

    delete ef;
}

TEST(LineHorizontalTest, NonHorizontalLine) {
    Point2D start{0.0, 0.0};
    Point2D end{1.0, 1.0};
    Line line{&start, &end};

    LineHorizontal req(&line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}

TEST(LineHorizontalTest, ChangeYCoordinates) {
    Point2D start{0.0, 0.0};
    Point2D end{1.0, 0.0};
    Line line{&start, &end};

    LineHorizontal req(&line);
    ErrorFunction* ef = req.toFunction();

    start[1] = 2.0;
    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}
