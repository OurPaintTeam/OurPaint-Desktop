#ifndef APPLICATION_H_
#define APPLICATION_H_

class QApplication;
class PainterController;
class MainWindController;
class LeftMenuController;
class SceneQtAdapter;
class LeftMenuBar;
class MainWindow;
class QTPainter;
class QString;
class DocumentManager;

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
    DocumentManager* documentManager;

    SceneQtAdapter* sqa;

    QApplication* app;
    MainWindow* mainWind;
    QTPainter* painter;
    LeftMenuBar* leftMenu;

    PainterController* pc;
    MainWindController* mwc;
    LeftMenuController* lmc;
};

#endif // APPLICATION_H_

