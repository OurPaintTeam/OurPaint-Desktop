#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <string>

#include "DocumentManager.h"
#include "InteractionTools/EditorSession.h"
#include "InteractionTools/ToolId.h"

class UIController {
public:
    UIController(EditorSession& editorSession, DocumentManager& manager);
    ~UIController() = default;

    void selectTool(ToolId tool);

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
};

#endif // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_
