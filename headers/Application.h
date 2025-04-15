#ifndef APPLICATION_H
#define APPLICATION_H

#include <QTranslator>
#include <QPixmap>
#include <memory>
#include <QApplication>
#include "mainwindow.h"
#include "QTPainter.h"
#include "Server.h"
#include "Client.h"

class Application {
public:

    Application(int &argc, char **argv);

    int exec();

private:
    QApplication app;
    std::unique_ptr<QTPainter> painter;
    MainWindow w;
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