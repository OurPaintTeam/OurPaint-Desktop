#ifndef OURPAINT_APPLICATION_ACTION_REPORT_PRESENTER_H_
#define OURPAINT_APPLICATION_ACTION_REPORT_PRESENTER_H_

#include <QString>
#include <string>

#include "../editor/ActionReport.h"

class Document;
namespace UI {
class ProjectManager;
}
namespace app {
struct FileError;
struct SaveReport;
}

class ActionReportPresenter {
public:
    explicit ActionReportPresenter(UI::ProjectManager& manager);

    void present(const Document& document, const ActionReport& report) const;
    void present(const std::string& tabName, const ActionReport& report) const;
    void present(const app::SaveReport& report) const;
    void present(const app::FileError& error) const;
    static QString text(const ActionReport& report);

private:
    UI::ProjectManager& manager_;
};

#endif
