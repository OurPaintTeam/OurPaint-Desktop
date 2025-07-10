#ifndef APPLICATION_H
#define APPLICATION_H

#include <QTranslator>
#include <QPixmap>
#include <memory>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QIcon>
#include <QTimer>

#include "Mainwindow.h"
#include "QTPainter.h"
#include "Server.h"
#include "Client.h"
#include "TreeModel.h"
#include "GUI_Logger.h"
#include "UndoRedo.h"
#include "ConsoleManager.h"

class PainterController;
class MainWindController;
class LeftMenuController;
//namespace UndoRedo { class UndoRedoManager; }
//class CommandManager;

class Application {
public:
    Application(int &argc, char **argv);
    int exec();

private:
    void initGUI();
    void initNetwork();
    void initLogger();
    void initCore();
    void initControllers();

private:
    QApplication app;
    MainWindow mainWind;
    QTPainter* painter;

    Scene* scene;
    UndoRedo::UndoRedoManager undoRedo;
    CommandManager commandManager;

    SceneQtAdapter sqa;
    LeftMenuBar* leftMenu;

    QString username;
    Server server;
    Client client;

    PainterController* pc;
    MainWindController* mwc;
    LeftMenuController* lmc;
};

#endif // APPLICATION_H

