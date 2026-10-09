#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "../editor/ActionReport.h"
#include "../ui/ActionReportPresenter.h"

namespace {
using namespace core::sketch;

TEST(ActionReports, CreationRetainsConfirmedIdentityWithoutImplyingASolve) {
    const auto report = ActionReport::creation(ActionKind::CreatePoint, Result<EntityId>::success(EntityId(17)));
    EXPECT_EQ(report.change, ModelChange::Changed);
    EXPECT_EQ(report.entityIds, (std::vector<EntityId>{EntityId(17)}));
    EXPECT_FALSE(report.operationError);
    EXPECT_FALSE(report.solve);
    EXPECT_FALSE(report.solveError);
    EXPECT_EQ(ActionReportPresenter::text(report), QStringLiteral("Point created."));
}

TEST(ActionReports, PreflightRejectionOwnsItsErrorAndLeavesTheModelUnchanged) {
    ActionReport report{ActionKind::CreateLine};
    {
        std::string reason = "Coincident line endpoints";
        const auto result = Result<EntityId>::failure(ErrorCode::InvalidArgument, reason);
        report = ActionReport::creation(ActionKind::CreateLine, result);
        reason.clear();
    }
    EXPECT_EQ(report.change, ModelChange::Unchanged);
    EXPECT_TRUE(report.entityIds.empty());
    ASSERT_TRUE(report.operationError);
    EXPECT_EQ(report.operationError->code, ErrorCode::InvalidArgument);
    EXPECT_EQ(report.operationError->message, "Coincident line endpoints");
    const auto message = ActionReportPresenter::text(report);
    EXPECT_TRUE(message.contains(QStringLiteral("Line creation failed.")));
    EXPECT_FALSE(message.contains(QStringLiteral("may have changed")));
}

TEST(ActionReports, BackendMutationFailureDoesNotClaimConfirmedCreation) {
    const auto report = ActionReport::creation(ActionKind::CreateCircle, Result<EntityId>::failure(ErrorCode::BackendFailure, "Insertion failed"));
    EXPECT_EQ(report.change, ModelChange::PotentiallyChanged);
    EXPECT_TRUE(report.entityIds.empty());
    const auto message = ActionReportPresenter::text(report);
    EXPECT_TRUE(message.contains(QStringLiteral("Circle creation could not be confirmed.")));
    EXPECT_TRUE(message.contains(QStringLiteral("The sketch may have changed.")));
    EXPECT_FALSE(message.contains(QStringLiteral("Circle created.")));
}

TEST(ActionReports, FailedConvergenceRetainsInsertionAndOwningDiagnostics) {
    ActionReport report{ActionKind::ApplyConstraint};
    report.change = ModelChange::Changed;
    report.constraintIds.push_back(ConstraintId(12));
    {
        SolveDiagnostics diagnostics;
        diagnostics.status = SolveStatus::Failed;
        diagnostics.conflictingConstraints = std::vector<ConstraintId>{ConstraintId(5), ConstraintId(12)};
        report.recordSolve(Result<SolveDiagnostics>::success(diagnostics));
        diagnostics.conflictingConstraints->clear();
    }
    EXPECT_EQ(report.change, ModelChange::Changed);
    EXPECT_EQ(report.constraintIds, (std::vector<ConstraintId>{ConstraintId(12)}));
    ASSERT_TRUE(report.solve);
    EXPECT_EQ(report.solve->status, SolveStatus::Failed);
    ASSERT_TRUE(report.solve->conflictingConstraints);
    EXPECT_EQ(*report.solve->conflictingConstraints, (std::vector<ConstraintId>{ConstraintId(5), ConstraintId(12)}));
    EXPECT_FALSE(report.solve->degreesOfFreedom);
    EXPECT_FALSE(report.solve->redundantConstraints);
    EXPECT_EQ(ActionReportPresenter::text(report), QStringLiteral("Constraint added, but the solver did not converge."));
}

TEST(ActionReports, ConvergenceDoesNotImplyFullyConstrainedOrAvailableDiagnostics) {
    ActionReport report{ActionKind::ApplyConstraint};
    report.change = ModelChange::Changed;
    report.constraintIds.push_back(ConstraintId(1));
    report.recordSolve(Result<SolveDiagnostics>::success(SolveDiagnostics{}));
    ASSERT_TRUE(report.solve);
    EXPECT_EQ(report.solve->status, SolveStatus::Converged);
    EXPECT_FALSE(report.solve->degreesOfFreedom);
    EXPECT_FALSE(report.solve->conflictingConstraints);
    EXPECT_FALSE(report.solve->redundantConstraints);
    EXPECT_EQ(ActionReportPresenter::text(report), QStringLiteral("Constraint added."));

    SolveDiagnostics underconstrained;
    underconstrained.degreesOfFreedom = 3;
    report.recordSolve(Result<SolveDiagnostics>::success(underconstrained));
    EXPECT_EQ(ActionReportPresenter::text(report), QStringLiteral("Constraint added."));
}

TEST(ActionReports, SolveOperationErrorRetainsConfirmedInsertion) {
    ActionReport report{ActionKind::ApplyConstraint};
    report.change = ModelChange::Changed;
    report.constraintIds.push_back(ConstraintId(9));
    report.recordSolve(Result<SolveDiagnostics>::failure(ErrorCode::BackendFailure, "Numerical solve failed"));
    EXPECT_EQ(report.change, ModelChange::Changed);
    EXPECT_EQ(report.constraintIds, (std::vector<ConstraintId>{ConstraintId(9)}));
    EXPECT_FALSE(report.operationError);
    EXPECT_FALSE(report.solve);
    ASSERT_TRUE(report.solveError);
    EXPECT_EQ(report.solveError->code, ErrorCode::BackendFailure);
    EXPECT_EQ(report.solveError->message, "Numerical solve failed");
    EXPECT_EQ(ActionReportPresenter::text(report), QStringLiteral("Constraint added, but solving failed."));
}

TEST(ActionReports, ConfirmedPartialPasteReportsCountsAndPreservesRejection) {
    ActionReport report{ActionKind::Paste};
    report.change = ModelChange::Changed;
    report.requestedEntities = 2;
    report.requestedConstraints = 2;
    report.entityIds = {EntityId(3), EntityId(4)};
    report.constraintIds = {ConstraintId(6)};
    report.mutationFailed({ErrorCode::Unsupported, "Copied constraint is unsupported"});
    EXPECT_EQ(report.change, ModelChange::Changed);
    const auto message = ActionReportPresenter::text(report);
    EXPECT_TRUE(message.contains(QStringLiteral("Pasted 2 objects and 1 constraint.")));
    EXPECT_TRUE(message.contains(QStringLiteral("Paste stopped before all items were added.")));
    EXPECT_TRUE(message.contains(QStringLiteral("not supported")));
    EXPECT_FALSE(message.contains(QStringLiteral("may have changed")));
}

TEST(ActionReports, UnconfirmedPartialPasteDoesNotInventInsertedIds) {
    ActionReport report{ActionKind::Paste};
    report.requestedEntities = 3;
    report.mutationFailed({ErrorCode::BackendFailure, "Entity batch stopped"});
    EXPECT_EQ(report.change, ModelChange::PotentiallyChanged);
    EXPECT_TRUE(report.entityIds.empty());
    const auto message = ActionReportPresenter::text(report);
    EXPECT_TRUE(message.contains(QStringLiteral("Paste failed.")));
    EXPECT_TRUE(message.contains(QStringLiteral("The sketch may have changed.")));
    EXPECT_FALSE(message.contains(QStringLiteral("Pasted")));
}

TEST(ActionReports, BatchDeletionProducesOneSummaryAndEmptyActionsStaySilent) {
    ActionReport report{ActionKind::Delete};
    EXPECT_TRUE(ActionReportPresenter::text(report).isEmpty());
    report.change = ModelChange::Changed;
    report.requestedEntities = 3;
    report.entityIds = {EntityId(1), EntityId(4), EntityId(7)};
    EXPECT_EQ(ActionReportPresenter::text(report), QStringLiteral("Deleted 3 objects."));

    EXPECT_TRUE(ActionReportPresenter::text(ActionReport{ActionKind::Paste}).isEmpty());
    EXPECT_TRUE(ActionReportPresenter::text(ActionReport{ActionKind::Drag}).isEmpty());
}

TEST(ActionReports, StagedBackendFailureKeepsItsUnchangedGuarantee) {
    ActionReport report{ActionKind::SwitchBackend};
    report.backend = BackendKind::SolveSpace;
    report.operationError = SketchError{ErrorCode::BackendFailure, "Candidate backend could not load state"};
    EXPECT_EQ(report.change, ModelChange::Unchanged);
    const auto message = ActionReportPresenter::text(report);
    EXPECT_TRUE(message.contains(QStringLiteral("Solver could not be switched.")));
    EXPECT_FALSE(message.contains(QStringLiteral("may have changed")));
    EXPECT_FALSE(message.contains(QStringLiteral("Solver switched to")));
}

TEST(ActionReports, PreparationErrorsDescribeMissingGeometryAndBackendFailure) {
    ActionReport report{ActionKind::ApplyConstraint};
    report.rejection = ActionRejection::InvalidSelection;
    report.operationError = SketchError{ErrorCode::NotFound, "The selected entity was removed"};
    const auto missing = ActionReportPresenter::text(report);
    EXPECT_TRUE(missing.contains(QStringLiteral("The selected geometry no longer exists.")));
    EXPECT_FALSE(missing.contains(QStringLiteral("Select compatible geometry")));

    report.operationError = SketchError{ErrorCode::BackendFailure, "Entity query failed"};
    const auto failedQuery = ActionReportPresenter::text(report);
    EXPECT_TRUE(failedQuery.contains(QStringLiteral("The solver backend reported an error.")));
    EXPECT_FALSE(failedQuery.contains(QStringLiteral("Select compatible geometry")));
    EXPECT_FALSE(failedQuery.contains(QStringLiteral("may have changed")));
}

TEST(ActionReports, FailedDragReportsRetainedGeometryAndFailedConvergence) {
    ActionReport report{ActionKind::Drag};
    report.change = ModelChange::Changed;
    SolveDiagnostics diagnostics;
    diagnostics.status = SolveStatus::Failed;
    report.recordSolve(Result<SolveDiagnostics>::success(diagnostics));
    const auto message = ActionReportPresenter::text(report);
    EXPECT_TRUE(message.contains(QStringLiteral("The solver did not converge.")));
    EXPECT_TRUE(message.contains(QStringLiteral("Geometry changes were retained.")));
    EXPECT_FALSE(message.contains(QStringLiteral("fully constrained")));
}
}  // namespace
