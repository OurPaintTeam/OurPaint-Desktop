#include <gtest/gtest.h>

#include "objects/GeometricObjects.h"
#include "objects/BoundBox.h"
#include "objects/ID.h"

TEST(PointTest, RectMethod) {
    Point p;
    p.x = 3;
    p.y = 4;
    BoundBox2D r = p.getBox();
    EXPECT_EQ(r.min_x, 3);
    EXPECT_EQ(r.max_x, 3);
    EXPECT_EQ(r.min_y, 4);
    EXPECT_EQ(r.max_y, 4);
}

TEST(CircleTest, RectMethod) {
    Point center;
    center.x = 0;
    center.y = 0;
    Circle c;
    c.center = &center;
    c.r = 5;
    BoundBox2D r = c.getBox();
    EXPECT_EQ(r.min_x, -5);
    EXPECT_EQ(r.max_x, 5);
    EXPECT_EQ(r.min_y, -5);
    EXPECT_EQ(r.max_y, 5);
}

TEST(SectionTest, RectMethod) {
    Point beg;
    beg.x = 1;
    beg.y = 2;
    Point end;
    end.x = 3;
    end.y = 4;
    Section s;
    s.beg = &beg;
    s.end = &end;
    BoundBox2D r = s.getBox();
    EXPECT_EQ(r.min_x, 1);
    EXPECT_EQ(r.max_x, 3);
    EXPECT_EQ(r.min_y, 2);
    EXPECT_EQ(r.max_y, 4);
}

TEST(RectangleTest, UnionOperator) {
    BoundBox2D r1{1, 3, 2, 4};
    BoundBox2D r2{2, 4, 1, 3};
    BoundBox2D r3 = r1 | r2;
    EXPECT_EQ(r3.min_x, 1);
    EXPECT_EQ(r3.max_x, 4);
    EXPECT_EQ(r3.min_y, 1);
    EXPECT_EQ(r3.max_y, 4);
}

TEST(IDTest, IDTest) {
    ID id1(1);
    ID id5(5);
    ID id10(10);
    ID id10again(10);

    EXPECT_TRUE(id1 < id5);
    EXPECT_TRUE(id5 > id1);
    EXPECT_TRUE(id10 == id10again);
    EXPECT_TRUE(id10 <= id10again);
    EXPECT_TRUE(id10 >= id10again);
    EXPECT_TRUE(id1 <= id10);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}