#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <string>

#include "../editor/EditorSession.h"
#include "../editor/tools/ToolId.h"
#include "../platform/IPlatformRuntime.h"
#include "../platform/IViewportHost.h"
#include "../platform/QtViewportHost.h"
#include "../project/DocumentView.h"
#include "DocumentManager.h"
#include "Lib/Core/ProjectManager.h"

class UIController {
public:
    UIController(std::vector<DocumentView*>& views,
                 DocumentManager& manager,
                 IPlatformRuntime& platformRuntime,
                 UI::ProjectManager& projectManager,
                 QtViewportHost& host);
    ~UIController() = default;

    void selectTool(ToolId tool, double value = 0.0);

    // Console
    void executeConsoleCommand(std::string str);

    // Project / Window
    void openProjectInNewWindow();
    void createProjectInNewWindow();

    void openProjectInCurrentWindow();
    void createProjectInCurrentWindow();

    void openFile();

    void createFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);
    void removeFile(const std::string& fileName);
    void renameTab(const std::string& oldName, const std::string& newName);

    void renameProject();
    void deleteProject();

    void closeApplication();

private:
    std::vector<DocumentView*>& views_;
    DocumentManager& manager_;
    IPlatformRuntime& platformRuntime_;
    QtViewportHost& viewportHost_;
    UI::ProjectManager& projectManager_;


    std::unordered_map<std::string, int> indicesMap_;
    std::unordered_map<ViewportController*, int> controllerMap_;
    std::unordered_set<std::string> set_;
    std::string activeTabName_;
    std::string activeIndex_;
};

#endif // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_
