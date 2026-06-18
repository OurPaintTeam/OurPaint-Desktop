#include "QtPlatformRuntime.h"
#include "QtViewportHost.h"

#include <QApplication>
#include <QSurfaceFormat>

QtPlatformRuntime::QtPlatformRuntime(int& argc, char** argv)
    : argc_(argc), argv_(argv) {}

QtPlatformRuntime::~QtPlatformRuntime() {
    for (auto* host : hosts_) {
        delete host;
    }
    //delete app_;
}

bool QtPlatformRuntime::init() {
    if (app_) {
        return true;
    }

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    app_ = new QApplication(argc_, argv_);

    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);

    return true;
}

int QtPlatformRuntime::run() {
    if (!app_) {
        return -1;
    }

    return app_->exec();
}

IViewportHost* QtPlatformRuntime::createViewportHost() {
    auto* host = new QtViewportHost(nextHostId_++);

    if (!hosts_.empty()) {
        // Shared context: создаём контекст с явным shareContext
        // QtViewportHost::initContext() вызовется позже при первом expose,
        // поэтому shared context передаём через setFormat до этого момента —
        // либо чуть усложняем и передаём shareContext в конструктор:
        // new QtViewportHost(nextHostId_++, hosts_[0]->context())
    }

    hosts_.push_back(host);
    return host;
}

void QtPlatformRuntime::destroyViewportHost(IViewportHost* host) {
    //auto* qtHost = static_cast<QtViewportHost*>(host);
    //auto  it     = std::find(hosts_.begin(), hosts_.end(), qtHost);
    //if (it == hosts_.end()) return;
    //delete *it;
    //hosts_.erase(it);
}




