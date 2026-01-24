#ifndef APPLICATION_H
#define APPLICATION_H

class QApplication;
class PainterController;
class MainWindController;
class LeftMenuController;
namespace UndoRedo { class UndoRedoManager; }
class CommandManager;
class SceneQtAdapter;
class LeftMenuBar;
class MainWindow;
class QTPainter;
class Scene;
class QString;

class Application {
public:
    Application(int& argc, char** argv);
    ~Application();
    int exec();

private:
    void initCore();
    void initGUI(int& argc, char** argv);
    void initLogger();
    void initControllers();

private:
    Scene* scene;
    UndoRedo::UndoRedoManager* undoRedo;
    CommandManager* commandManager;

    SceneQtAdapter* sqa;

    QApplication* app;
    MainWindow* mainWind;
    QTPainter* painter;
    LeftMenuBar* leftMenu;


    PainterController* pc;
    MainWindController* mwc;
    LeftMenuController* lmc;
};

#endif // APPLICATION_H

