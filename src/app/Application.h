#ifndef APPLICATION_H_
#define APPLICATION_H_

#define GL_GLEXT_PROTOTYPES

#include "UI/QtMainWindowBinder.h"
#include "editor/EditorSession.h"
#include "project/DocumentView.h"
#include "project/Project.h"

class DocumentManager;

class IPlatformRuntime;
class IViewportHost;

class UIController;

class UIObserver;

#include "ViewportStyle.h"

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
    IViewportHost* host_;

    // Controllers
    UIController* uiController_;

    // Core observer
    //UIObserver* uiObserver_;

    // Application
    std::vector<DocumentView*> views_;

    // UI
    UI::ProjectManager* projectManager_;
    QtMainWindowBinder* binder_;

    //app::Project* project_;
    app::ViewportStyle viewportStyle_;
};

#endif // APPLICATION_H_

