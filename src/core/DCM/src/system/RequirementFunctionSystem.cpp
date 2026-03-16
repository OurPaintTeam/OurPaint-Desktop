#include "system/RequirementFunctionSystem.h"
#include <algorithm>
#include <Eigen/SVD>

using namespace OurPaintDCM::System;
using namespace OurPaintDCM::Function;

RequirementFunctionSystem::RequirementFunctionSystem() = default;

void RequirementFunctionSystem::addFunction(std::shared_ptr<RequirementFunction> func) {
    _functions.push_back(func);
    for (auto v : func->getVars()) {
        if (_allVarsSet.insert(v).second) {
            _allVars.push_back(v);
        }
    }
    size_t n = _allVars.size();
    size_t m = _functions.size();
    std::unordered_map<VAR, double> vars = func->gradient();
    _jacobian.resize(m, n);
    for (size_t i = 0; i < n; ++i) {
        if (vars.contains(_allVars[i])) {
            _jacobian.insert(m - 1, i) = vars[_allVars[i]];
            break;
        }
        _jacobian.insert(m - 1, i) = 0;
    }
}

void RequirementFunctionSystem::updateJ() {
    const size_t m = _functions.size();
    const size_t n = _allVars.size();
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(m * 6);

    for (size_t i = 0; i < m; ++i) {
        auto grad = _functions[i]->gradient();
        for (size_t j = 0; j < n; ++j) {
            VAR v = _allVars[j];
            double val = grad.contains(v) ? grad[v] : 0.0;
            if (val != 0.0)
                triplets.emplace_back(i, j, val);
        }
    }

    _jacobian.resize(m, n);
    _jacobian.setFromTriplets(triplets.begin(), triplets.end());
}


Eigen::SparseMatrix<double> RequirementFunctionSystem::J() const {
    return _jacobian;
}

Eigen::SparseMatrix<double> RequirementFunctionSystem::JTJ() const {
    Eigen::SparseMatrix<double> JT = _jacobian.transpose();
    return JT * _jacobian;
}

Eigen::VectorXd RequirementFunctionSystem::residuals() const {
    Eigen::VectorXd r(_functions.size());
    for (size_t i = 0; i < _functions.size(); ++i)
        r[i] = _functions[i]->evaluate() * _functions[i]->getWeight();
    return r;
}

std::vector<VAR> RequirementFunctionSystem::getAllVars() const {
    return _allVars;
}

OurPaintDCM::Utils::SystemStatus RequirementFunctionSystem::diagnose() const {
    if (_jacobian.rows() == 0 || _jacobian.cols() == 0)
        return Utils::SystemStatus::EMPTY;

    Eigen::MatrixXd denseJ = Eigen::MatrixXd(_jacobian);
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(denseJ);
    int rank = (svd.singularValues().array() > 1e-8).count();
    int m = denseJ.rows();
    int n = denseJ.cols();

    if (m == n && rank == n)
        return Utils::SystemStatus::WELL_CONSTRAINED;
    if (rank < std::min(m, n))
        return Utils::SystemStatus::SINGULAR_SYSTEM;
    if (m < n)
        return Utils::SystemStatus::UNDER_CONSTRAINED;
    if (m > n)
        return Utils::SystemStatus::OVER_CONSTRAINED;
    return Utils::SystemStatus::UNKNOWN;
}

void RequirementFunctionSystem::clear() {
    _functions.clear();
    _allVars.clear();
    _allVarsSet.clear();
    _jacobian.resize(0, 0);
}
