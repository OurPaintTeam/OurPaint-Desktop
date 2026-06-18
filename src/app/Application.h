#ifndef APPLICATION_H_
#define APPLICATION_H_

#define GL_GLEXT_PROTOTYPES

#include "App/CustomConsole.h"
#include "Camera2D.h"
#include "Cpu2dPicker.h"
#include "IRenderer.h"
#include "InteractionTools/EditorSession.h"
#include "InteractionTools/IInteractionTool.h"
#include "OverlayModel.h"
#include "QtMainWindowBinder.h"

class DocumentManager;

class IPlatformRuntime;
class IViewportHost;

class UIController;
class ViewportController;

class UIObserver;
class RenderSceneBuilder;

class CommandConsole;
class AxisTexts;

namespace renderer { class RenderData; }

#include "ViewportStyle.h"


// ProjectSession
// ProjectContext
// Project
// Workspace

class Application {
public:
    Application(int& argc, char** argv);
    ~Application();
    int exec();

private:
    void init(int& argc, char** argv);

private:
    // Core (UndoRedoManager, CommandSystem, Scene)
    DocumentManager* documentManager_;

    // Platform
    IPlatformRuntime* platformRuntime_;

    // Host
    IViewportHost* viewportHost_;
    QtMainWindowBinder* mainWindowBinder_;

    // Rendering
    IRenderer* renderer_;
    render::RenderScene* renderScene_;

    // Controllers
    UIController* uiController_;
    ViewportController* viewportController_;

    // Core observer
    UIObserver* uiObserver_;
    RenderSceneBuilder* builder_;

    // Application
    UI::ProjectManager* mainWindow_;
    QtMainWindowBinder* binder_;
    EditorSession* editorSession_;
    IInteractionTool* interactionTool_;
    Camera2D* camera2D_;
    Cpu2dPicker* picker_;
    OverlayModel* overlay_;
    CommandConsole* commandConsole_;
    AxisTexts* axisTexts_;
    app::ViewportStyle viewportStyle_;
};

#endif // APPLICATION_H_

