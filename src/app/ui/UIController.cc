#include "UIController.h"

#include <iostream>

#include "../editor/CommandConsole.h"
#include "../platform/IViewportHost.h"
#include "../platform/QtViewportHost.h"
#include "../viewport/AxisTexts.h"
#include "../viewport/ViewportController.h"
#include "../viewport/render/RenderSceneBuilder.h"
#include "ConsoleManager.h"
#include "Document.h"
#include "DocumentManager.h"
#include "OpenGL2dRenderer.h"
#include "Transaction.h"
#include "UndoRedo.h"

UIController::UIController(std::vector<DocumentView*>& views,
                           DocumentManager& manager,
                           IPlatformRuntime& platformRuntime,
                           UI::ProjectManager& projectManager,
                           QtViewportHost& host)
    : views_(views),
      manager_(manager),
      platformRuntime_(platformRuntime),
      projectManager_(projectManager),
      activeTabName_({}),
      viewportHost_(host) {}

void UIController::selectTool(ToolId tool, double value) {
    for (auto& view : views_) {
        if (view->document().name() == activeTabName_) {
            view->editorSession_.select(tool, value);
        }
    }
}

void UIController::executeConsoleCommand(std::string str) {
    try {
        Document* document = manager_.at(activeIndex_);
        UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
        CommandManager& cm = document->commandManager();
        Transaction* txn = cm.invoke(str);
        urm.push(std::move(*txn));

        for (const auto& view : views_) {
            if (view->document_.name() == activeTabName_) {
                viewportHost_.requestRedraw();
            }
        }
    }catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

#include <cstdlib>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string getExecutablePath() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    return std::string(buffer);
#else
    char buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::string(buffer);
    }
    return "";
#endif
}

#ifdef _WIN32
void UIController::openProjectInNewWindow() {
    std::string path = getExecutablePath();

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;

    if (CreateProcessA(NULL, (LPSTR)path.c_str(), NULL, NULL,
                      FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}
#endif

#ifdef __linux__
void UIController::openProjectInNewWindow() {
    std::string path = getExecutablePath();

    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        execl(path.c_str(), path.c_str(), NULL);
        exit(0);
    }
}
#endif

void UIController::createProjectInNewWindow() {}

void UIController::openProjectInCurrentWindow() {}

void UIController::createProjectInCurrentWindow() {}

void UIController::openFile() {}


void UIController::createFile(const std::string& fileName) {
    activeTabName_ = fileName;
    DocumentId id;
    if (set_.contains(fileName)) {
        id = manager_.findIdByName(fileName);
    } else {
        id = manager_.createNewDocument(fileName);
    }

    DocumentView* view = new DocumentView(*manager_.at(id), viewportHost_);

    //projectManager_.setCommandConsoleEngine(view.commandConsole_);
    int index = viewportHost_.setEventSink(&view->viewportController_);
    if (index >= 0) {
        activeIndex_ = index;
        indicesMap_[fileName] = index;
        controllerMap_[&view->viewportController_] = index;
    }

    views_.push_back(view);
    set_.insert(fileName);

    // this should be in the end
    projectManager_.addTabSlot(fileName.data());
}

void UIController::setActiveFile(const std::string& fileName) {
    Document* document = manager_.at(fileName);
    if (!document) {
        return;
    }

    activeTabName_ = fileName;
    int index = indicesMap_[fileName];
    viewportHost_.setActiveController(index);
}

void UIController::removeFile(const std::string& fileName) {
    for (auto it = views_.begin(); it != views_.end(); ++it) {
        if ((*it)->document_.name() == fileName) {
            viewportHost_.removeController(&(*it)->viewportController_);
            controllerMap_.erase(&(*it)->viewportController_);
            views_.erase(it);
            set_.erase(fileName);
            indicesMap_.erase(fileName);
            return;
        }
    }
}

void UIController::renameTab(const std::string& oldName, const std::string& newName) {
    int index = indicesMap_[oldName];
    indicesMap_.erase(oldName);
    indicesMap_[newName] = index;
    Document* document = manager_.at(index);
    document->name() = newName;
    if (activeTabName_ == oldName) {
        activeTabName_ = newName;
    }
    projectManager_.renameTabSlot(QString(oldName.data()), QString(newName.data()));
}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}











