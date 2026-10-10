#ifndef OURPAINT_APPLICATION_ACTION_REPORT_PRESENTER_H_
#define OURPAINT_APPLICATION_ACTION_REPORT_PRESENTER_H_

#include <QString>

#include "../editor/ActionReport.h"

class Document;
namespace UI {
class ProjectManager;
}

class ActionReportPresenter {
public:
    explicit ActionReportPresenter(UI::ProjectManager& manager);

    void present(const Document& document, const ActionReport& report) const;
    static QString text(const ActionReport& report);

private:
    UI::ProjectManager& manager_;
};

#endif
