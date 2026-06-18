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
        : documentManager_(nullptr),
          tabs_()
          {
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

    uiController_ = new UIController(*documentManager_,  *platformRuntime_, *projectManager_, tabs_);

    binder_ = new QtMainWindowBinder(*projectManager_, *uiController_);

}

int Application::exec() {
    return platformRuntime_->run();
}

Application::~Application() {}
































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



