#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <string>

#include "DocumentManager.h"
#include "InteractionTools/EditorSession.h"
#include "InteractionTools/ToolId.h"
#include "IViewportHost.h"

#include "Lib/Core/ProjectManager.h"
#include "Tab.h"
#include "IPlatformRuntime.h"

class UIController {
public:
    UIController(DocumentManager& manager,
                 IPlatformRuntime& platformRuntime,
                 UI::ProjectManager& projectManager,
                 std::vector<Tab>& tabs);
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
    void renameTab();
    void removeTab();

    void createFile(const std::string& fileName);
    void setActiveFile(const std::string& fileName);

    void renameProject();
    void deleteProject();

    void closeApplication();

private:
    DocumentManager& documentManager_;
    IPlatformRuntime& platformRuntime_;
    UI::ProjectManager& projectManager_;
    std::vector<Tab>& tabs_;

    std::unordered_map<std::string, int> indicesMap_;

    std::string activeTabName_;
};

#endif // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_
