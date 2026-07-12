#include "Application.h"

#include "App/CustomConsole.h"
#include "DocumentManager.h"
#include "OpenGL2dRenderer.h"
#include "platform/QtPlatformRuntime.h"
#include "platform/QtViewportHost.h"
#include "Scene.h"

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

    uiController_->createFile("default");

    binder_ = new QtMainWindowBinder(*projectManager_, *uiController_);

    //project_ = new app::Project("new project");
}

int Application::exec() {
    return platformRuntime_->run();
}

Application::~Application() {
    delete documentManager_;
    delete platformRuntime_;
    delete host_;
    delete uiController_;
    delete projectManager_;
    delete binder_;
}
