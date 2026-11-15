#pragma once

#include <QApplication>
#include <QLocalServer>
#include <memory>

#include "UIController.h"

class QtUIController;

// Composition root: Qt owns its event loop/windows; App owns documents and views.
class Application {
public:
    Application(int& argc, char** argv);
    ~Application();
    int exec();

private:
    bool setupSingleInstance(const QStringList& paths);
    static bool trySendToRunningInstance(const QStringList& paths);
    QApplication qt_;
    app::UIController controller_;
    std::unique_ptr<QLocalServer> server_;
    std::unique_ptr<QtUIController> ui_;
    bool secondary_ = false;
};
