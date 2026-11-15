#include "Application.h"

#include <QCommandLineParser>
#include <QDataStream>
#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>
#include <QStandardPaths>
#include <QTimer>

#include "QtUIController.h"

namespace {
constexpr auto serverName = "OurPaintSingleInstance";
}

Application::Application(int& argc, char** argv) : qt_(argc, argv) {
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(translations);
    QCoreApplication::setOrganizationName(QStringLiteral("OurPaint"));
    QCoreApplication::setApplicationName(QStringLiteral("OurPaint"));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument(QStringLiteral("files"), QStringLiteral("Paths to .ourp documents."), QStringLiteral("[files...]"));
    parser.process(qt_);
    QStringList paths;
    for (const auto& path : parser.positionalArguments()) {
        paths.push_back(QFileInfo(path).absoluteFilePath());
    }
    if (!setupSingleInstance(paths)) {
        secondary_ = true;
        return;
    }

    // The current renderer resolves shaders/fonts relative to the process directory.
    // Resolve incoming filenames first so command-line paths keep their original meaning.
    if (!QDir::setCurrent(QCoreApplication::applicationDirPath())) {
        qWarning("Could not select the application resource directory");
    }
    const auto config = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    const auto metadata = QDir(config).filePath(QStringLiteral(".ourpaint"));
#ifdef Q_OS_WIN
    controller_.loadApplicationMetadata(metadata.toStdWString());
#else
    controller_.loadApplicationMetadata(std::filesystem::u8path(metadata.toUtf8().constData()));
#endif
    ui_ = std::make_unique<QtUIController>(controller_, config);
    ui_->openFilesFromCommandLine(paths);
}

Application::~Application() {
    server_.reset();
    ui_.reset();
}

bool Application::trySendToRunningInstance(const QStringList& paths) {
    QLocalSocket socket;
    socket.connectToServer(QString::fromLatin1(serverName));
    if (!socket.waitForConnected(500)) {
        return false;
    }
    QDataStream stream(&socket);
    stream << paths;
    socket.flush();
    return socket.bytesToWrite() == 0 || socket.waitForBytesWritten(500);
}

bool Application::setupSingleInstance(const QStringList& paths) {
    if (trySendToRunningInstance(paths)) {
        return false;
    }
    server_ = std::make_unique<QLocalServer>();
    server_->setSocketOptions(QLocalServer::UserAccessOption);
    if (!server_->listen(QString::fromLatin1(serverName))) {
        if (trySendToRunningInstance(paths)) {
            return false;
        }
        // A previous process may have left a stale local socket on Unix.
        if (server_->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(QString::fromLatin1(serverName));
            server_->listen(QString::fromLatin1(serverName));
        }
        if (!server_->isListening()) {
            qWarning() << "Single-instance listener unavailable:" << server_->errorString();
        }
    }
    QObject::connect(server_.get(), &QLocalServer::newConnection, server_.get(), [this] {
        while (auto* socket = server_->nextPendingConnection()) {
            const auto read = [this, socket] {
                QDataStream stream(socket);
                stream.startTransaction();
                QStringList received;
                stream >> received;
                if (!stream.commitTransaction()) {
                    return;  // Wait for the rest of a fragmented message.
                }
                if (ui_) {
                    ui_->openFilesFromCommandLine(received);
                }
                socket->disconnectFromServer();
                socket->deleteLater();
            };
            QObject::connect(socket, &QLocalSocket::readyRead, socket, read);
            QObject::connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
            if (socket->bytesAvailable()) {
                QTimer::singleShot(0, socket, read);
            }
        }
    });
    return true;
}

int Application::exec() { return secondary_ ? 0 : qt_.exec(); }
