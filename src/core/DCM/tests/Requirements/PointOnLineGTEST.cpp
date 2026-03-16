#include <gtest/gtest.h>
#include "Requirements.h"
#include "Line.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(PointOnLine, ConstructorAndVariableCount) {
    Point2D p{0.5, 0.5};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{1.0, 1.0};
    Line line{&l_start, &l_end};

    PointOnLine req(&p, &line);
    ErrorFunction* ef = req.toFunction();

    ASSERT_EQ(ef->getVariables().size(), 6u);
    delete ef;
}

TEST(PointOnLine, PointOnLineZeroError) {
    Point2D p{0.5, 0.5};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{1.0, 1.0};
    Line line{&l_start, &l_end};

    PointOnLine req(&p, &line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}
TEST(PointOnLine, PointNotOnLineNonZeroError) {
    Point2D p{0.5, 0.6};  // Point above the line y=x
    Point2D l_start{0.0, 0.0};
    Point2D l_end{1.0, 1.0};
    Line line{&l_start, &l_end};

    PointOnLine req(&p, &line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 0.0);
    delete ef;
}

TEST(PointOnLine, PointAtLineStart) {
    Point2D p{0.0, 0.0};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{1.0, 1.0};
    Line line{&l_start, &l_end};

    PointOnLine req(&p, &line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}
TEST(PointOnLine, PointAtLineEnd) {
    Point2D p{1.0, 1.0};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{1.0, 1.0};
    Line line{&l_start, &l_end};

    PointOnLine req(&p, &line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}

TEST(PointOnLine, HorizontalLine) {
    Point2D p{1.0, 0.0};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{2.0, 0.0};
    Line line{&l_start, &l_end};

    PointOnLine req(&p, &line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}
TEST(PointOnLine, VerticalLine) {
    Point2D p{0.0, 1.0};
    Point2D l_start{0.0, 0.0};
    Point2D l_end{0.0, 2.0};
    Line line{&l_start, &l_end};

    PointOnLine req(&p, &line);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);
    delete ef;
}