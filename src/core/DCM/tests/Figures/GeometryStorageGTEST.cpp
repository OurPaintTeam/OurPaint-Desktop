#include <gtest/gtest.h>
#include <algorithm>
#include <stdexcept>

#include "GeometryStorage.h"

using namespace OurPaintDCM::Figures;

namespace {
bool containsId(const std::vector<ID>& ids, ID id) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}
}

TEST(GeometryStorageTest, CreatePointAndAccess) {
    GeometryStorage storage;

    auto [id, p] = storage.createPoint(1.5, -2.0);
    ASSERT_NE(p, nullptr);

    EXPECT_TRUE(storage.contains(id));
    EXPECT_EQ(storage.getType(id).value(), FigureType::ET_POINT2D);
    EXPECT_EQ(storage.getEntry(id)->type, FigureType::ET_POINT2D);
    EXPECT_EQ(storage.getRaw(id), static_cast<void*>(p));

    auto point = storage.get<Point2D>(id);
    ASSERT_NE(point, nullptr);
    EXPECT_DOUBLE_EQ(point->x(), 1.5);
    EXPECT_DOUBLE_EQ(point->y(), -2.0);

    EXPECT_EQ(storage.size(), 1u);
    EXPECT_EQ(storage.pointCount(), 1u);
    EXPECT_TRUE(storage.getIDsByType(FigureType::ET_POINT2D).size() == 1u);
    EXPECT_TRUE(storage.allEntries().size() == 1u);
}

TEST(GeometryStorageTest, CreateLineCircleArcAndDependencies) {
    GeometryStorage storage;

    auto [p1Id, p1] = storage.createPoint(0.0, 0.0);
    auto [p2Id, p2] = storage.createPoint(3.0, 0.0);
    auto [cId, c] = storage.createPoint(1.0, 1.0);

    auto [lineId, line] = storage.createLine(p1, p2);
    auto [circleId, circle] = storage.createCircle(c, 5.0);
    auto [arcId, arc] = storage.createArc(p1, p2, c);

    auto lineDeps = storage.getDependencies(lineId);
    EXPECT_EQ(lineDeps.size(), 2u);
    EXPECT_TRUE(containsId(lineDeps, p1Id));
    EXPECT_TRUE(containsId(lineDeps, p2Id));

    auto circleDeps = storage.getDependencies(circleId);
    EXPECT_EQ(circleDeps.size(), 1u);
    EXPECT_TRUE(containsId(circleDeps, cId));

    auto arcDeps = storage.getDependencies(arcId);
    EXPECT_EQ(arcDeps.size(), 3u);
    EXPECT_TRUE(containsId(arcDeps, p1Id));
    EXPECT_TRUE(containsId(arcDeps, p2Id));
    EXPECT_TRUE(containsId(arcDeps, cId));

    auto p1Dependents = storage.getDependents(p1Id);
    EXPECT_EQ(p1Dependents.size(), 2u);
    EXPECT_TRUE(containsId(p1Dependents, lineId));
    EXPECT_TRUE(containsId(p1Dependents, arcId));

    auto cDependents = storage.getDependents(cId);
    EXPECT_EQ(cDependents.size(), 2u);
    EXPECT_TRUE(containsId(cDependents, circleId));
    EXPECT_TRUE(containsId(cDependents, arcId));
}

TEST(GeometryStorageTest, CreateFigureFromData) {
    GeometryStorage storage;

    FigureData pointData;
    pointData.points.push_back({1.0, 2.0});
    auto pointId = storage.createFigure(FigureType::ET_POINT2D, pointData);
    EXPECT_EQ(storage.getType(pointId).value(), FigureType::ET_POINT2D);

    FigureData lineData;
    lineData.points.push_back({0.0, 0.0});
    lineData.points.push_back({1.0, 0.0});
    auto lineId = storage.createFigure(FigureType::ET_LINE, lineData);
    EXPECT_EQ(storage.getType(lineId).value(), FigureType::ET_LINE);

    FigureData circleData;
    circleData.center = {5.0, 5.0};
    circleData.radius = 2.5;
    auto circleId = storage.createFigure(FigureType::ET_CIRCLE, circleData);
    EXPECT_EQ(storage.getType(circleId).value(), FigureType::ET_CIRCLE);

    FigureData arcData;
    arcData.points.push_back({0.0, 0.0});
    arcData.points.push_back({2.0, 0.0});
    arcData.center = {1.0, 1.0};
    auto arcId = storage.createFigure(FigureType::ET_ARC, arcData);
    EXPECT_EQ(storage.getType(arcId).value(), FigureType::ET_ARC);
}

TEST(GeometryStorageTest, CreateFigureInvalidData) {
    GeometryStorage storage;

    FigureData emptyPoints;
    EXPECT_THROW((void)storage.createFigure(FigureType::ET_POINT2D, emptyPoints), std::runtime_error);

    FigureData onePoint;
    onePoint.points.push_back({0.0, 0.0});
    EXPECT_THROW((void)storage.createFigure(FigureType::ET_LINE, onePoint), std::runtime_error);
    EXPECT_THROW((void)storage.createFigure(FigureType::ET_ARC, onePoint), std::runtime_error);
    EXPECT_THROW((void)storage.createFigure(static_cast<FigureType>(255), onePoint), std::runtime_error);
}

TEST(GeometryStorageTest, RemoveAndCascade) {
    GeometryStorage storage;

    auto [p1Id, p1] = storage.createPoint(0.0, 0.0);
    auto [p2Id, p2] = storage.createPoint(1.0, 0.0);
    auto [lineId, line] = storage.createLine(p1, p2);

    EXPECT_THROW(storage.remove(p1Id, false), std::runtime_error);

    storage.remove(p1Id, true);
    EXPECT_FALSE(storage.contains(p1Id));
    EXPECT_FALSE(storage.contains(lineId));
    EXPECT_TRUE(storage.contains(p2Id));
}

TEST(GeometryStorageTest, RemoveNotFound) {
    GeometryStorage storage;
    EXPECT_THROW(storage.remove(ID(123)), std::runtime_error);
}

TEST(GeometryStorageTest, ClearAndStats) {
    GeometryStorage storage;

    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.currentID().id, 1ull);

    [[maybe_unused]] auto _p1 = storage.createPoint(0.0, 0.0);
    [[maybe_unused]] auto _p2 = storage.createPoint(1.0, 1.0);
    EXPECT_FALSE(storage.empty());
    EXPECT_EQ(storage.pointCount(), 2u);
    EXPECT_EQ(storage.currentID().id, 3ull);

    storage.clear();
    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0u);
    EXPECT_EQ(storage.pointCount(), 0u);
    EXPECT_EQ(storage.lineCount(), 0u);
    EXPECT_EQ(storage.circleCount(), 0u);
    EXPECT_EQ(storage.arcCount(), 0u);
    EXPECT_EQ(storage.currentID().id, 1ull);
}

TEST(GeometryStorageTest, SpansAndCounts) {
    GeometryStorage storage;

    EXPECT_TRUE(storage.allPoints().empty());
    EXPECT_TRUE(storage.allLines().empty());
    EXPECT_TRUE(storage.allCircles().empty());
    EXPECT_TRUE(storage.allArcs().empty());

    auto [p1Id, p1] = storage.createPoint(0.0, 0.0);
    auto [p2Id, p2] = storage.createPoint(1.0, 0.0);
    auto [cId, c] = storage.createPoint(2.0, 2.0);
    [[maybe_unused]] auto _line = storage.createLine(p1, p2);
    [[maybe_unused]] auto _circle = storage.createCircle(c, 3.0);
    [[maybe_unused]] auto _arc = storage.createArc(p1, p2, c);

    EXPECT_EQ(storage.allPoints().size(), 3u);
    EXPECT_EQ(storage.allLines().size(), 1u);
    EXPECT_EQ(storage.allCircles().size(), 1u);
    EXPECT_EQ(storage.allArcs().size(), 1u);

    EXPECT_EQ(storage.pointCount(), 3u);
    EXPECT_EQ(storage.lineCount(), 1u);
    EXPECT_EQ(storage.circleCount(), 1u);
    EXPECT_EQ(storage.arcCount(), 1u);
}

TEST(GeometryStorageTest, BuildObjectGraph) {
    GeometryStorage storage;

    auto [p1Id, p1] = storage.createPoint(0.0, 0.0);
    auto [p2Id, p2] = storage.createPoint(1.0, 0.0);
    auto [cId, c] = storage.createPoint(2.0, 2.0);

    auto [lineId, line] = storage.createLine(p1, p2);
    auto [circleId, circle] = storage.createCircle(c, 3.0);
    auto [arcId, arc] = storage.createArc(p1, p2, c);

    auto graph = storage.buildObjectGraph();

    EXPECT_EQ(graph.vertexCount(), 6u);
    EXPECT_EQ(graph.edgeCount(), 6u);
    EXPECT_TRUE(graph.hasEdge(lineId, p1Id));
    EXPECT_TRUE(graph.hasEdge(lineId, p2Id));
    EXPECT_TRUE(graph.hasEdge(circleId, cId));
    EXPECT_TRUE(graph.hasEdge(arcId, p1Id));
    EXPECT_TRUE(graph.hasEdge(arcId, p2Id));
    EXPECT_TRUE(graph.hasEdge(arcId, cId));
}

TEST(GeometryStorageTest, BuildObjectSubgraph) {
    GeometryStorage storage;

    auto [p1Id, p1] = storage.createPoint(0.0, 0.0);
    auto [p2Id, p2] = storage.createPoint(1.0, 0.0);
    auto [lineId, line] = storage.createLine(p1, p2);
    auto [p3Id, p3] = storage.createPoint(5.0, 5.0);

    auto subgraph = storage.buildObjectSubgraph(lineId);
    EXPECT_EQ(subgraph.vertexCount(), 3u);
    EXPECT_TRUE(subgraph.hasEdge(lineId, p1Id));
    EXPECT_TRUE(subgraph.hasEdge(lineId, p2Id));

    auto pointGraph = storage.buildObjectSubgraph(p1Id);
    EXPECT_EQ(pointGraph.vertexCount(), 3u);
    EXPECT_TRUE(pointGraph.hasEdge(p1Id, lineId));
    EXPECT_TRUE(pointGraph.hasEdge(lineId, p2Id));

    auto isolated = storage.buildObjectSubgraph(p3Id);
    EXPECT_EQ(isolated.vertexCount(), 1u);
    EXPECT_FALSE(isolated.hasEdge(p3Id, p3Id));

    EXPECT_THROW(storage.buildObjectSubgraph(ID(999)), std::runtime_error);
}

TEST(GeometryStorageTest, SharedPointBetweenLines) {
    GeometryStorage storage;

    auto [pSharedId, pShared] = storage.createPoint(0.0, 0.0);
    auto [p1Id, p1] = storage.createPoint(1.0, 0.0);
    auto [p2Id, p2] = storage.createPoint(2.0, 0.0);

    auto [line1Id, line1] = storage.createLine(pShared, p1);
    auto [line2Id, line2] = storage.createLine(pShared, p2);

    auto dependents = storage.getDependents(pSharedId);
    EXPECT_EQ(dependents.size(), 2u);
    EXPECT_TRUE(std::find(dependents.begin(), dependents.end(), line1Id) != dependents.end());
    EXPECT_TRUE(std::find(dependents.begin(), dependents.end(), line2Id) != dependents.end());

    auto subgraph = storage.buildObjectSubgraph(pSharedId);
    EXPECT_EQ(subgraph.vertexCount(), 5u);
    EXPECT_TRUE(subgraph.hasEdge(pSharedId, line1Id));
    EXPECT_TRUE(subgraph.hasEdge(pSharedId, line2Id));
    EXPECT_TRUE(subgraph.hasEdge(line1Id, p1Id));
    EXPECT_TRUE(subgraph.hasEdge(line2Id, p2Id));
}

TEST(GeometryStorageTest, GetTypeMismatch) {
    GeometryStorage storage;
    auto [id, p] = storage.createPoint(0.0, 0.0);
    EXPECT_THROW((void)storage.get<Line2D>(id), std::runtime_error);
}

TEST(GeometryStorageTest, GetNotFound) {
    GeometryStorage storage;
    EXPECT_THROW((void)storage.get<Point2D>(ID(999)), std::runtime_error);
}

