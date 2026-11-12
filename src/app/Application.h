#ifndef APPLICATION_H
#define APPLICATION_H

#include <QLocalServer>
#include <QObject>
#include <memory>
#include <vector>

class DocumentManager;
class IPlatformRuntime;
class DocumentController;
class QtUIController;
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
    static bool trySendToRunningInstance(const QStringList& paths, int timeoutMs = 500);

    bool isSecondaryInstance_{false};

    IPlatformRuntime* platformRuntime_{nullptr};
    DocumentManager* documentManager_{nullptr};
    DocumentController* documentController_{nullptr};
    QtUIController* qtUIController_{nullptr};
    QLocalServer* localServer_{nullptr};

    std::vector<std::unique_ptr<DocumentView>> views_;
};

#endif // APPLICATION_H