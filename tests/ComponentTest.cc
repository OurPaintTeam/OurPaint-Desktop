#include <gtest/gtest.h>
#include "Component.h"
#include "LSMFORLMTask.h"
#include "GeometricObjects.h"

TEST(ComponentTest, Constructor) {
    Component component;
    EXPECT_EQ(component.task(), nullptr);
}

TEST(ComponentTest, MoveConstructor_and_MoveOperator) {
    Component component;

    Point p1(1, 2);
    Point p2(3, 4);

    double x1 = p1.x;
    double y1 = p1.y;
    double x2 = p2.x;
    double y2 = p2.y;

    std::vector<Variable*> vars = {
            new Variable(&x1),
            new Variable(&y1),
            new Variable(&x2),
            new Variable(&y2)
    };

    std::vector<Function*> errorFunctions;
    Function* f = new PointOnPointError(vars);
    errorFunctions.push_back(f);

    LSMFORLMTask* task = new LSMFORLMTask(errorFunctions, vars);

    std::unordered_set<ID> objectIDs = { ID(1), ID(2) };

    component.errorFunctions() = errorFunctions;
    component.vars() = vars;
    component.task() = task;
    component.objectIDs() = objectIDs;

    Component c2(std::move(component));

    EXPECT_EQ(component.task(), nullptr);
    EXPECT_EQ(component.objectIDs().size(), 0);
    EXPECT_EQ(component.vars().size(), 0);
    EXPECT_EQ(component.errorFunctions().size(), 0);

    EXPECT_EQ(c2.task(), task);
    EXPECT_EQ(c2.vars(), vars);
    EXPECT_EQ(c2.errorFunctions(), errorFunctions);
    EXPECT_EQ(c2.objectIDs(), objectIDs);

    Component c3;
    c3 = std::move(c2);

    EXPECT_EQ(c2.task(), nullptr);
    EXPECT_EQ(c2.objectIDs().size(), 0);
    EXPECT_EQ(c2.vars().size(), 0);
    EXPECT_EQ(c2.errorFunctions().size(), 0);

    EXPECT_EQ(c3.task(), task);
    EXPECT_EQ(c3.vars(), vars);
    EXPECT_EQ(c3.errorFunctions(), errorFunctions);
    EXPECT_EQ(c3.objectIDs(), objectIDs);
}
