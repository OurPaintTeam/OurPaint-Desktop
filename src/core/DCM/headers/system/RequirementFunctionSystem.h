#ifndef OURPAINTDCM_HEADERS_SYSTEM_REQUIREMENTFUNCTIONSYSTEM_H
#define OURPAINTDCM_HEADERS_SYSTEM_REQUIREMENTFUNCTIONSYSTEM_H
#include "RequirementFunction.h"
#include "Enums.h"
#include <vector>
#include <memory>
#include <unordered_set>
#include <Eigen/Dense>
#include <Eigen/Sparse>


namespace OurPaintDCM::System {
    /**
     * @brief System of geometric constraint functions.
     *
     * Collects multiple RequirementFunction objects and manages
     * the set of unique variables involved in them.
     * Provides interfaces for evaluating residuals, assembling
     * Jacobian matrices, and performing diagnostics of the system.
     */
    class RequirementFunctionSystem {
        std::vector<std::shared_ptr<Function::RequirementFunction>> _functions; ///< All constraint functions
        std::vector<VAR> _allVars;                                              ///< Unique variable pointers
        std::unordered_set<VAR> _allVarsSet;                                    ///< Fast lookup for uniqueness
        Eigen::SparseMatrix<double> _jacobian;                                  ///< Cached Jacobian

    public:
        /// @brief Default constructor
        RequirementFunctionSystem();

        /**
         * @brief Add a new geometric constraint function to the system.
         * @param function Shared pointer to a RequirementFunction.
         */
        void addFunction(std::shared_ptr<Function::RequirementFunction> function);

        /**
         * @brief Update the cached Jacobian matrix (sparse).
         *
         * Recomputes the full Jacobian from all active functions and their gradients.
         */
        void updateJ();

        /**
         * @brief Get the current sparse Jacobian matrix.
         * @return The most recently computed Jacobian.
         */
        Eigen::SparseMatrix<double> J() const;

        /**
         * @brief Compute the normal matrix JᵀJ used in LM and Dogleg solvers.
         * @return Sparse matrix JᵀJ.
         */
        Eigen::SparseMatrix<double> JTJ() const;

        /**
         * @brief Compute the full residual vector f(x) of all constraints.
         * @return Dense Eigen vector of residuals.
         */
        Eigen::VectorXd residuals() const;

        /**
         * @brief Get the list of all unique variable pointers in the system.
         * @return Vector of VAR (double*).
         */
        std::vector<VAR> getAllVars() const;

        /**
         * @brief Diagnose the system's constraint state based on Jacobian rank.
         * @return One of: "Well-constrained", "Under-constrained", or "Over-constrained".
         */
        Utils::SystemStatus diagnose() const;

        /// @brief Get all constraint functions.
        const std::vector<std::shared_ptr<Function::RequirementFunction>>& getFunctions() const { return _functions; }

        /**
         * @brief Clear all stored functions and variables.
         */
        void clear();
    };
}


#endif //OURPAINTDCM_HEADERS_SYSTEM_REQUIREMENTFUNCTIONSYSTEM_H
