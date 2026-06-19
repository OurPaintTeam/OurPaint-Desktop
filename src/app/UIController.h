#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <string>

#include "DocumentManager.h"
#include "IPlatformRuntime.h"
#include "IViewportHost.h"
#include "InteractionTools/EditorSession.h"
#include "InteractionTools/ToolId.h"
#include "Lib/Core/ProjectManager.h"
#include "QtViewportHost.h"
#include "Tab.h"

class UIController {
public:
    UIController(DocumentManager& manager,
                 IPlatformRuntime& platformRuntime,
                 UI::ProjectManager& projectManager,
                 std::vector<Tab>& tabs,
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
    DocumentManager& documentManager_;
    IPlatformRuntime& platformRuntime_;
    QtViewportHost& viewportHost_;
    UI::ProjectManager& projectManager_;
    std::vector<Tab>& tabs_;

    std::unordered_map<std::string, int> indicesMap_;

    std::unordered_map<ViewportController*, int> controllerMap_;

    std::string activeTabName_;
};

#endif // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_
