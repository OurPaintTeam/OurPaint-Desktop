#ifndef OURPAINT_APPLICATION_UI_CONTROLLER_H_
#define OURPAINT_APPLICATION_UI_CONTROLLER_H_

#include <string>

#include "DocumentManager.h"
#include "Cpu2dPicker.h"
#include "InteractionTools/EditorSession.h"
#include "InteractionTools/ToolId.h"
#include "IViewportHost.h"
#include "OverlayModel.h"
#include "RenderDataBuilder.h"

class UIController {
public:
    UIController(EditorSession& editorSession,
                 DocumentManager& manager,
                 IViewportHost& host,
                 RenderDataBuilder& builder,
                 Cpu2dPicker& picker,
                 OverlayModel& overlay);
    ~UIController() = default;

    void selectTool(ToolId tool);

    // Console
    void executeConsoleCommand(std::string str);

    // Project / Window
    void openProjectInNewWindow();
    void createProjectInNewWindow();

    void openProjectInCurrentWindow();
    void createProjectInCurrentWindow();

    std::string openFile(const std::string& filePath);
    bool saveActiveDocument();
    std::string saveActiveDocumentAs(const std::string& filePath);
    bool setActiveDocument(const std::string& documentName);
    std::string activeDocumentName();
    void renameTab();
    void removeTab();
    std::string createFile(const std::string& documentName);

    void renameProject();
    void deleteProject();

    void closeApplication();

private:
    EditorSession& editorSession_;
    DocumentManager& docManager_;
    IViewportHost& host_;
    RenderDataBuilder& builder_;
    Cpu2dPicker& picker_;
    OverlayModel& overlay_;

    void syncActiveDocument();
};

#endif // ! OURPAINT_APPLICATION_UI_CONTROLLER_H_
