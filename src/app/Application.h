#ifndef APPLICATION_H
#define APPLICATION_H

#include <QLocalServer>
#include <QObject>
#include <QStringList>
#include <memory>
#include <vector>

class QLocalServer;
class DocumentManager;
class IPlatformRuntime;
class IViewportHost;
class UIController;
class DocumentView;

class Application : public QObject {
    Q_OBJECT
public:
    Application(int& argc, char** argv, QObject* parent = nullptr);
    ~Application() override;

    int exec();

private:
    void init(int& argc, char** argv);
    bool setupSingleInstance(const QStringList& filePaths);
    bool trySendToRunningInstance(const QStringList& paths, int timeoutMs = 500);

    bool isSecondaryInstance_{false};
    IPlatformRuntime* platformRuntime_{nullptr};
    DocumentManager* documentManager_{nullptr};
    UIController* uiController_{nullptr};
    QLocalServer* localServer_{nullptr};
    std::vector<std::unique_ptr<DocumentView>> views_;
};

#endif  // APPLICATION_H