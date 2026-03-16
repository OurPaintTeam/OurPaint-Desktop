#include <gtest/gtest.h>
#include "Requirements.h"
#include "Line.h"
#include "Circle.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(LineCircleDist, CircleTouchingLine) {
    Point2D l_start{0.0, 0.0};
    Point2D l_end{1.0, 1.0};
    Line line{&l_start, &l_end};

    Point2D center{0.0, 0.0};
    double radius = 1.0;
    Circle circle{&center, radius};

    double dist = -1.0;
    LineCircleDist req(&line, &circle, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);

    delete ef;
}

TEST(LineCircleDist, CircleAboveLine) {
    Point2D l_start{0.0, 0.0};
    Point2D l_end{2.0, 0.0};
    Line line{&l_start, &l_end};

    Point2D center{1.0, 3.0};
    double radius = 1.0;
    Circle circle{&center, radius};

    double dist = 0.0;
    LineCircleDist req(&line, &circle, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, -1.8944271909999157, 1e-8);

    delete ef;
}

TEST(LineCircleDist, IncorrectDistance) {
    Point2D l_start{0.0, 0.0};
    Point2D l_end{2.0, 0.0};
    Line line{&l_start, &l_end};

    Point2D center{1.0, 4.0};
    double radius = 1.0;
    Circle circle{&center, radius};

    double dist = 3.0;
    LineCircleDist req(&line, &circle, dist);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}
