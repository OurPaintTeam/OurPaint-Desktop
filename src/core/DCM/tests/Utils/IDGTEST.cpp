#include <gtest/gtest.h>
#include "ID.h"

using namespace OurPaintDCM::Utils;

TEST(IDTest, DefaultConstructor) {
    ID id;
    EXPECT_EQ(id.id, 0);
}

TEST(IDTest, ExplicitConstructor) {
    ID id(42);
    EXPECT_EQ(id.id, 42);
}

TEST(IDTest, EqualityOperator) {
    ID id1(10);
    ID id2(10);
    ID id3(20);
    EXPECT_TRUE(id1 == id2);
    EXPECT_FALSE(id1 == id3);
}

TEST(IDTest, InequalityOperator) {
    ID id1(10);
    ID id2(20);
    EXPECT_TRUE(id1 != id2);
    EXPECT_FALSE(id1 != ID(10));
}

TEST(IDTest, LessThanOperator) {
    ID id1(5);
    ID id2(10);
    EXPECT_TRUE(id1 < id2);
    EXPECT_FALSE(id2 < id1);
    EXPECT_FALSE(ID(7) < ID(7));
}

TEST(IDTest, GreaterThanOperator) {
    ID id1(15);
    ID id2(10);
    EXPECT_TRUE(id1 > id2);
    EXPECT_FALSE(id2 > id1);
    EXPECT_FALSE(ID(7) > ID(7));
}

TEST(IDTest, PrefixIncrement) {
    ID id(5);
    ID& ref = ++id;
    EXPECT_EQ(id.id, 6);
    EXPECT_EQ(ref.id, 6);
}

TEST(IDTest, PostfixIncrement) {
    ID id(5);
    ID old = id++;
    EXPECT_EQ(old.id, 5);
    EXPECT_EQ(id.id, 6);
}
