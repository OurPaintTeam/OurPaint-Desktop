#include <gtest/gtest.h>
#include "RequirementSystem.h"
#include "GeometryStorage.h"

using namespace OurPaintDCM::System;
using namespace OurPaintDCM::Figures;
using namespace OurPaintDCM::Utils;

class RequirementSystemTest : public ::testing::Test {
protected:
    GeometryStorage storage;
    ID p1Id, p2Id, p3Id, centerId;
    ID line1Id, line2Id;
    ID circleId;
    ID arcId;

    void SetUp() override {
        auto [id1, pt1] = storage.createPoint(0.0, 0.0);
        auto [id2, pt2] = storage.createPoint(3.0, 4.0);
        auto [id3, pt3] = storage.createPoint(6.0, 0.0);
        auto [id4, pt4] = storage.createPoint(5.0, 5.0);

        p1Id = id1;
        p2Id = id2;
        p3Id = id3;
        centerId = id4;

        auto [lid1, l1] = storage.createLine(pt1, pt2);
        auto [lid2, l2] = storage.createLine(pt2, pt3);
        line1Id = lid1;
        line2Id = lid2;

        auto [cid, c] = storage.createCircle(pt4, 10.0);
        circleId = cid;

        auto [aid, a] = storage.createArc(pt1, pt3, pt4);
        arcId = aid;
    }
};

TEST_F(RequirementSystemTest, ConstructWithStorage) {
    RequirementSystem system(&storage);
    EXPECT_TRUE(system.getAllVars().empty());
}

TEST_F(RequirementSystemTest, AddPointPointDist) {
    RequirementSystem system(&storage);
    system.addPointPointDist(p1Id, p2Id, 5.0);
    
    EXPECT_EQ(system.getAllVars().size(), 4);
    
    auto residuals = system.residuals();
    EXPECT_EQ(residuals.size(), 1);
    EXPECT_NEAR(residuals[0], 0.0, 1e-9);
}

TEST_F(RequirementSystemTest, AddPointOnPoint) {
    RequirementSystem system(&storage);
    system.addPointOnPoint(p1Id, p1Id);
    
    auto residuals = system.residuals();
    EXPECT_NEAR(residuals[0], 0.0, 1e-9);
}

TEST_F(RequirementSystemTest, AddPointLineDist) {
    RequirementSystem system(&storage);
    system.addPointLineDist(p3Id, line1Id, 2.4);
    
    EXPECT_EQ(system.getAllVars().size(), 6);
}

TEST_F(RequirementSystemTest, AddPointOnLine) {
    RequirementSystem system(&storage);
    system.addPointOnLine(p1Id, line1Id);
    
    auto residuals = system.residuals();
    EXPECT_NEAR(residuals[0], 0.0, 1e-9);
}

TEST_F(RequirementSystemTest, AddLineCircleDist) {
    RequirementSystem system(&storage);
    system.addLineCircleDist(line1Id, circleId, 1.0);
    
    EXPECT_EQ(system.getAllVars().size(), 7);
}

TEST_F(RequirementSystemTest, AddLineOnCircle) {
    RequirementSystem system(&storage);
    system.addLineOnCircle(line1Id, circleId);
    
    EXPECT_EQ(system.getAllVars().size(), 7);
}

TEST_F(RequirementSystemTest, AddLineLineParallel) {
    RequirementSystem system(&storage);
    system.addLineLineParallel(line1Id, line2Id);
    
    EXPECT_EQ(system.getAllVars().size(), 6);
}

TEST_F(RequirementSystemTest, AddLineLinePerpendicular) {
    RequirementSystem system(&storage);
    system.addLineLinePerpendicular(line1Id, line2Id);
    
    EXPECT_EQ(system.getAllVars().size(), 6);
}

TEST_F(RequirementSystemTest, AddLineLineAngle) {
    RequirementSystem system(&storage);
    system.addLineLineAngle(line1Id, line2Id, 1.57);
    
    EXPECT_EQ(system.getAllVars().size(), 6);
}

TEST_F(RequirementSystemTest, AddVertical) {
    RequirementSystem system(&storage);
    system.addVertical(line1Id);
    
    EXPECT_EQ(system.getAllVars().size(), 4);
}

TEST_F(RequirementSystemTest, AddHorizontal) {
    RequirementSystem system(&storage);
    system.addHorizontal(line1Id);
    
    EXPECT_EQ(system.getAllVars().size(), 4);
}

TEST_F(RequirementSystemTest, AddArcCenterOnPerpendicular) {
    RequirementSystem system(&storage);
    system.addArcCenterOnPerpendicular(arcId);
    
    EXPECT_EQ(system.getAllVars().size(), 6);
}

TEST_F(RequirementSystemTest, UpdateJacobianAndGet) {
    RequirementSystem system(&storage);
    system.addPointPointDist(p1Id, p2Id, 5.0);
    system.updateJ();
    
    auto J = system.J();
    EXPECT_EQ(J.rows(), 1);
    EXPECT_EQ(J.cols(), 4);
}

TEST_F(RequirementSystemTest, JTJComputation) {
    RequirementSystem system(&storage);
    system.addPointPointDist(p1Id, p2Id, 5.0);
    system.updateJ();
    
    auto JTJ = system.JTJ();
    EXPECT_EQ(JTJ.rows(), 4);
    EXPECT_EQ(JTJ.cols(), 4);
}

TEST_F(RequirementSystemTest, DiagnoseUnderConstrained) {
    RequirementSystem system(&storage);
    system.addHorizontal(line1Id);
    system.updateJ();
    
    auto status = system.diagnose();
    EXPECT_EQ(status, SystemStatus::UNDER_CONSTRAINED);
}

TEST_F(RequirementSystemTest, Clear) {
    RequirementSystem system(&storage);
    system.addPointPointDist(p1Id, p2Id, 5.0);
    system.updateJ();
    
    system.clear();
    
    EXPECT_TRUE(system.getAllVars().empty());
    EXPECT_EQ(system.J().rows(), 0);
}

TEST_F(RequirementSystemTest, MultipleConstraints) {
    RequirementSystem system(&storage);
    
    system.addPointPointDist(p1Id, p2Id, 5.0);
    system.addHorizontal(line1Id);
    system.addVertical(line2Id);
    
    EXPECT_EQ(system.residuals().size(), 3);
    
    system.updateJ();
    auto J = system.J();
    EXPECT_EQ(J.rows(), 3);
}

TEST_F(RequirementSystemTest, SharedVariablesBetweenConstraints) {
    RequirementSystem system(&storage);
    
    system.addPointPointDist(p1Id, p2Id, 5.0);
    system.addPointOnLine(p1Id, line1Id);
    
    auto vars = system.getAllVars();
    EXPECT_LT(vars.size(), 10);
}

TEST_F(RequirementSystemTest, InvalidIdThrows) {
    RequirementSystem system(&storage);
    
    EXPECT_THROW(system.addPointPointDist(ID(999), p2Id, 5.0), std::runtime_error);
    EXPECT_THROW(system.addHorizontal(ID(999)), std::runtime_error);
}

TEST_F(RequirementSystemTest, BuildDependencyGraphEmpty) {
    RequirementSystem system(&storage);
    
    auto graph = system.buildDependencyGraph();
    
    EXPECT_TRUE(graph.hasVertex(p1Id));
    EXPECT_TRUE(graph.hasVertex(p2Id));
    EXPECT_TRUE(graph.hasVertex(p3Id));
    EXPECT_TRUE(graph.hasVertex(line1Id));
    EXPECT_TRUE(graph.hasVertex(line2Id));
    EXPECT_TRUE(graph.hasVertex(circleId));
    EXPECT_TRUE(graph.hasVertex(arcId));
}

TEST_F(RequirementSystemTest, BuildDependencyGraphWithConstraints) {
    RequirementSystem system(&storage);
    
    system.addPointPointDist(p1Id, p2Id, 5.0);
    system.addHorizontal(line1Id);
    system.addLineLineParallel(line1Id, line2Id);
    
    auto graph = system.buildDependencyGraph();
    
    EXPECT_TRUE(graph.hasEdge(p1Id, p2Id));
    EXPECT_TRUE(graph.hasEdge(line1Id, line2Id));

    EXPECT_GE(graph.edgeCount(), 2u);
}

TEST_F(RequirementSystemTest, BuildDependencyGraphEdgeWeightIsConstraintId) {
    RequirementSystem system(&storage);
    
    system.addPointPointDist(p1Id, p2Id, 5.0);
    
    auto graph = system.buildDependencyGraph();
    
    EXPECT_TRUE(graph.hasEdge(p1Id, p2Id));

    auto weight = graph.getEdgeWeight(p1Id, p2Id);
    EXPECT_EQ(weight.id, 1u);
}

TEST_F(RequirementSystemTest, BuildDependencyGraphMultiObjectConstraint) {
    RequirementSystem system(&storage);
    
    system.addPointOnLine(p3Id, line1Id);
    
    auto graph = system.buildDependencyGraph();
    
    EXPECT_TRUE(graph.hasEdge(p3Id, line1Id));
}
