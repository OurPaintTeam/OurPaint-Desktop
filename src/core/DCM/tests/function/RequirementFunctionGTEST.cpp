#include <gtest/gtest.h>
#include "functions/RequirementFunction.h"
#include <cmath>
#include <memory>
#include <vector>

using namespace OurPaintDCM::Function;

#define EQ(a, b) EXPECT_NEAR((a), (b), 1e-6)

// ======== PointPointDistanceFunction ========
TEST(PointPointDistanceFunctionTest, EvaluateAndGradient) {
    double x1 = 0, y1 = 0, x2 = 3, y2 = 4;
    std::vector<VAR> vars = {&x1, &y1, &x2, &y2};

    PointPointDistanceFunction f(vars, 5.0);
    double val = f.evaluate();
    EQ(val, 0.0);

    auto grad = f.gradient();
    EQ(grad[&x1], -0.6);
    EQ(grad[&y1], -0.8);
    EQ(grad[&x2], 0.6);
    EQ(grad[&y2], 0.8);
}

// ======== PointOnPointFunction ========
TEST(PointOnPointFunctionTest, EvaluateAndGradient) {
    double x1 = 0, y1 = 0, x2 = 3, y2 = 4;
    std::vector<VAR> vars = {&x1, &y1, &x2, &y2};

    PointOnPointFunction f(vars);
    EQ(f.evaluate(), 5.0);
    auto grad = f.gradient();
    EQ(grad[&x1], -0.6);
    EQ(grad[&y1], -0.8);
    EQ(grad[&x2], 0.6);
    EQ(grad[&y2], 0.8);
}

// ======== PointLineDistanceFunction ========
TEST(PointLineDistanceFunctionTest, EvaluateSimple) {
    double px = 0, py = 1;
    double x1 = 1, y1 = 0, x2 = -1, y2 = 0;
    std::vector<VAR> vars = {&px, &py, &x1, &y1, &x2, &y2};

    PointLineDistanceFunction f(vars, 1);
    EQ(f.evaluate(), 0.0);
}

// ======== LineLineParallelFunction ========
TEST(LineLineParallelFunctionTest, Evaluate) {
    double x1 = 0, y1 = 0, x2 = 1, y2 = 0;
    double x3 = 0, y3 = 1, x4 = 1, y4 = 1;
    std::vector<VAR> vars = {&x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4};
    LineLineParallelFunction f(vars);
    EQ(f.evaluate(), 0.0);
}

// ======== LineLinePerpendicularFunction ========
TEST(LineLinePerpendicularFunctionTest, Evaluate) {
    double x1 = 0, y1 = 0, x2 = 1, y2 = 0;
    double x3 = 0, y3 = 0, x4 = 0, y4 = 1;
    std::vector<VAR> vars = {&x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4};
    LineLinePerpendicularFunction f(vars);
    EQ(f.evaluate(), 0.0);
}

// ======== VerticalFunction ========
TEST(VerticalFunctionTest, EvaluateVerticalLine) {
    double x1 = 2, y1 = 0, x2 = 2, y2 = 5;
    std::vector<VAR> vars = {&x1, &y1, &x2, &y2};
    VerticalFunction f(vars);
    EQ(f.evaluate(), 0.0); // dx = 0
}

// ======== HorizontalFunction ========
TEST(HorizontalFunctionTest, EvaluateHorizontalLine) {
    double x1 = 0, y1 = 0, x2 = 5, y2 = 0;
    std::vector<VAR> vars = {&x1, &y1, &x2, &y2};
    HorizontalFunction f(vars);
    EQ(f.evaluate(), 0.0); // dy = 0
}

// ======== ArcCenterOnPerpendicularFunction ========
TEST(ArcCenterOnPerpendicularFunctionTest, Evaluate) {
    double Ax = 0, Ay = 0, Bx = 2, By = 0, Cx = 1, Cy = 1;
    std::vector<VAR> vars = {&Ax, &Ay, &Bx, &By, &Cx, &Cy};
    ArcCenterOnPerpendicularFunction f(vars);
    EQ(f.evaluate(), 0.0); // центр лежит на перпендикуляре
}
