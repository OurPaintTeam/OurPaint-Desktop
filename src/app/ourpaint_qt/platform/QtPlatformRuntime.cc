#include "QtPlatformRuntime.h"

#include <QApplication>
#include <QSurfaceFormat>

#include "QtViewportHost.h"

QtPlatformRuntime::QtPlatformRuntime(int& argc, char** argv)
    : argc_(argc), argv_(argv) {}

QtPlatformRuntime::~QtPlatformRuntime() {
    for (auto& host : hosts_) {
        if (!host.isNull()) {
            delete host.data();
        }
    }
    hosts_.clear();

    app_ = nullptr;
}

bool QtPlatformRuntime::init() {
    if (app_) {
        return true;
    }

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

QtViewportHost* QtPlatformRuntime::createViewportHost() {
    auto* host = new QtViewportHost(QGuiApplication::primaryScreen());

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

void QtPlatformRuntime::destroyViewportHost(QtViewportHost* host) {
    if (!host) return;
    auto it = std::find(hosts_.begin(), hosts_.end(), host);
    if (it == hosts_.end()) return;
    hosts_.erase(it);
    delete host;
}




