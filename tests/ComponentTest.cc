#include <gtest/gtest.h>
#include "Component.h"
#include "LSMFORLMTask.h"
#include "GeometricObjects.h"

TEST(ComponentTest, Constructor) {
    Component component;
    EXPECT_EQ(component._task, nullptr);
}

TEST(ComponentTest, MoveConstructor_and_MoveOperator) {
    Point p1(1, 2);
    Point p2(3, 4);

    double* x1 = &p1.x;
    double* y1 = &p1.y;
    double* x2 = &p2.x;
    double* y2 = &p2.y;

    std::vector<Variable*> vars = {
            new Variable(x1),
            new Variable(y1),
            new Variable(x2),
            new Variable(y2)
    };

    std::vector<Function*> errorFunctions;
    Function* f = new PointOnPointError(vars);
    errorFunctions.push_back(f);

    LSMFORLMTask* task = new LSMFORLMTask(errorFunctions, vars);

    std::unordered_set<ID> objectIDs = { ID(1), ID(2) };

    std::vector<Variable*> varsCopy = {
            new Variable(x1),
            new Variable(y1),
            new Variable(x2),
            new Variable(y2)
    };

    Component component;
    component._errorFunctions = errorFunctions;
    component._vars = varsCopy;
    component._task = task;
    component._objectIDs = objectIDs;
    Component c2(std::move(component));

    EXPECT_EQ(component._task, nullptr);
    EXPECT_EQ(component._objectIDs.size(), 0);
    EXPECT_EQ(component._vars.size(), 0);
    EXPECT_EQ(component._errorFunctions.size(), 0);

    EXPECT_EQ(c2._task, task);
    EXPECT_EQ(c2._vars, varsCopy);
    EXPECT_EQ(c2._errorFunctions, errorFunctions);
    EXPECT_EQ(c2._objectIDs, objectIDs);

    Component c3;
    c3 = std::move(c2);

    EXPECT_EQ(c2._task, nullptr);
    EXPECT_EQ(c2._objectIDs.size(), 0);
    EXPECT_EQ(c2._vars.size(), 0);
    EXPECT_EQ(c2._errorFunctions.size(), 0);

    EXPECT_EQ(c3._task, task);
    EXPECT_EQ(c3._vars, varsCopy);
    EXPECT_EQ(c3._errorFunctions, errorFunctions);
    EXPECT_EQ(c3._objectIDs, objectIDs);
}
