#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <string>

#include "DocumentManager.h"
#include "InteractionTools/EditorSession.h"
#include "InteractionTools/ToolId.h"
#include "IViewportHost.h"

class UIController {
public:
    UIController(EditorSession& editorSession, DocumentManager& manager, IViewportHost& host);
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
    void createFile();

    void renameProject();
    void deleteProject();

    void closeApplication();

private:
    EditorSession& editorSession_;
    DocumentManager& docManager_;
    IViewportHost& host_;
};

#endif // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_
