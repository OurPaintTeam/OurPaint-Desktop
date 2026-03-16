#include <gtest/gtest.h>
#include "DCMManager.h"

using namespace OurPaintDCM;
using namespace OurPaintDCM::Utils;

class DCMManagerTest : public ::testing::Test {
protected:
    DCMManager manager;
};

TEST_F(DCMManagerTest, DefaultConstruction) {
    EXPECT_EQ(manager.figureCount(), 0);
    EXPECT_EQ(manager.requirementCount(), 0);
    EXPECT_EQ(manager.getComponentCount(), 0);
}

TEST_F(DCMManagerTest, AddPointFigure) {
    auto id = manager.addFigure(FigureDescriptor::point(10.0, 20.0));

    EXPECT_TRUE(manager.hasFigure(id));
    EXPECT_EQ(manager.figureCount(), 1);

    auto desc = manager.getFigure(id);
    ASSERT_TRUE(desc.has_value());
    EXPECT_EQ(desc->type, FigureType::ET_POINT2D);
    EXPECT_DOUBLE_EQ(desc->x.value(), 10.0);
    EXPECT_DOUBLE_EQ(desc->y.value(), 20.0);
    EXPECT_EQ(manager.getComponentCount(), 1);
}

TEST_F(DCMManagerTest, AddLineFigure) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto line = manager.addFigure(FigureDescriptor::line(0.0, 0.0, 10.0, 0.0));

    EXPECT_TRUE(manager.hasFigure(line));
    EXPECT_EQ(manager.figureCount(), 5);

    auto desc = manager.getFigure(line);
    ASSERT_TRUE(desc.has_value());
    EXPECT_EQ(desc->type, FigureType::ET_LINE);
    EXPECT_EQ(desc->pointIds.size(), 2);
}

TEST_F(DCMManagerTest, AddCircleFigure) {
    auto circle = manager.addFigure(FigureDescriptor::circle(5.0, 5.0, 10.0));

    EXPECT_TRUE(manager.hasFigure(circle));

    auto desc = manager.getFigure(circle);
    ASSERT_TRUE(desc.has_value());
    EXPECT_EQ(desc->type, FigureType::ET_CIRCLE);
    EXPECT_DOUBLE_EQ(desc->radius.value(), 10.0);
}

TEST_F(DCMManagerTest, AddArcFigure) {
    auto arc = manager.addFigure(FigureDescriptor::arc(0.0, 0.0, 10.0, 0.0, 5.0, 5.0));

    EXPECT_TRUE(manager.hasFigure(arc));

    auto desc = manager.getFigure(arc);
    ASSERT_TRUE(desc.has_value());
    EXPECT_EQ(desc->type, FigureType::ET_ARC);
}

TEST_F(DCMManagerTest, RemoveFigure) {
    auto id = manager.addFigure(FigureDescriptor::point(10.0, 20.0));
    EXPECT_TRUE(manager.hasFigure(id));

    manager.removeFigure(id);
    EXPECT_FALSE(manager.hasFigure(id));
    EXPECT_EQ(manager.figureCount(), 0);
}

TEST_F(DCMManagerTest, GetFigureUpdatedCoordinates) {
    auto pointId = manager.addFigure(FigureDescriptor::point(15.0, 25.0));

    manager.updatePoint(PointUpdateDescriptor(pointId, 100.0, 200.0));

    auto desc = manager.getFigure(pointId);
    ASSERT_TRUE(desc.has_value());
    EXPECT_DOUBLE_EQ(desc->x.value(), 100.0);
    EXPECT_DOUBLE_EQ(desc->y.value(), 200.0);
}

TEST_F(DCMManagerTest, GetFigureNotFound) {
    auto desc = manager.getFigure(ID(999));
    EXPECT_FALSE(desc.has_value());
}

TEST_F(DCMManagerTest, UpdatePoint) {
    auto id = manager.addFigure(FigureDescriptor::point(10.0, 20.0));

    manager.updatePoint(PointUpdateDescriptor(id, 30.0, 40.0));

    auto desc = manager.getFigure(id);
    ASSERT_TRUE(desc.has_value());
    EXPECT_DOUBLE_EQ(desc->x.value(), 30.0);
    EXPECT_DOUBLE_EQ(desc->y.value(), 40.0);
}

TEST_F(DCMManagerTest, UpdateCircle) {
    auto circleId = manager.addFigure(FigureDescriptor::circle(5.0, 5.0, 10.0));

    manager.updateCircle(CircleUpdateDescriptor(circleId, 25.0));

    auto desc = manager.getFigure(circleId);
    ASSERT_TRUE(desc.has_value());
    EXPECT_DOUBLE_EQ(desc->radius.value(), 25.0);
}

TEST_F(DCMManagerTest, GetAllFigures) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto p3 = manager.addFigure(FigureDescriptor::point(5.0, 5.0));

    auto figures = manager.getAllFigures();
    EXPECT_EQ(figures.size(), 3);
}

TEST_F(DCMManagerTest, AddRequirement) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));

    auto reqId = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    EXPECT_TRUE(manager.hasRequirement(reqId));
    EXPECT_EQ(manager.requirementCount(), 1);
}

TEST_F(DCMManagerTest, RemoveRequirement) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto reqId = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    manager.removeRequirement(reqId);

    EXPECT_FALSE(manager.hasRequirement(reqId));
    EXPECT_EQ(manager.requirementCount(), 0);
}

TEST_F(DCMManagerTest, UpdateRequirementParam) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto reqId = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    manager.updateRequirementParam(reqId, 100.0);

    auto desc = manager.getRequirement(reqId);
    ASSERT_TRUE(desc.has_value());
    ASSERT_TRUE(desc->param.has_value());
    EXPECT_DOUBLE_EQ(desc->param.value(), 100.0);
}

TEST_F(DCMManagerTest, GetRequirement) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto line = manager.addFigure(FigureDescriptor::line(0.0, 0.0, 10.0, 0.0));

    auto req1 = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));
    auto req2 = manager.addRequirement(RequirementDescriptor::horizontal(line));

    auto desc1 = manager.getRequirement(req1);
    auto desc2 = manager.getRequirement(req2);

    ASSERT_TRUE(desc1.has_value());
    ASSERT_TRUE(desc2.has_value());

    EXPECT_EQ(desc1->type, RequirementType::ET_POINTPOINTDIST);
    EXPECT_EQ(desc1->objectIds.size(), 2);
    EXPECT_EQ(desc1->objectIds[0], p1);
    EXPECT_EQ(desc1->objectIds[1], p2);
    EXPECT_DOUBLE_EQ(desc1->param.value(), 50.0);

    EXPECT_EQ(desc2->type, RequirementType::ET_HORIZONTAL);
}

TEST_F(DCMManagerTest, GetRequirementNotFound) {
    auto desc = manager.getRequirement(ID(999));
    EXPECT_FALSE(desc.has_value());
}

TEST_F(DCMManagerTest, GetAllRequirements) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto line = manager.addFigure(FigureDescriptor::line(0.0, 0.0, 10.0, 0.0));

    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));
    manager.addRequirement(RequirementDescriptor::horizontal(line));
    manager.addRequirement(RequirementDescriptor::pointOnLine(p1, line));

    auto reqs = manager.getAllRequirements();
    EXPECT_EQ(reqs.size(), 3);
}

TEST_F(DCMManagerTest, ComponentsInitialState) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto p3 = manager.addFigure(FigureDescriptor::point(20.0, 0.0));

    EXPECT_EQ(manager.getComponentCount(), 3);

    auto comp1 = manager.getComponentForFigure(p1);
    auto comp2 = manager.getComponentForFigure(p2);
    auto comp3 = manager.getComponentForFigure(p3);

    EXPECT_TRUE(comp1.has_value());
    EXPECT_TRUE(comp2.has_value());
    EXPECT_TRUE(comp3.has_value());

    EXPECT_NE(comp1.value(), comp2.value());
    EXPECT_NE(comp2.value(), comp3.value());
}

TEST_F(DCMManagerTest, ComponentsMergeOnRequirement) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));

    EXPECT_EQ(manager.getComponentCount(), 2);

    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    EXPECT_EQ(manager.getComponentCount(), 1);

    auto comp1 = manager.getComponentForFigure(p1);
    auto comp2 = manager.getComponentForFigure(p2);

    EXPECT_TRUE(comp1.has_value());
    EXPECT_TRUE(comp2.has_value());
    EXPECT_EQ(comp1.value(), comp2.value());
}

TEST_F(DCMManagerTest, ComponentsSplitOnRequirementRemoval) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));

    auto reqId = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));
    EXPECT_EQ(manager.getComponentCount(), 1);

    manager.removeRequirement(reqId);
    EXPECT_EQ(manager.getComponentCount(), 2);
}

TEST_F(DCMManagerTest, GetFiguresInComponent) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));

    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    auto compId = manager.getComponentForFigure(p1);
    ASSERT_TRUE(compId.has_value());

    auto figures = manager.getFiguresInComponent(compId.value());
    EXPECT_EQ(figures.size(), 2);
}

TEST_F(DCMManagerTest, GetRequirementsInComponent) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));

    auto req1 = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));
    auto req2 = manager.addRequirement(RequirementDescriptor::pointOnPoint(p1, p2));

    auto compId = manager.getComponentForFigure(p1);
    ASSERT_TRUE(compId.has_value());

    auto reqs = manager.getRequirementsInComponent(compId.value());
    EXPECT_EQ(reqs.size(), 2);
}

TEST_F(DCMManagerTest, GetAllComponents) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto p3 = manager.addFigure(FigureDescriptor::point(20.0, 0.0));
    auto p4 = manager.addFigure(FigureDescriptor::point(30.0, 0.0));

    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 10.0));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p3, p4, 10.0));

    auto components = manager.getAllComponents();
    EXPECT_EQ(components.size(), 2);
}

TEST_F(DCMManagerTest, MergeMultipleComponentsAtOnce) {
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));

    EXPECT_EQ(manager.getComponentCount(), 1);

    auto line = manager.addFigure(FigureDescriptor::line(0.0, 0.0, 20.0, 0.0));
    manager.addRequirement(RequirementDescriptor::pointOnLine(p2, line));

    EXPECT_LE(manager.getComponentCount(), 2);
}

TEST_F(DCMManagerTest, ClearAll) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    manager.clear();

    EXPECT_EQ(manager.figureCount(), 0);
    EXPECT_EQ(manager.requirementCount(), 0);
    EXPECT_EQ(manager.getComponentCount(), 0);
}

TEST_F(DCMManagerTest, StorageAccess) {
    auto p = manager.addFigure(FigureDescriptor::point(5.0, 10.0));

    const auto& constStorage = manager.getStorage();
    auto& mutableStorage = manager.storage();

    EXPECT_TRUE(constStorage.contains(p));
    EXPECT_TRUE(mutableStorage.contains(p));
}

TEST_F(DCMManagerTest, RequirementSystemAccess) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    const auto& constReqSys = manager.getRequirementSystem();
    auto& mutableReqSys = manager.requirementSystem();

    EXPECT_FALSE(constReqSys.getAllVars().empty());
    EXPECT_FALSE(mutableReqSys.getAllVars().empty());
}

TEST_F(DCMManagerTest, FigureDescriptorValidation) {
    FigureDescriptor invalid;
    invalid.type = FigureType::ET_POINT2D;

    EXPECT_THROW(manager.addFigure(invalid), std::invalid_argument);
}

TEST_F(DCMManagerTest, RequirementDescriptorValidation) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));

    RequirementDescriptor invalid;
    invalid.type = RequirementType::ET_POINTPOINTDIST;
    invalid.objectIds = {p1};

    EXPECT_THROW(manager.addRequirement(invalid), std::invalid_argument);
}

TEST_F(DCMManagerTest, RemoveNonExistentFigure) {
    EXPECT_THROW(manager.removeFigure(ID(999)), std::runtime_error);
}

TEST_F(DCMManagerTest, RemoveNonExistentRequirement) {
    EXPECT_THROW(manager.removeRequirement(ID(999)), std::runtime_error);
}

TEST_F(DCMManagerTest, UpdateNonExistentPoint) {
    EXPECT_THROW(manager.updatePoint(PointUpdateDescriptor(ID(999), 0.0, 0.0)), std::runtime_error);
}

TEST_F(DCMManagerTest, UpdateNonExistentCircle) {
    EXPECT_THROW(manager.updateCircle(CircleUpdateDescriptor(ID(999), 10.0)), std::runtime_error);
}

TEST_F(DCMManagerTest, RemoveFigureWithCascade) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto reqId = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    manager.removeFigure(p1, true);

    EXPECT_FALSE(manager.hasFigure(p1));
    EXPECT_FALSE(manager.hasRequirement(reqId));
}

TEST_F(DCMManagerTest, ComplexScenario) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(100.0, 0.0));
    auto p3 = manager.addFigure(FigureDescriptor::point(100.0, 100.0));
    auto p4 = manager.addFigure(FigureDescriptor::point(0.0, 100.0));

    auto l1 = manager.addFigure(FigureDescriptor::line(0.0, 0.0, 100.0, 0.0));
    auto l2 = manager.addFigure(FigureDescriptor::line(100.0, 0.0, 100.0, 100.0));
    auto l3 = manager.addFigure(FigureDescriptor::line(100.0, 100.0, 0.0, 100.0));
    auto l4 = manager.addFigure(FigureDescriptor::line(0.0, 100.0, 0.0, 0.0));

    manager.addRequirement(RequirementDescriptor::horizontal(l1));
    manager.addRequirement(RequirementDescriptor::horizontal(l3));
    manager.addRequirement(RequirementDescriptor::vertical(l2));
    manager.addRequirement(RequirementDescriptor::vertical(l4));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 100.0));
    manager.addRequirement(RequirementDescriptor::pointPointDist(p2, p3, 100.0));

    EXPECT_EQ(manager.figureCount(), 16);
    EXPECT_EQ(manager.requirementCount(), 6);
    EXPECT_EQ(manager.getComponentCount(), 6);
}

TEST_F(DCMManagerTest, FigureDescriptorHasId) {
    auto id = manager.addFigure(FigureDescriptor::point(10.0, 20.0));

    auto desc = manager.getFigure(id);
    ASSERT_TRUE(desc.has_value());
    ASSERT_TRUE(desc->id.has_value());
    EXPECT_EQ(desc->id.value(), id);
}

TEST_F(DCMManagerTest, RequirementDescriptorHasId) {
    auto p1 = manager.addFigure(FigureDescriptor::point(0.0, 0.0));
    auto p2 = manager.addFigure(FigureDescriptor::point(10.0, 0.0));
    auto reqId = manager.addRequirement(RequirementDescriptor::pointPointDist(p1, p2, 50.0));

    auto desc = manager.getRequirement(reqId);
    ASSERT_TRUE(desc.has_value());
    ASSERT_TRUE(desc->id.has_value());
    EXPECT_EQ(desc->id.value(), reqId);
}
