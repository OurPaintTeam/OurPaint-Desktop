#include <gtest/gtest.h>
#include "PointBase.h"

using namespace OurPaintDCM::Figures;

TEST(PointBaseTest, DefaultConstructorInitializesToZero) {
    PointBase<2> p;
    EXPECT_DOUBLE_EQ(p[0], 0.0);
    EXPECT_DOUBLE_EQ(p[1], 0.0);
}

TEST(PointBaseTest, ConstructorWithValuesInitializesCorrectly) {
    PointBase<3> p({1.1, 2.2, 3.3});
    EXPECT_DOUBLE_EQ(p[0], 1.1);
    EXPECT_DOUBLE_EQ(p[1], 2.2);
    EXPECT_DOUBLE_EQ(p[2], 3.3);
}

TEST(PointBaseTest, OperatorBracketsAllowsModification) {
    PointBase<2> p;
    p[0] = 4.5;
    p[1] = -1.2;
    EXPECT_DOUBLE_EQ(p[0], 4.5);
    EXPECT_DOUBLE_EQ(p[1], -1.2);
}

TEST(PointBaseTest, DimensionReturnsCorrectValue) {
    EXPECT_EQ(PointBase<2>::dimension(), 2);
    EXPECT_EQ(PointBase<3>::dimension(), 3);
}
