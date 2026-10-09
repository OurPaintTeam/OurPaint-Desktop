#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <memory>
#include <vector>

#define GL_GLEXT_PROTOTYPES

#include "editor/SketchEditor.h"
#include "project/DocumentView.h"
#include "project/Project.h"
#include "ui/QtMainWindowBinder.h"

class DocumentManager;

class IPlatformRuntime;
class IViewportHost;

class UIController;

class UIObserver;

class Application {
public:
    Application(int& argc, char** argv);
    ~Application();
    int exec();

private:
    void init(int& argc, char** argv);

private:
    // Core (UndoRedoManager, CommandSystem, Scene)
    DocumentManager* documentManager_ = nullptr;

    // Platform
    IPlatformRuntime* platformRuntime_ = nullptr;

    // Host
    IViewportHost* host_ = nullptr;

    // Controllers
    UIController* uiController_ = nullptr;

    // Core observer
    //UIObserver* uiObserver_;

    // Application
    std::vector<std::unique_ptr<DocumentView>> views_;

    // UI
    UI::ProjectManager* projectManager_ = nullptr;
    QtMainWindowBinder* binder_ = nullptr;

    //app::Project* project_;
};

#endif // APPLICATION_H_

