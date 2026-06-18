#include "Application.h"

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>

#include "App/CustomConsole.h"
#include "CommandConsole.h"
#include "Document.h"
#include "DocumentManager.h"
#include "OpenGLRenderer.h"
#include "QtPlatformRuntime.h"
#include "QtViewportHost.h"
#include "RenderData.h"
#include "RenderDataBuilder.h"
#include "Scene.h"
#include "ViewportController.h"
#include "AxisTexts.h"


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
    // init qt platform
    platformRuntime_ = new QtPlatformRuntime(argc, argv);
    platformRuntime_->init();



    // init core
    documentManager_ = new DocumentManager();
    documentManager_->createNewDocument("newDocument");

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

    // init AxisTexts
    axisTexts_ = new AxisTexts(*camera2D_);

    // init core observer
    builder_ = new RenderDataBuilder(documentManager_->getActiveDocument()->scene(), *overlay_, *axisTexts_, *renderData_);

    // init viewport controller
    viewportController_ = new ViewportController(*camera2D_, *editorSession_, *renderer_, *renderData_, *builder_, *documentManager_);

    // set EventSink viewport controller to viewport host
    viewportHost_->setEventSink(viewportController_);

    // other
    editorSession_->select(ToolId::Cursor);

    // init UI
    Q_INIT_RESOURCE(resources);
    Q_INIT_RESOURCE(translations);
    commandConsole_ = new CommandConsole(*overlay_, *qt_host);
    mainWindow_ = new UI::ProjectManager({}, qt_host,commandConsole_);
    mainWindow_->addTabSlot("ds");

    // init UIController
    uiController_ = new UIController(*editorSession_, *documentManager_, *viewportHost_);

    // init binder
    binder_ = new QtMainWindowBinder(*mainWindow_, *uiController_);

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
































/*


Проверь правильно ли я понял функции ProjectManager:


void saveFullProjectTriggered();
- сохранить проект (при закрытии проекта или приложения либо от кнопки)

void openProjectThisWindowTriggered(const QString& path);
- открыть существующий проект в этом окне и закрыть старый если открыт( требует новый рендеринг и новую консоль)

void createProjectThisWindowTriggered(const QString& path);
- создать проект в этом окне и закрыть старый если открыт( требует новый рендеринг и новую консоль)

void openNewWindowOpenProjectTriggered(const QString& path);
- открыть в новом окне существующий проект( требует новый рендеринг и новую консоль)

void openNewWindowCreateProjectTriggered(const QString& path);
- открыть в новом окне новый проект.( требует новый рендеринг и новую консоль)

void openNewWindowTabTriggered(const QString& name);
- открытие окна вкладки (когда мы выносим вкладку в отдельное окно) ( требует новый рендеринг и новую консоль)

void renameProjectTriggered(const QString& newName, const QString& path);
- изменить имя проекта.

void deleteProjectTriggered(const QString& path);
- удалить проект. (Почему не по имени, или передавать два параметра?) ОТВЕТ а если корелляция? моя файловая система требует полный путь

void openFileTriggered(const QString& filePath);
- открыть файл/документ. в этом окне!

void createFileTriggered(const QString& fileName);
- создать файл/документ. в этом окне!

void renameTabTriggered(const QString& oldName, const QString& newName);
- изменить имя TAB???? а выше же был file. выше был проект renameProjectTriggered

void removeTabTriggered(const QString& fileName);
- закрыть tab, а почему не file или document? потому что у меня вкладки

void setActiveTabTriggered(const QString& name);
- установить активный документ, но почему то написано таб. потому что у меня вкладки

void returnTabWindowTriggered(const QString& name);
- что это? посмотри на 170 строку
это обратное действие я возвращаю вкладку в основное окно

void closeTabWindowTriggered(const QString& name);
- закрыть? а чем отличается от remove?
закрыть это закрыть
удалить это удалить
закрыть значит она вне системы
удалить это навсегда ее удалить


В общем не хватает в UI комментариев в публичном api.

*/



