#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include <QApplication>
#include "MainWindow.h"
#include "QTPainter.h"
#include "Server.h"
#include "Client.h"
#include "Paint.h"

class Application {
public:
    Application(int &argc, char **argv);
    int exec();

private:
    QApplication app;
    MainWindow w;
    std::unique_ptr<QTPainter> painter;
    Paint screen;

    QString username;

    Server server;
    Client client;

    void initialize();
    void setupConnections();

    bool isConnected;
    bool isServer;

    void updateState();
    void handler(const QString &command);
};

#endif // APPLICATION_H