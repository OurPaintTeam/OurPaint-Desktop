#include <gtest/gtest.h>
#include "Point2D.h"
#include "Line.h"

using namespace OurPaintDCM::Figures;

TEST(Line2DTest, ConstructorAndLength) {
    Point2D p1(0,0);
    Point2D p2(3,4);

    Line line(&p1, &p2);

    EXPECT_DOUBLE_EQ(line.length(), 5.0);

    EXPECT_DOUBLE_EQ(line.p1->x(), 0.0);
    EXPECT_DOUBLE_EQ(line.p1->y(), 0.0);
    EXPECT_DOUBLE_EQ(line.p2->x(), 3.0);
    EXPECT_DOUBLE_EQ(line.p2->y(), 4.0);

    p2.x() = 0;
    p2.y() = 0;
    EXPECT_DOUBLE_EQ(line.length(), 0.0);
}
