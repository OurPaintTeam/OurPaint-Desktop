#include <gtest/gtest.h>
#include "Requirements.h"
#include "Line.h"
#include "ErrorFunction.h"

using namespace OurPaintDCM::Requirements;
using namespace OurPaintDCM::Figures;

TEST(LineLinePerpendicularTest, PerpendicularLines) {
    Point2D l1_start{0.0, 0.0};
    Point2D l1_end{1.0, 0.0};
    Line line1{&l1_start, &l1_end};

    Point2D l2_start{0.0, 0.0};
    Point2D l2_end{0.0, 1.0};
    Line line2{&l2_start, &l2_end};

    LineLinePerpendicular req(&line1, &line2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_NEAR(error_val, 0.0, 1e-8);

    delete ef;
}

TEST(LineLinePerpendicularTest, NonPerpendicularLines) {
    Point2D l1_start{0.0, 0.0};
    Point2D l1_end{1.0, 0.0};
    Line line1{&l1_start, &l1_end};

    Point2D l2_start{0.0, 0.0};
    Point2D l2_end{1.0, 1.0};
    Line line2{&l2_start, &l2_end};

    LineLinePerpendicular req(&line1, &line2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}

TEST(LineLinePerpendicularTest, CoincidentLines) {
    Point2D l1_start{0.0, 0.0};
    Point2D l1_end{1.0, 1.0};
    Line line1{&l1_start, &l1_end};

    Point2D l2_start{0.0, 0.0};
    Point2D l2_end{1.0, 1.0};
    Line line2{&l2_start, &l2_end};

    LineLinePerpendicular req(&line1, &line2);
    ErrorFunction* ef = req.toFunction();

    double error_val = ef->evaluate();
    // Coincident lines are not perpendicular, so expect non-zero error
    EXPECT_GT(std::abs(error_val), 1e-8);

    delete ef;
}
