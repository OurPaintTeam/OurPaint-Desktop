#include <gtest/gtest.h>
#include "Point2D.h"
#include "math.h"

using namespace OurPaintDCM::Figures;

class Point2DTest : public ::testing::Test {
protected:
    Point2D p;

    void SetUp() override {
        p.x() = 0.0;
        p.y() = 0.0;
    }
};

TEST_F(Point2DTest, DefaultCoordinates) {
    EXPECT_DOUBLE_EQ(p.x(), 0.0);
    EXPECT_DOUBLE_EQ(p.y(), 0.0);
}

TEST_F(Point2DTest, SetCoordinates) {
    p.x() = 5.5;
    p.y() = -3.2;
    EXPECT_DOUBLE_EQ(p.x(), 5.5);
    EXPECT_DOUBLE_EQ(p.y(), -3.2);
}
TEST_F(Point2DTest, DistanceToAnotherPoint) {
    Point2D p2;
    p2.x() = 3.0;
    p2.y() = 4.0;

    double dx = p2.x() - p.x();
    double dy = p2.y() - p.y();
    double distance = std::sqrt(dx * dx + dy * dy);

    EXPECT_DOUBLE_EQ(distance, 5.0);
}