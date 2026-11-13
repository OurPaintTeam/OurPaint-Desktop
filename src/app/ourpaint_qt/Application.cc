#include "Application.h"

#include <QCoreApplication>

#include "QtMainWindowBinder.h"
#include "QtPlatformRuntime.h"
#include "QtViewportHost.h"

Application::Application(int& argc, char** argv) : platformRuntime_(std::make_unique<QtPlatformRuntime>(argc, argv)) {
    platformRuntime_->init();

    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(translations);

    projectManager_ = std::make_unique<UI::ProjectManager>(UI::ProjectManager::ProjectData{}, nullptr, nullptr);
    auto* host = platformRuntime_->createViewportHost();
    projectManager_->setQWindowRender(host);
    uiController_ = std::make_unique<UIController>();
    binder_ = std::make_unique<QtMainWindowBinder>(*projectManager_, *uiController_, *host);

    const auto arguments = QCoreApplication::arguments();
    bool opened = false;
    try {
        if (arguments.size() == 3 && arguments[1] == QStringLiteral("--open-project")) {
            opened = binder_->openProjectInCurrentWindow(arguments[2]);
        } else if (arguments.size() == 3 && arguments[1] == QStringLiteral("--create-project")) {
            opened = binder_->createProjectInCurrentWindow(arguments[2]);
        }
    } catch (const std::exception& error) {
        projectManager_->addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open the project: %1")
                                             .arg(QString::fromUtf8(error.what())));
    }
    if (!opened) {
        binder_->showNewView(uiController_->createDocument("default"));
    }

}

int Application::exec() {
    return platformRuntime_->run();
}

Application::~Application() = default;
