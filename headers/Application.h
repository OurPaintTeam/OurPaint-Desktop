#ifndef OURPAINT_HEADERS_APPLICATION_H_
#define OURPAINT_HEADERS_APPLICATION_H_

#include <QTranslator>
#include <QPixmap>
#include <memory>
#include <QApplication>
#include "mainwindow.h"
#include "QTPainter.h"
#include "Server.h"
#include "Client.h"
#include "DrawMode.h"
#include "Scene.h"

#include "UndoRedo.h"
#include "Transactions.h"
#include "Command.h"

class Application {
public:
    Application(int &argc, char **argv);
    int exec();

private:
    QApplication app;
    MainWindow w;
    std::unique_ptr<QTPainter> painter;
    Scene scene;

    UndoRedo managerUR;

    QString username;

    Server server;
    Client client;
    bool isConnected;
    bool isServer;

    void init();
    void setupConnections();

    void updateState();
    void handler(const QString &command);
};

#endif // ! OURPAINT_HEADERS_APPLICATION_H_