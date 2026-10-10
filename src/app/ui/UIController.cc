#include "UIController.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "../../core/Document.h"
#include "../../core/DocumentManager.h"
#include "../editor/CommandConsole.h"
#include "../platform/IViewportHost.h"
#include "../platform/QtViewportHost.h"
#include "../viewport/AxisTexts.h"
#include "../viewport/ViewportController.h"
#include "../viewport/render/RenderSceneBuilder.h"
#include "OpenGL2dRenderer.h"

UIController::UIController(std::vector<std::unique_ptr<DocumentView>>& views,
                           DocumentManager& manager,
                           IPlatformRuntime& platformRuntime,
                           UI::ProjectManager& projectManager,
                           QtViewportHost& host)
    : views_(views),
      manager_(manager),
      platformRuntime_(platformRuntime),
      viewportHost_(host),
      projectManager_(projectManager),
      reportPresenter_(projectManager),
      activeTabName_({}) {}

UIController::~UIController() {
    for (auto& view : views_) {
        view->editorSession_.setReportCallback({});
        viewportHost_.removeController(&view->viewportController_);
    }
}

DocumentView* UIController::findView(const std::string& name) const {
    for (const auto& view : views_) {
        if (view->document().name() == name) {
            return view.get();
        }
    }
    return nullptr;
}

void UIController::selectTool(ToolId tool, const std::string& tabName) {
    if (auto* view = findView(tabName.empty() ? activeTabName_ : tabName)) {
        view->editorSession_.select(tool);
        view->viewportController_.requestRedraw();
        viewportHost_.requestRedraw();
    }
}

void UIController::requestConstraint(const ConstraintRequest& request, const std::string& tabName) {
    const auto& name = tabName.empty() ? activeTabName_ : tabName;
    if (auto* view = findView(name)) {
        view->editorSession_.requestConstraint(request);
        view->viewportController_.requestRedraw();
        viewportHost_.requestRedraw();
    }
}

std::function<void(const ConstraintRequest&)> UIController::constraintRequestHandler(const std::string& tabName) {
    const auto* view = findView(tabName.empty() ? activeTabName_ : tabName);
    const Document* origin = view ? &view->document() : nullptr;
    return [this, origin](const ConstraintRequest& request) {
        for (const auto& candidate : views_) {
            if (&candidate->document() == origin) {
                requestConstraint(request, candidate->document().name());
                return;
            }
        }
    };
}

void UIController::updateSolverBackend(const std::string& tabName) {
    const auto* view = findView(tabName);
    if (!view) {
        return;
    }
    const Document* document = &view->document();

    using core::sketch::BackendKind;
    using core::sketch::Sketch;
    const auto current = document->sketch().backendKind();
    const auto next = current == BackendKind::Dcm ? BackendKind::SolveSpace : BackendKind::Dcm;
    const auto available = Sketch::availableBackends();
    const bool canSwitch = std::find(available.begin(), available.end(), next) != available.end();
    projectManager_.setSolverBackend(QString::fromStdString(tabName), current == BackendKind::Dcm ? QStringLiteral("DCM") : QStringLiteral("SS"),
                                    canSwitch);
}

void UIController::switchSolverBackend(const std::string& tabName) {
    auto* view = findView(tabName);
    if (!view) {
        return;
    }

    using core::sketch::BackendKind;
    const auto current = std::as_const(view->document()).sketch().backendKind();
    const auto next = current == BackendKind::Dcm ? BackendKind::SolveSpace : BackendKind::Dcm;
    view->editorSession_.switchSolverBackend(next);
    updateSolverBackend(tabName);
    view->viewportController_.requestRedraw();
    viewportHost_.requestRedraw();
}

void UIController::executeConsoleCommand(std::string str) {
    try {
        Document* document = manager_.at(activeIndex_);
        // UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
        // CommandManager& cm = document->commandManager();
        // Transaction* txn = cm.invoke(str);
        // urm.push(std::move(*txn));

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
    if (findView(fileName)) {
        setActiveFile(fileName);
        return;
    }
    if (auto* previous = findView(activeTabName_)) {
        previous->editorSession_.endInteraction();
    }
    activeTabName_ = fileName;
    DocumentId id;
    if (set_.contains(fileName)) {
        id = manager_.findIdByName(fileName);
    } else {
        id = manager_.createNewDocument(fileName);
    }

    auto view = std::make_unique<DocumentView>(*manager_.at(id), viewportHost_);
    Document* origin = &view->document();
    view->editorSession_.setReportCallback([this, origin](const ActionReport& report) { reportPresenter_.present(*origin, report); });

    //projectManager_.setCommandConsoleEngine(view.commandConsole_);
    int index = viewportHost_.setEventSink(&view->viewportController_);
    if (index >= 0) {
        activeIndex_ = index;
        indicesMap_[fileName] = index;
        controllerMap_[&view->viewportController_] = index;
    }

    views_.push_back(std::move(view));
    set_.insert(fileName);

    // this should be in the end
    projectManager_.addTabSlot(fileName.data());
    updateSolverBackend(fileName);
}

void UIController::setActiveFile(const std::string& fileName) {
    if (!findView(fileName)) {
        return;
    }
    if (activeTabName_ != fileName) {
        if (auto* previous = findView(activeTabName_)) {
            previous->editorSession_.endInteraction();
        }
    }

    activeTabName_ = fileName;
    int index = indicesMap_[fileName];
    viewportHost_.setActiveController(index);
    updateSolverBackend(fileName);
}

void UIController::removeFile(const std::string& fileName) {
    for (auto it = views_.begin(); it != views_.end(); ++it) {
        if ((*it)->document_.name() == fileName) {
            (*it)->editorSession_.setReportCallback({});
            const auto removed = controllerMap_.find(&(*it)->viewportController_);
            const int removedIndex = removed == controllerMap_.end() ? -1 : removed->second;
            viewportHost_.removeController(&(*it)->viewportController_);
            controllerMap_.erase(&(*it)->viewportController_);
            views_.erase(it);
            set_.erase(fileName);
            indicesMap_.erase(fileName);
            if (removedIndex >= 0) {
                for (auto& [name, index] : indicesMap_) {
                    if (index > removedIndex) {
                        --index;
                    }
                }
                for (auto& [controller, index] : controllerMap_) {
                    if (index > removedIndex) {
                        --index;
                    }
                }
            }
            if (activeTabName_ == fileName) {
                activeTabName_ = views_.empty() ? std::string{} : views_.front()->document().name();
                activeIndex_ = activeTabName_.empty() ? -1 : indicesMap_.at(activeTabName_);
            } else if (indicesMap_.contains(activeTabName_)) {
                activeIndex_ = indicesMap_.at(activeTabName_);
            }
            return;
        }
    }
}

void UIController::renameTab(const std::string& oldName, const std::string& newName) {
    auto* view = findView(oldName);
    if (!view || (oldName != newName && findView(newName))) {
        return;
    }
    int index = indicesMap_[oldName];
    indicesMap_.erase(oldName);
    indicesMap_[newName] = index;
    view->document().name() = newName;
    set_.erase(oldName);
    set_.insert(newName);
    if (activeTabName_ == oldName) {
        activeTabName_ = newName;
    }
    projectManager_.renameTabSlot(QString(oldName.data()), QString(newName.data()));
}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}











