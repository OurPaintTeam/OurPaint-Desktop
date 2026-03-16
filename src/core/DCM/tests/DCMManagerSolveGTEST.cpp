#include <gtest/gtest.h>
#include "DCMManager.h"
#include <cmath>

using namespace OurPaintDCM;
using namespace OurPaintDCM::Utils;

class DCMManagerSolveTest : public ::testing::Test {
protected:
    DCMManager manager;
};

TEST_F(DCMManagerSolveTest, GlobalSolve_PointPointDist) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(3.0, 0.0));

    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 5.0));
    manager.setSolveMode(SolveMode::GLOBAL);

    bool converged = manager.solve();
    EXPECT_TRUE(converged);

    auto d1 = manager.getFigure(p1);
    auto d2 = manager.getFigure(p2);
    ASSERT_TRUE(d1.has_value() && d2.has_value());

    double dx = d2->x.value() - d1->x.value();
    double dy = d2->y.value() - d1->y.value();
    double dist = std::sqrt(dx * dx + dy * dy);
    EXPECT_NEAR(dist, 5.0, 0.1);
}

TEST_F(DCMManagerSolveTest, GlobalSolve_Horizontal) {
    auto line = manager.addFigure(FigureDescriptor::line(0.0, 0.0, 5.0, 3.0));
    auto lineDesc = manager.getFigure(line);
    ASSERT_TRUE(lineDesc.has_value());
    auto p1 = lineDesc->pointIds[0];
    auto p2 = lineDesc->pointIds[1];

    manager.addRequirement(RequirementDescriptor::horizontal(line));
    manager.setSolveMode(SolveMode::GLOBAL);

    bool converged = manager.solve();
    EXPECT_TRUE(converged);

    auto d1 = manager.getFigure(p1);
    auto d2 = manager.getFigure(p2);
    ASSERT_TRUE(d1.has_value() && d2.has_value());

    EXPECT_NEAR(d1->y.value(), d2->y.value(), 0.1);
}

TEST_F(DCMManagerSolveTest, LocalSolve_SingleComponent) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(1.0, 0.0));
    auto p3 = manager.addFigure(FigureDescriptor::point(100.0, 100.0));
    auto p4 = manager.addFigure(FigureDescriptor::point(103.0, 100.0));

    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 10.0));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p3, p4, 20.0));

    EXPECT_EQ(manager.getComponentCount(), 2);

    auto comp12 = manager.getComponentForFigure(p1);
    ASSERT_TRUE(comp12.has_value());

    auto d3before = manager.getFigure(p3);
    auto d4before = manager.getFigure(p4);

    manager.setSolveMode(SolveMode::LOCAL);
    bool converged = manager.solve(comp12.value());
    EXPECT_TRUE(converged);

    auto d1 = manager.getFigure(p1);
    auto d2 = manager.getFigure(p2);
    ASSERT_TRUE(d1.has_value() && d2.has_value());

    double dx = d2->x.value() - d1->x.value();
    double dy = d2->y.value() - d1->y.value();
    double dist = std::sqrt(dx * dx + dy * dy);
    EXPECT_NEAR(dist, 10.0, 0.1);

    auto d3after = manager.getFigure(p3);
    auto d4after = manager.getFigure(p4);
    ASSERT_TRUE(d3after.has_value() && d4after.has_value());
    EXPECT_DOUBLE_EQ(d3after->x.value(), d3before->x.value());
    EXPECT_DOUBLE_EQ(d3after->y.value(), d3before->y.value());
    EXPECT_DOUBLE_EQ(d4after->x.value(), d4before->x.value());
    EXPECT_DOUBLE_EQ(d4after->y.value(), d4before->y.value());
}

TEST_F(DCMManagerSolveTest, DragMode_AutoSolveOnUpdate) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(5.0, 0.0));

    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 5.0));

    manager.setSolveMode(SolveMode::DRAG);
    manager.updatePoint(PointUpdateDescriptor(p1, 2.0, 0.0));

    auto d1 = manager.getFigure(p1);
    auto d2 = manager.getFigure(p2);
    ASSERT_TRUE(d1.has_value() && d2.has_value());

    double dx = d2->x.value() - d1->x.value();
    double dy = d2->y.value() - d1->y.value();
    double dist = std::sqrt(dx * dx + dy * dy);
    EXPECT_NEAR(dist, 5.0, 0.5);
}

TEST_F(DCMManagerSolveTest, GlobalSolve_Rectangle) {
    auto l1 = manager.addFigure(FigureDescriptor::line(0.0, 0.0, 100.0, 2.0));
    auto l2 = manager.addFigure(FigureDescriptor::line(100.0, 2.0, 102.0, 52.0));
    auto l3 = manager.addFigure(FigureDescriptor::line(102.0, 52.0, -1.0, 49.0));
    auto l4 = manager.addFigure(FigureDescriptor::line(-1.0, 49.0, 0.0, 0.0));

    auto dL1 = manager.getFigure(l1);
    auto dL2 = manager.getFigure(l2);
    ASSERT_TRUE(dL1.has_value() && dL2.has_value());
    auto p1 = dL1->pointIds[0];
    auto p2 = dL1->pointIds[1];
    auto p3 = dL2->pointIds[1];

    manager.addRequirement(RequirementDescriptor::horizontal(l1));
    manager.addRequirement(RequirementDescriptor::horizontal(l3));
    manager.addRequirement(RequirementDescriptor::vertical(l2));
    manager.addRequirement(RequirementDescriptor::vertical(l4));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 100.0));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p2, p3, 50.0));

    manager.setSolveMode(SolveMode::GLOBAL);
    bool converged = manager.solve();
    EXPECT_TRUE(converged);

    auto d1 = manager.getFigure(p1);
    auto d2 = manager.getFigure(p2);
    auto d3 = manager.getFigure(p3);
    ASSERT_TRUE(d1.has_value() && d2.has_value() && d3.has_value());

    EXPECT_NEAR(d1->y.value(), d2->y.value(), 0.5);

    double dx12 = d2->x.value() - d1->x.value();
    double dy12 = d2->y.value() - d1->y.value();
    EXPECT_NEAR(std::sqrt(dx12 * dx12 + dy12 * dy12), 100.0, 1.0);

    double dx23 = d3->x.value() - d2->x.value();
    double dy23 = d3->y.value() - d2->y.value();
    EXPECT_NEAR(std::sqrt(dx23 * dx23 + dy23 * dy23), 50.0, 1.0);
}

TEST_F(DCMManagerSolveTest, SolveEmptySystem) {
    manager.setSolveMode(SolveMode::GLOBAL);
    EXPECT_TRUE(manager.solve());
}

TEST_F(DCMManagerSolveTest, LocalSolve_ThrowsWithoutComponentId) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(5.0, 0.0));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 10.0));

    manager.setSolveMode(SolveMode::LOCAL);
    EXPECT_THROW(manager.solve(), std::runtime_error);
}
