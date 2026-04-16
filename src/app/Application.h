#ifndef APPLICATION_H_
#define APPLICATION_H_

#define GL_GLEXT_PROTOTYPES

#include "IRenderer.h"
#include "InteractionTools/EditorSession.h"
#include "InteractionTools/IInteractionTool.h"
#include "QtMainWindowBinder.h"
#include "Camera2D.h"
#include "Cpu2dPicker.h"
#include "OverlayModel.h"

class DocumentManager;

class IPlatformRuntime;
class IViewportHost;

class UIController;
class ViewportController;

class UIObserver;
class RenderDataBuilder;

namespace renderer { class RenderData; }


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
    renderer::RenderData* renderData_;

    // Controllers
    UIController* uiController_;
    ViewportController* viewportController_;

    // Core observer
    UIObserver* uiObserver_;
    RenderDataBuilder* builder_;

    // Application
    UI::ProjectManager* mainWindow_;
    QtMainWindowBinder* binder_;
    EditorSession* editorSession_;
    IInteractionTool* interactionTool_;
    Camera2D* camera2D_;
    Cpu2dPicker* picker_;
    OverlayModel* overlay_;
};

#endif // APPLICATION_H_

