#include <gtest/gtest.h>
#include "RequirementFunctionFactory.h"
#include "GeometryStorage.h"

using namespace OurPaintDCM::Function;
using namespace OurPaintDCM::Figures;

class RequirementFunctionFactoryTest : public ::testing::Test {
protected:
    GeometryStorage storage;
    Point2D* p1;
    Point2D* p2;
    Point2D* p3;
    Point2D* center;
    Line2D* line1;
    Line2D* line2;
    Circle2D* circle;
    Arc2D* arc;

    void SetUp() override {
        auto [id1, pt1] = storage.createPoint(0.0, 0.0);
        auto [id2, pt2] = storage.createPoint(3.0, 4.0);
        auto [id3, pt3] = storage.createPoint(6.0, 0.0);
        auto [id4, pt4] = storage.createPoint(5.0, 5.0);

        p1 = pt1;
        p2 = pt2;
        p3 = pt3;
        center = pt4;

        auto [lid1, l1] = storage.createLine(p1, p2);
        auto [lid2, l2] = storage.createLine(p2, p3);
        line1 = l1;
        line2 = l2;

        auto [cid, c] = storage.createCircle(center, 10.0);
        circle = c;

        auto [aid, a] = storage.createArc(p1, p3, center);
        arc = a;
    }
};

TEST_F(RequirementFunctionFactoryTest, CreatePointLineDist) {
    auto func = RequirementFunctionFactory::createPointLineDist(p3, line1, 2.4);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 6);
    EXPECT_EQ(func->getVars().size(), 6);
    
    auto vars = func->getVars();
    EXPECT_EQ(vars[0], p3->ptrX());
    EXPECT_EQ(vars[1], p3->ptrY());
    EXPECT_EQ(vars[2], line1->p1->ptrX());
    EXPECT_EQ(vars[3], line1->p1->ptrY());
    EXPECT_EQ(vars[4], line1->p2->ptrX());
    EXPECT_EQ(vars[5], line1->p2->ptrY());
}

TEST_F(RequirementFunctionFactoryTest, CreatePointOnLine) {
    auto func = RequirementFunctionFactory::createPointOnLine(p1, line1);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 6);
    
    double val = func->evaluate();
    EXPECT_NEAR(val, 0.0, 1e-9);
}

TEST_F(RequirementFunctionFactoryTest, CreatePointPointDist) {
    auto func = RequirementFunctionFactory::createPointPointDist(p1, p2, 5.0);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 4);
    
    double val = func->evaluate();
    EXPECT_NEAR(val, 0.0, 1e-9);
}

TEST_F(RequirementFunctionFactoryTest, CreatePointOnPoint) {
    auto func = RequirementFunctionFactory::createPointOnPoint(p1, p1);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 4);
    
    double val = func->evaluate();
    EXPECT_NEAR(val, 0.0, 1e-9);
}

TEST_F(RequirementFunctionFactoryTest, CreateLineCircleDist) {
    auto func = RequirementFunctionFactory::createLineCircleDist(line1, circle, 1.0);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 7);
    
    auto vars = func->getVars();
    EXPECT_EQ(vars[6], circle->ptrRadius());
}

TEST_F(RequirementFunctionFactoryTest, CreateLineOnCircle) {
    auto func = RequirementFunctionFactory::createLineOnCircle(line1, circle);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 7);
}

TEST_F(RequirementFunctionFactoryTest, CreateLineLineParallel) {
    auto func = RequirementFunctionFactory::createLineLineParallel(line1, line2);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 8);
}

TEST_F(RequirementFunctionFactoryTest, CreateLineLinePerpendicular) {
    auto func = RequirementFunctionFactory::createLineLinePerpendicular(line1, line2);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 8);
}

TEST_F(RequirementFunctionFactoryTest, CreateLineLineAngle) {
    auto func = RequirementFunctionFactory::createLineLineAngle(line1, line2, 1.57);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 8);
}

TEST_F(RequirementFunctionFactoryTest, CreateVertical) {
    auto func = RequirementFunctionFactory::createVertical(line1);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 4);
}

TEST_F(RequirementFunctionFactoryTest, CreateHorizontal) {
    auto func = RequirementFunctionFactory::createHorizontal(line1);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 4);
}

TEST_F(RequirementFunctionFactoryTest, CreateArcCenterOnPerpendicular) {
    auto func = RequirementFunctionFactory::createArcCenterOnPerpendicular(arc);
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->getVarCount(), 6);
    
    auto vars = func->getVars();
    EXPECT_EQ(vars[0], arc->p1->ptrX());
    EXPECT_EQ(vars[1], arc->p1->ptrY());
    EXPECT_EQ(vars[2], arc->p2->ptrX());
    EXPECT_EQ(vars[3], arc->p2->ptrY());
    EXPECT_EQ(vars[4], arc->p_center->ptrX());
    EXPECT_EQ(vars[5], arc->p_center->ptrY());
}

TEST_F(RequirementFunctionFactoryTest, GradientNotEmpty) {
    auto func = RequirementFunctionFactory::createPointPointDist(p1, p2, 5.0);
    auto grad = func->gradient();
    EXPECT_FALSE(grad.empty());
}

TEST_F(RequirementFunctionFactoryTest, VarsPointToStorageData) {
    auto func = RequirementFunctionFactory::createPointPointDist(p1, p2, 5.0);
    auto vars = func->getVars();
    
    *vars[0] = 100.0;
    EXPECT_DOUBLE_EQ(p1->x(), 100.0);
    
    *vars[1] = 200.0;
    EXPECT_DOUBLE_EQ(p1->y(), 200.0);
}
