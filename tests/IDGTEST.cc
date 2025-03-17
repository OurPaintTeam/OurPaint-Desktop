#include <gtest/gtest.h>
#include "ID.h"
#include <sstream>

TEST(IDTest, AssignmentOperator) {
    ID id1, id2;
    id1.id = 42;
    id2 = id1;
    EXPECT_EQ(id2.id, 42);
}

TEST(IDTest, GreaterThanOperator) {
    ID id1, id2;
    id1.id = 10;
    id2.id = 5;
    EXPECT_TRUE(id1 > id2);
    EXPECT_FALSE(id2 > id1);
}

TEST(IDTest, LessThanOperator) {
    ID id1, id2;
    id1.id = 3;
    id2.id = 8;
    EXPECT_TRUE(id1 < id2);
    EXPECT_FALSE(id2 < id1);
}

TEST(IDTest, EqualityOperator) {
    ID id1, id2;
    id1.id = 7;
    id2.id = 7;
    EXPECT_TRUE(id1 == id2);
    id2.id = 9;
    EXPECT_FALSE(id1 == id2);
}

TEST(IDTest, InputStreamOperator) {
    ID id;
    std::istringstream input("123");
    input >> id.id;
    EXPECT_EQ(id.id, 123);
}
