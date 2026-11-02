#include "Application.h"

#include <QCoreApplication>

#include "../core/DocumentManager.h"
#include "../core/Scene.h"
#include "App/CustomConsole.h"
#include "OpenGL2dRenderer.h"
#include "platform/QtPlatformRuntime.h"
#include "platform/QtViewportHost.h"

Application::Application(int& argc, char** argv) : documentManager_(nullptr) {
    try {
        init(argc, argv);
    } catch (std::exception& e) {
        std::cout << e.what();
    }
}

void Application::init(int& argc, char** argv) {
    platformRuntime_ = new QtPlatformRuntime(argc, argv);
    platformRuntime_->init();

    documentManager_ = new DocumentManager();

    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(translations);

    projectManager_ = new UI::ProjectManager({}, nullptr, nullptr);

    host_ = platformRuntime_->createViewportHost();
    QtViewportHost* qt_host = static_cast<QtViewportHost*>(host_);
    projectManager_->setQWindowRender(qt_host);

    uiController_ = new UIController(views_, *documentManager_,  *platformRuntime_, *projectManager_, *qt_host);

    binder_ = new QtMainWindowBinder(*projectManager_, *uiController_);

    const auto arguments = QCoreApplication::arguments();
    bool opened = false;
    try {
        if (arguments.size() == 3 && arguments[1] == QStringLiteral("--open-project")) {
            opened = uiController_->openProjectInCurrentWindow(arguments[2]);
        } else if (arguments.size() == 3 && arguments[1] == QStringLiteral("--create-project")) {
            opened = uiController_->createProjectInCurrentWindow(arguments[2]);
        }
    } catch (const std::exception& error) {
        projectManager_->addNotification(QCoreApplication::translate("ProjectPersistence", "Could not open the project: %1")
                                             .arg(QString::fromUtf8(error.what())));
    }
    if (!opened) {
        uiController_->createFile("default");
    }

    //project_ = new app::Project("new project");
}

int Application::exec() {
    return platformRuntime_->run();
}

Application::~Application() {
    delete binder_;
    // Disconnect editor callbacks and viewport sinks while their owners live.
    delete uiController_;
    views_.clear();
    delete projectManager_;
    delete documentManager_;
    // The platform runtime owns host_, including its OpenGL contexts.
    delete platformRuntime_;
}
