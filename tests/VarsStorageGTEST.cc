#include <gtest/gtest.h>
#include "Requirements.h"
#include <functional>

class VarsStorageTest : public ::testing::Test {
protected:
    void SetUp() override {
        VarsStorage::clearVars(); // Очистка перед каждым тестом
    }

    void TearDown() override {
        VarsStorage::clearVars(); // Очистка после каждого теста
    }
};

TEST_F(VarsStorageTest, AddVarTest) {
    double id_val = 1.0;
    PARAMID id = &id_val;
    Variable* var = VarsStorage::addVar(id);
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->evaluate(), id_val);
}

TEST_F(VarsStorageTest, GetVarTest) {
    double id_val = 2.0;
    PARAMID id = &id_val;
    VarsStorage::addVar(id);
    Variable* var = VarsStorage::getVar(id);
    ASSERT_NE(var, nullptr);
    EXPECT_EQ(var->evaluate(), id_val);
}

TEST_F(VarsStorageTest, GetVarThrowsException) {
    double id_val = 3.0;
    PARAMID id = &id_val;
    EXPECT_THROW(VarsStorage::getVar(id), std::runtime_error);
}

TEST_F(VarsStorageTest, GetVarsTest) {
    double id_val1 = 4.0, id_val2 = 5.0;
    VarsStorage::addVar(&id_val1);
    VarsStorage::addVar(&id_val2);
    auto vars = VarsStorage::getVars();
    EXPECT_EQ(vars.size(), 2);
}

TEST_F(VarsStorageTest, ClearVarsTest) {
    double id_val = 6.0;
    PARAMID id = &id_val;
    VarsStorage::addVar(id);
    VarsStorage::clearVars();
    EXPECT_THROW(VarsStorage::getVar(id), std::runtime_error);
    EXPECT_TRUE(VarsStorage::getVars().empty());
}
