#ifndef APPLICATION_H_
#define APPLICATION_H_

#define GL_GLEXT_PROTOTYPES



#include "InteractionTools/EditorSession.h"
#include "QtMainWindowBinder.h"
#include "Project.h"

#include "Tab.h"

class DocumentManager;

class IPlatformRuntime;
class IViewportHost;

class UIController;

class UIObserver;


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

    // Rendering

    // Controllers
    UIController* uiController_;

    // Core observer
    UIObserver* uiObserver_;

    // Application
    std::vector<Tab> tabs_;

    // UI
    UI::ProjectManager* projectManager_;
    QtMainWindowBinder* binder_;
};

#endif // APPLICATION_H_

