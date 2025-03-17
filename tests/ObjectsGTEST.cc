#include <gtest/gtest.h>
#include "Objects.h"

TEST(PointTest, RectMethod) {
    Point p;
    p.x = 3;
    p.y = 4;
    rectangle r = p.rect();
    EXPECT_EQ(r.x_1, 3);
    EXPECT_EQ(r.x_2, 3);
    EXPECT_EQ(r.y_1, 4);
    EXPECT_EQ(r.y_2, 4);
}

TEST(CircleTest, RectMethod) {
    Point center;
    center.x = 0;
    center.y = 0;
    Circle c;
    c.center = &center;
    c.R = 5;
    rectangle r = c.rect();
    EXPECT_EQ(r.x_1, -5);
    EXPECT_EQ(r.x_2, 5);
    EXPECT_EQ(r.y_1, -5);
    EXPECT_EQ(r.y_2, 5);
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
    rectangle r = s.rect();
    EXPECT_EQ(r.x_1, 1);
    EXPECT_EQ(r.x_2, 3);
    EXPECT_EQ(r.y_1, 2);
    EXPECT_EQ(r.y_2, 4);
}

TEST(RectangleTest, UnionOperator) {
    rectangle r1{1, 3, 2, 4};
    rectangle r2{2, 4, 1, 3};
    rectangle r3 = r1 || r2;
    EXPECT_EQ(r3.x_1, 1);
    EXPECT_EQ(r3.x_2, 4);
    EXPECT_EQ(r3.y_1, 1);
    EXPECT_EQ(r3.y_2, 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}