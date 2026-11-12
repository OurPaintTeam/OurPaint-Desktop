#include "Application.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDataStream>
#include <QLocalSocket>
#include <QThread>
#include <iostream>

#include "../core/DocumentManager.h"
#include "DocumentController.h"

#include "QtUIController.h"
#include "platform/QtPlatformRuntime.h"

#ifdef Q_OS_WIN
#include "ui/RegWin.h"
#endif

namespace {
    constexpr const char* kServerName = "OurPaintSingleInstance";
}

Application::Application(int& argc, char** argv, QObject* parent) : QObject(parent) {
    try {
        init(argc, argv);
    } catch (std::exception& e) {
        std::cout << e.what();
    }
}

void Application::init(int& argc, char** argv) {
    platformRuntime_ = new QtPlatformRuntime(argc, argv);
    platformRuntime_->init();

    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(translations);

    QCoreApplication::setOrganizationName("OurPaint");
    QCoreApplication::setApplicationName("OurPaint");

#ifdef Q_OS_WIN
    QSettings appSettings;
    if (!appSettings.value("fileAssociationRegistered", false).toBool()) {
        registerFileAssociation();
        appSettings.setValue("fileAssociationRegistered", true);
    }
#endif

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("files", QCoreApplication::translate("main", "Paths to .ourp project files."), "[files...]");
    parser.process(QCoreApplication::arguments());

    QStringList filePaths;
    for (const QString& f : parser.positionalArguments()) {
        if (QFileInfo(f).suffix().compare("ourp", Qt::CaseInsensitive) == 0) {
            filePaths.append(f);
        } else {
            qWarning() << "[main] Not a .ourp file:" << f;
        }
    }

    if (!setupSingleInstance(filePaths)) {
        return;
    }

    documentManager_ = new DocumentManager();
    documentController_ = new DocumentController(*documentManager_);

    qtUIController_ = new QtUIController(views_, *documentController_, *platformRuntime_);

    if (!filePaths.isEmpty()) {
        qtUIController_->openFilesFromCommandLine(filePaths);
    }
}

bool Application::setupSingleInstance(const QStringList& filePaths) {
    if (trySendToRunningInstance(filePaths)) {
        isSecondaryInstance_ = true;
        return false;
    }

    localServer_ = new QLocalServer(this);
    bool isServer = localServer_->listen(kServerName);

    if (!isServer) {
        for (int i = 0; i < 10 && !isServer; ++i) {
            if (trySendToRunningInstance(filePaths, 200)) {
                isSecondaryInstance_ = true;
                return false;
            }
            QThread::msleep(50);
            isServer = localServer_->listen(kServerName);
        }
    }

    if (!isServer) {
        QLocalServer::removeServer(kServerName);
        isServer = localServer_->listen(kServerName);
        if (!isServer) {
            qWarning() << "[main] Cannot listen:" << localServer_->errorString();
        }
    }

    if (isServer) {
        connect(localServer_, &QLocalServer::newConnection, this, [this]() {
            while (QLocalSocket* client = localServer_->nextPendingConnection()) {
                connect(client, &QLocalSocket::readyRead, this, [this, client]() {
                    QDataStream stream(client);
                    QStringList received;
                    stream >> received;

                    if (!received.isEmpty() && qtUIController_) {
                        qtUIController_->openFilesFromCommandLine(received);
                    }
                    client->deleteLater();
                });
            }
        });
    }

    return true;
}

bool Application::trySendToRunningInstance(const QStringList& paths, int timeoutMs) {
    if (paths.isEmpty()) return false;

    QLocalSocket socket;
    socket.connectToServer(kServerName);
    if (!socket.waitForConnected(timeoutMs)) return false;

    QDataStream stream(&socket);
    stream << paths;
    socket.flush();
    socket.waitForBytesWritten(timeoutMs);
    return true;
}

int Application::exec() {
    if (isSecondaryInstance_) {
        return 0;
    }
    return platformRuntime_->run();
}

Application::~Application() {

    delete qtUIController_;
    views_.clear();

    delete documentController_;
    delete documentManager_;

    delete platformRuntime_;
}