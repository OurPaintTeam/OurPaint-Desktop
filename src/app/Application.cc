#include "Application.h"

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>

#include "App/CustomConsole.h"
#include "Document.h"
#include "DocumentManager.h"
#include "OpenGLRenderer.h"
#include "QtPlatformRuntime.h"
#include "QtViewportHost.h"
#include "RenderData.h"
#include "RenderDataBuilder.h"
#include "Scene.h"
#include "ViewportController.h"

Application::Application(int& argc, char** argv)
        : documentManager_(nullptr)
          {
    try {
        init(argc, argv);
    } catch (std::exception& e) {
        std::cout << e.what();
    }
}

void Application::init(int& argc, char** argv) {

    // init core
    documentManager_ = new DocumentManager();
    documentManager_->createNewDocument("newDocument");


    // init qt platform
    platformRuntime_ = new QtPlatformRuntime(argc, argv);
    platformRuntime_->init();


    // init viewport host
    viewportHost_ = platformRuntime_->createViewportHost();
    QtViewportHost* qt_host = static_cast<QtViewportHost*>(viewportHost_);

    // init Camera2D
    camera2D_ = new Camera2D();

    // init render scene
    renderData_ = new renderer::RenderData();
    renderData_->overlay.points.reserve(32);
    renderData_->overlay.lines.reserve(32);
    renderData_->overlay.circles.reserve(32);

    // Overlay
    overlay_ = new OverlayModel();

    // picker
    picker_ = new Cpu2dPicker(documentManager_->getActiveDocument()->scene(), *camera2D_);

    // init app
    editorSession_ = new EditorSession(*documentManager_, *camera2D_, *renderData_, *picker_, *overlay_);

    // init renderer
    renderer_ = new renderer::OpenGLRenderer();

    // init core observer
    builder_ = new RenderDataBuilder(documentManager_->getActiveDocument()->scene(), *overlay_, *renderData_);

    // init viewport controller
    viewportController_ = new ViewportController(*camera2D_, *editorSession_, *renderer_, *renderData_, *builder_);

    // set EventSink viewport controller to viewport host
    viewportHost_->setEventSink(viewportController_);




    // init UI
    Q_INIT_RESOURCE(resources);
    mainWindow_ = new UI::ProjectManager({}, qt_host,nullptr);
    mainWindow_->addTabSlot("ds");

    // init UIController
    uiController_ = new UIController(*editorSession_, *documentManager_, *viewportHost_);

    // init binder
    binder_ = new QtMainWindowBinder(*mainWindow_, *uiController_);

    // other
    editorSession_->select(ToolId::Cursor);

}

int Application::exec() {
    return platformRuntime_->run();
}

Application::~Application() {
    /* free core */
    delete documentManager_;

    /* free platform */
    delete platformRuntime_;
    //delete viewportHost_;

    /* free controllers*/
    //delete UIController_;
    delete viewportController_;

    /* free observers*/
    //delete UIObserver_;
    //delete viewportObserver_;
}



