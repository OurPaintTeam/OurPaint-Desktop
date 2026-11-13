#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <memory>

class QtMainWindowBinder;
class QtPlatformRuntime;
class UIController;
namespace UI {
class ProjectManager;
}

class Application {
public:
    Application(int& argc, char** argv);
    ~Application();
    int exec();

private:
    // Reverse destruction detaches the viewport before releasing the editor
    // and destroys all Qt windows before the platform runtime ends.
    std::unique_ptr<QtPlatformRuntime> platformRuntime_;
    std::unique_ptr<UI::ProjectManager> projectManager_;
    std::unique_ptr<UIController> uiController_;
    std::unique_ptr<QtMainWindowBinder> binder_;
};

#endif  // APPLICATION_H_
