#include "QtPlatformRuntime.h"

#include <algorithm>
#include <QApplication>
#include <QSurfaceFormat>

#include "QtViewportHost.h"

QtPlatformRuntime::QtPlatformRuntime(int& argc, char** argv)
    : argc_(argc), argv_(argv) {}

QtPlatformRuntime::~QtPlatformRuntime() {
    for (const auto& host : hosts_) {
        delete host.data();
    }
    delete app_;
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

QtViewportHost* QtPlatformRuntime::createViewportHost() {
    auto* host = new QtViewportHost(nextHostId_++);

    hosts_.push_back(host);
    return host;
}

void QtPlatformRuntime::destroyViewportHost(QtViewportHost* host) {
    const auto it = std::find(hosts_.begin(), hosts_.end(), host);
    if (it != hosts_.end()) {
        hosts_.erase(it);
        delete host;
    }
}
