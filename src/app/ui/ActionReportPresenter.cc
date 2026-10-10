#include "ActionReportPresenter.h"

#include <QCoreApplication>

#include "../../core/Document.h"
#include "Lib/Core/ProjectManager.h"

namespace {

QString failureReason(const ActionReport& report) {
    if (report.operationError) {
        using core::sketch::ErrorCode;
        switch (report.operationError->code) {
            case ErrorCode::NotFound:
                return QCoreApplication::translate("SketchActions", "The selected geometry no longer exists.");
            case ErrorCode::BackendFailure:
                return QCoreApplication::translate("SketchActions", "The solver backend reported an error.");
            case ErrorCode::SolveFailure:
                return QCoreApplication::translate("SketchActions", "The solver failed.");
            default:
                break;
        }
    }
    switch (report.rejection) {
        case ActionRejection::InvalidSelection:
            return QCoreApplication::translate("SketchActions", "Select compatible geometry for this constraint.");
        case ActionRejection::InvalidValue:
            return QCoreApplication::translate("SketchActions", "Enter a valid dimension value.");
        case ActionRejection::Unsupported:
            return QCoreApplication::translate("SketchActions", "This operation is not supported.");
        case ActionRejection::InsufficientTargets:
            return QCoreApplication::translate("SketchActions", "There are not enough eligible points for this constraint.");
        case ActionRejection::InvalidGeometry:
            return QCoreApplication::translate("SketchActions", "The geometry is invalid or degenerate.");
        case ActionRejection::None:
            break;
    }
    if (!report.operationError) {
        return {};
    }
    using core::sketch::ErrorCode;
    switch (report.operationError->code) {
        case ErrorCode::InvalidArgument:
            return QCoreApplication::translate("SketchActions", "The input geometry or parameters are invalid.");
        case ErrorCode::NotFound:
            return QCoreApplication::translate("SketchActions", "The selected geometry no longer exists.");
        case ErrorCode::Unsupported:
            return QCoreApplication::translate("SketchActions", "This operation is not supported by the selected solver.");
        case ErrorCode::BackendFailure:
            return QCoreApplication::translate("SketchActions", "The solver backend reported an error.");
        case ErrorCode::SolveFailure:
            return QCoreApplication::translate("SketchActions", "The solver failed.");
    }
    return {};
}

QString pasteSummary(const ActionReport& report) {
    if (report.entityIds.size() == 1 && report.constraintIds.size() == 1) {
        return QCoreApplication::translate("SketchActions", "Pasted 1 object and 1 constraint.");
    }
    if (report.entityIds.size() == 1) {
        return QCoreApplication::translate("SketchActions", "Pasted 1 object and %1 constraints.").arg(static_cast<qulonglong>(report.constraintIds.size()));
    }
    if (report.constraintIds.size() == 1) {
        return QCoreApplication::translate("SketchActions", "Pasted %1 objects and 1 constraint.").arg(static_cast<qulonglong>(report.entityIds.size()));
    }
    return QCoreApplication::translate("SketchActions", "Pasted %1 objects and %2 constraints.")
        .arg(static_cast<qulonglong>(report.entityIds.size()))
        .arg(static_cast<qulonglong>(report.constraintIds.size()));
}
}  // namespace

ActionReportPresenter::ActionReportPresenter(UI::ProjectManager& manager) : manager_(manager) {}

void ActionReportPresenter::present(const Document& document, const ActionReport& report) const {
    const QString message = text(report);
    if (!message.isEmpty()) {
        const QString name = QString::fromStdString(document.name());
        // The existing API falls back to a shared window for non-detached tabs.
        // Keep the origin visible even when another tab is active there.
        manager_.addNotification(name, QCoreApplication::translate("SketchActions", "%1: %2").arg(name, message));
    }
}

QString ActionReportPresenter::text(const ActionReport& report) {
    const bool failed = report.operationError.has_value() || report.rejection != ActionRejection::None;
    const bool solveFailed = report.solve && report.solve->status != core::sketch::SolveStatus::Converged;
    if (report.change == ModelChange::Unchanged && !failed && !solveFailed && !report.solveError) {
        return {};
    }

    QString message;
    switch (report.action) {
        case ActionKind::CreatePoint:
            message = !report.entityIds.empty() ? QCoreApplication::translate("SketchActions", "Point created.")
                      : report.change == ModelChange::PotentiallyChanged
                          ? QCoreApplication::translate("SketchActions", "Point creation could not be confirmed.")
                          : QCoreApplication::translate("SketchActions", "Point creation failed.");
            break;
        case ActionKind::CreateLine:
            message = !report.entityIds.empty()                          ? QCoreApplication::translate("SketchActions", "Line created.")
                      : report.change == ModelChange::PotentiallyChanged ? QCoreApplication::translate("SketchActions", "Line creation could not be confirmed.")
                                                                         : QCoreApplication::translate("SketchActions", "Line creation failed.");
            break;
        case ActionKind::CreateCircle:
            message = !report.entityIds.empty() ? QCoreApplication::translate("SketchActions", "Circle created.")
                      : report.change == ModelChange::PotentiallyChanged
                          ? QCoreApplication::translate("SketchActions", "Circle creation could not be confirmed.")
                          : QCoreApplication::translate("SketchActions", "Circle creation failed.");
            break;
        case ActionKind::ApplyConstraint:
            if (!report.constraintIds.empty()) {
                if (solveFailed) {
                    message = QCoreApplication::translate("SketchActions", "Constraint added, but the solver did not converge.");
                } else if (report.solveError) {
                    message = QCoreApplication::translate("SketchActions", "Constraint added, but solving failed.");
                } else {
                    message = QCoreApplication::translate("SketchActions", "Constraint added.");
                }
            } else {
                message = report.change == ModelChange::PotentiallyChanged
                              ? QCoreApplication::translate("SketchActions", "Constraint insertion could not be confirmed.")
                              : QCoreApplication::translate("SketchActions", "Constraint could not be added.");
            }
            break;
        case ActionKind::Delete:
            if (failed) {
                message = QCoreApplication::translate("SketchActions", "Deletion failed.");
            } else {
                message = report.entityIds.size() == 1
                              ? QCoreApplication::translate("SketchActions", "Deleted 1 object.")
                              : QCoreApplication::translate("SketchActions", "Deleted %1 objects.").arg(static_cast<qulonglong>(report.entityIds.size()));
            }
            break;
        case ActionKind::Paste:
            if (report.entityIds.empty()) {
                message = QCoreApplication::translate("SketchActions", "Paste failed.");
            } else {
                message = pasteSummary(report);
                if (failed) {
                    message += QLatin1Char(' ') + QCoreApplication::translate("SketchActions", "Paste stopped before all items were added.");
                }
            }
            break;
        case ActionKind::Drag:
            message = failed || solveFailed || report.solveError ? QCoreApplication::translate("SketchActions", "Drag stopped.")
                                                                 : QCoreApplication::translate("SketchActions", "Geometry moved.");
            break;
        case ActionKind::SwitchBackend:
            if (failed) {
                message = QCoreApplication::translate("SketchActions", "Solver could not be switched.");
            } else {
                const QString backend = report.backend == core::sketch::BackendKind::Dcm ? QStringLiteral("DCM") : QStringLiteral("SolveSpace");
                message = QCoreApplication::translate("SketchActions", "Solver switched to %1.").arg(backend);
            }
            break;
    }

    const QString reason = failureReason(report);
    if (!reason.isEmpty()) {
        message += QLatin1Char(' ') + reason;
    }
    if (report.change == ModelChange::PotentiallyChanged) {
        message += QLatin1Char(' ') + QCoreApplication::translate("SketchActions", "The sketch may have changed.");
    } else if (report.change == ModelChange::Changed && report.action == ActionKind::Drag && (failed || solveFailed || report.solveError)) {
        message += QLatin1Char(' ') + QCoreApplication::translate("SketchActions", "Geometry changes were retained.");
    }
    if (report.action != ActionKind::ApplyConstraint) {
        if (solveFailed) {
            message += QLatin1Char(' ') + QCoreApplication::translate("SketchActions", "The solver did not converge.");
        } else if (report.solveError) {
            message += QLatin1Char(' ') + QCoreApplication::translate("SketchActions", "Solving failed.");
        }
    }
    return message;
}
