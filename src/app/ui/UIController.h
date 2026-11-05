#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "../../core/DocumentManager.h"
#include "../editor/SketchEditor.h"
#include "../editor/tools/ToolId.h"
#include "../platform/IPlatformRuntime.h"
#include "../platform/IViewportHost.h"
#include "../platform/QtViewportHost.h"
#include "../project/DocumentView.h"
#include "../project/ProjectSaveLoad.h"
#include "Lib/Core/ProjectManager.h"

class UIController {
public:
    UIController(std::vector<std::unique_ptr<DocumentView>>& views,
                 DocumentManager& manager,
                 IPlatformRuntime& platformRuntime,
                 UI::ProjectManager& projectManager,
                 QtViewportHost& host);
    ~UIController();

    void selectTool(ToolId tool, std::optional<double> value, const std::string& tabName = {});

    // Console
    void executeConsoleCommand(std::string str);

    // Project / Window
    void openProjectInNewWindow(const QString& path = {});
    void createProjectInNewWindow(const QString& path = {});

    bool openProjectInCurrentWindow(const QString& path = {});
    bool createProjectInCurrentWindow(const QString& path = {});
    bool saveProject();
    bool saveProjectAs();
    bool confirmClose();

    void openFile(const QString& path = {});

    void createFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);
    void removeFile(const std::string& fileName);
    void renameTab(const std::string& oldName, const std::string& newName);

    void renameProject();
    void deleteProject();

    void closeApplication();

private:
    DocumentView* findView(const std::string& name) const;
    void attachDocument(DocumentId id, const std::string& tabName);
    void installProject(app::LoadedProject loaded);
    bool confirmProjectChange();
    void endInteractions();
    void updateProjectData();
    void presentActiveTool(DocumentId id, ToolId tool);
    //void presentReport(DocumentId id, const ActionReport& report);
    std::string uniqueTabName(const std::string& preferred) const;
    std::filesystem::path nextFilename(DocumentId id) const;

    std::vector<std::unique_ptr<DocumentView>>& views_;
    DocumentManager& manager_;
    IPlatformRuntime& platformRuntime_;
    QtViewportHost& viewportHost_;
    UI::ProjectManager& projectManager_;
    app::Project project_{{}, {{}, "Untitled"}, {}};
    bool projectModified_ = true;
    // Document IDs and viewport indices have different lifetimes and domains.
    std::unordered_map<std::string, DocumentId> documentIds_;
    std::unordered_map<std::string, int> indicesMap_;
    std::unordered_map<ViewportController*, int> controllerMap_;
    std::string activeTabName_;
};

#endif // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_
