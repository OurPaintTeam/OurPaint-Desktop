#include <gtest/gtest.h>

#include "Scene.h"
#include "Painter.h"
#include "ID.h"

class SceneFixture : public testing::Test {
protected:
    Scene scene;
    Painter* p;

    SceneFixture() : p(nullptr), scene(nullptr) {}

    void SetUp() override {
        //scene.setPainter(p);
    }

    void TearDown() override {
        delete p;
    }
};

TEST_F(SceneFixture, add_object) {
    ObjectData obj1;
    obj1.et = ET_POINT;
    obj1.params = {0, 0};

    ID id = scene.addObject(obj1);
    ObjectData obj2 = scene.getObjectData(id);
    EXPECT_EQ(obj1, obj2);

    scene.deleteObject(id);
    EXPECT_ANY_THROW(scene.getObjectData(id));
}

TEST_F(SceneFixture, boundingBox_and_moveObject) {
    ObjectData obj1;
    obj1.et = ET_POINT;
    obj1.params = {0, 0};

    ObjectData obj2;
    obj2.et = ET_POINT;
    obj2.params = {1000, 0};

    ObjectData obj3;
    obj3.et = ET_POINT;
    obj3.params = {0, 1000};

    ObjectData obj4;
    obj4.et = ET_POINT;
    obj4.params = {1000, 1000};

    scene.addObject(obj1);
    scene.addObject(obj2);
    scene.addObject(obj3);
    scene.addObject(obj4);

    //std::unordered_map<ID, IGeometricObject*> allObjects = scene.getAllObjects();
    //EXPECT_EQ(allObjects.size(), 4);

    //scene.updateBoundingBox();

    BoundBox2D bbox = scene.getBoundingBox();
    EXPECT_EQ(bbox.min_x, 0);
    EXPECT_EQ(bbox.max_x, 1000);
    EXPECT_EQ(bbox.min_y, 0);
    EXPECT_EQ(bbox.max_y, 1000);

    scene.moveObject(ID(4), 5000, 5000);
    bbox = scene.getBoundingBox();
    EXPECT_EQ(bbox.min_x, 0);
    EXPECT_EQ(bbox.max_x, 6000);
    EXPECT_EQ(bbox.min_y, 0);
    EXPECT_EQ(bbox.max_y, 6000);

    scene.deleteObject(ID(4));
    bbox = scene.getBoundingBox();
    EXPECT_EQ(bbox.min_x, 0);
    EXPECT_EQ(bbox.max_x, 1000);
    EXPECT_EQ(bbox.min_y, 0);
    EXPECT_EQ(bbox.max_y, 1000);

    EXPECT_EQ(bbox.width(), 1000);
    EXPECT_EQ(bbox.height(), 1000);

    EXPECT_EQ(scene.vertexCount(), 3);
    EXPECT_EQ(scene.edgeCount(), 0);
}
