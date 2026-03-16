#include <gtest/gtest.h>
#include "Requirements.h"
#include "Line.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(PointLineDist, PointAboveLineDistanceCorrect) {
    Point2D p{1.0, 1.0};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{2.0, 0.0};
    Line line{&l_start, &l_end};
    double dist = 1.0;

    PointLineDist req(&p, &line, dist);
    ErrorFunction* ef = req.toFunction();

    ASSERT_EQ(ef->getVariables().size(), 6u);
    delete ef;
}

TEST(PointLineDist, PointBelowLineDistanceIncorrect) {
    Point2D p{1.0, -1.0};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{2.0, 0.0};
    Line line{&l_start, &l_end};
    double dist = 2.0;

    PointLineDist req(&p, &line, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 0.0);

    delete ef;
}

TEST(PointLineDist, PointOnLine) {
    Point2D p{1.0, 0.0};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{2.0, 0.0};
    Line line{&l_start, &l_end};
    double dist = 0.0;

    PointLineDist req(&p, &line, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);

    delete ef;
}
