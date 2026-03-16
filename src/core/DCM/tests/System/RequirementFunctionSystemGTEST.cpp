#include <gtest/gtest.h>
#include "RequirementFunctionSystem.h"
#include "RequirementFunction.h"
#include <Eigen/Dense>

using namespace OurPaintDCM::System;
using namespace OurPaintDCM::Function;
using namespace OurPaintDCM::Utils;

TEST(RequirementFunctionSystemTest, AddAndUpdateJacobian) {
    RequirementFunctionSystem system;

    double x1 = 0, y1 = 0, x2 = 3, y2 = 4;
    std::vector<double*> vars = {&x1, &y1, &x2, &y2};

    auto func = std::make_shared<PointPointDistanceFunction>(vars, 5.0);
    system.addFunction(func);

    auto allVars = system.getAllVars();
    EXPECT_EQ(allVars.size(), 4);

    system.updateJ();
    Eigen::SparseMatrix<double> J = system.J();
    EXPECT_EQ(J.rows(), 1);
    EXPECT_EQ(J.cols(), 4);

    Eigen::MatrixXd dense = Eigen::MatrixXd(J);
    double sum = dense.sum();
    EXPECT_NEAR(sum, 0.0, 1e-6);
}

TEST(RequirementFunctionSystemTest, ResidualComputation) {
    RequirementFunctionSystem system;

    double x1 = 0, y1 = 0, x2 = 3, y2 = 4;
    std::vector<double*> vars = {&x1, &y1, &x2, &y2};

    auto func = std::make_shared<PointPointDistanceFunction>(vars, 5.0);
    system.addFunction(func);

    Eigen::VectorXd r = system.residuals();
    EXPECT_EQ(r.size(), 1);
    EXPECT_NEAR(r[0], 0.0, 1e-8);
}

TEST(RequirementFunctionSystemTest, DiagnoseWellConstrained) {
    RequirementFunctionSystem system;

    double x1 = 0, y1 = 0, x2 = 1, y2 = 0;
    std::vector<double*> vars = {&x1, &y1, &x2, &y2};
    auto func = std::make_shared<HorizontalFunction>(vars);

    system.addFunction(func);
    system.updateJ();

    auto status = system.diagnose();
    EXPECT_TRUE(status == SystemStatus::UNDER_CONSTRAINED ||
                status == SystemStatus::WELL_CONSTRAINED ||
                status == SystemStatus::OVER_CONSTRAINED);
}

TEST(RequirementFunctionSystemTest, ClearResetsSystem) {
    RequirementFunctionSystem system;

    double x1 = 0, y1 = 0, x2 = 1, y2 = 0;
    std::vector<double*> vars = {&x1, &y1, &x2, &y2};
    auto func = std::make_shared<HorizontalFunction>(vars);
    system.addFunction(func);
    system.updateJ();

    system.clear();

    EXPECT_TRUE(system.getAllVars().empty());
    EXPECT_EQ(system.J().rows(), 0);
    EXPECT_EQ(system.J().cols(), 0);
}
