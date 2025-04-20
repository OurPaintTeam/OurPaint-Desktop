#ifndef APPLICATION_H
#define APPLICATION_H

#include <QTranslator>
#include <QPixmap>
#include <memory>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>

#include "Mainwindow.h"
#include "QTPainter.h"
#include "Server.h"
#include "Client.h"
#include "TreeModelLazy.h"

#include "Scene.h"
#include "Painter.h"

class Application {
private:
    QApplication app;
    MainWindow mainWind;
    Scene scene;
    Painter* painter;
    std::unique_ptr<LeftMenuBar> leftMenu;

    std::vector<std::function<void()>> vecCalls;

    QString username;
    Server server;
    Client client;

    void initialize();
    void setupQTPainterConnections();
    void setupServerConnections();
    void setupRequirementsConnections();
    void setupLeftMenuConnections();
    void setupAddingCommandsConnections();
    void updateState();
    void handler(const QString &command);

public:
    Application(int &argc, char **argv);
    int exec();
};

#endif // APPLICATION_H