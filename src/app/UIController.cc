#include "UIController.h"
#include "DocumentManager.h"
#include "Transaction.h"
#include "ConsoleManager.h"
#include "UndoRedo.h"
#include "Document.h"
#include "IViewportHost.h"

#include "OpenGLRenderer.h"
#include "AxisTexts.h"
#include "RenderDataBuilder.h"
#include "ViewportController.h"
#include "CommandConsole.h"
#include "IViewportHost.h"
#include "QtViewportHost.h"

#include <iostream>

UIController::UIController(DocumentManager& manager,
                           IPlatformRuntime& platformRuntime,
                           UI::ProjectManager& projectManager,
                           std::vector<Tab>& tabs,
                           QtViewportHost& host)
    : documentManager_(manager),
      platformRuntime_(platformRuntime),
      projectManager_(projectManager),
      tabs_(tabs),
      activeTabName_({}),
      viewportHost_(host) {}

void UIController::selectTool(ToolId tool, double value) {
    for (const auto& tab : tabs_) {
        if (tab.name_ == activeTabName_) {
            tab.editorSession_->select(tool, value);
        }
    }
}

void UIController::executeConsoleCommand(std::string str) {
    try {
        Document* document = documentManager_.getActiveDocument();
        UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
        CommandManager& cm = document->commandManager();
        Transaction* txn = cm.invoke(str);
        urm.push(std::move(*txn));

        for (const auto& tab : tabs_) {
            if (tab.name_ == activeTabName_) {
                viewportHost_.requestRedraw();
            }
        }
    }catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

void UIController::openProjectInNewWindow() {}

void UIController::createProjectInNewWindow() {}

void UIController::openProjectInCurrentWindow() {}

void UIController::createProjectInCurrentWindow() {}

void UIController::openFile() {}


void UIController::createFile(const std::string& fileName) {
    int fileIndex = documentManager_.createNewDocument(fileName);
    if (fileIndex == -1) {
        return;
    }

    indicesMap_[fileName] = fileIndex;

    activeTabName_ = fileName;

    Tab tab;
    tab.camera2D_ = new Camera2D();
    tab.overlay_ = new OverlayModel();
    tab.picker_ = new Cpu2dPicker(documentManager_.getActiveDocument()->scene(), *tab.camera2D_);
    tab.renderData_ = new renderer::RenderData();
    tab.editorSession_ = new EditorSession(documentManager_, *tab.camera2D_, *tab.renderData_, *tab.picker_, *tab.overlay_);
    tab.editorSession_->select(ToolId::Cursor);
    tab.renderer_ = new renderer::OpenGLRenderer();
    tab.axisTexts_ = new AxisTexts(*tab.camera2D_);
    tab.builder_ = new RenderDataBuilder(documentManager_.getActiveDocument()->scene(), *tab.overlay_, *tab.axisTexts_, *tab.renderData_);
    tab.viewportController_ = new ViewportController(
        *tab.camera2D_,
        *tab.editorSession_,
        *tab.renderer_,
        *tab.renderData_,
        *tab.builder_,
        *documentManager_.getActiveDocument());
    tab.commandConsole_ = new CommandConsole(*tab.overlay_, viewportHost_);
    projectManager_.setCommandConsoleEngine(tab.commandConsole_);
    viewportHost_.setEventSink(tab.viewportController_);

    tab.name_ = fileName;

    tabs_.push_back(tab);

    int index = viewportHost_.addController(tab.viewportController_);
    controllerMap_[tab.viewportController_] = index;

    // this should be in the end
    projectManager_.addTabSlot(fileName.data());
}

void UIController::setActiveFile(const std::string& fileName) {
    int fileIndex = indicesMap_.at(fileName);
    if (!documentManager_.setActiveDocument(fileIndex)) {
        throw "didn't find active document with this index";
    }
    for (const auto& tab : tabs_) {
        if (tab.name_ == fileName) {
            activeTabName_ = fileName;
            int index = controllerMap_.at(tab.viewportController_);
            viewportHost_.setActiveController(index);
            return;
        }
    }
    throw "didn't find active document with this file name";
}

void UIController::removeFile(const std::string& fileName) {
    for (auto it = tabs_.begin(); it != tabs_.end(); ++it) {
        if ((*it).name_ == fileName) {
            viewportHost_.removeController((*it).viewportController_);
            controllerMap_.erase((*it).viewportController_);
            tabs_.erase(it);
            return;
        }
    }
}

void UIController::renameTab(const std::string& oldName, const std::string& newName) {
    int index = indicesMap_[oldName];
    indicesMap_[newName] = index;
    if (activeTabName_ == oldName) {
        activeTabName_ = newName;
    }
    projectManager_.renameTabSlot(QString(oldName.data()), QString(newName.data()));
}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}











