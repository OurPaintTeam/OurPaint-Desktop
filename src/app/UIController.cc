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

#include <iostream>

UIController::UIController(DocumentManager& manager,
                           IPlatformRuntime& platformRuntime,
                           UI::ProjectManager& projectManager,
                           std::vector<Tab>& tabs)
    : documentManager_(manager),
      platformRuntime_(platformRuntime),
      projectManager_(projectManager),
      tabs_(tabs),
      activeTabName_({}) {}

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
                tab.viewportHost_->requestRedraw();
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

void UIController::renameTab() {}

void UIController::removeTab() {}

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
    tab.viewportHost_ = platformRuntime_.createViewportHost();
    tab.viewportHost_->setEventSink(tab.viewportController_);
    tab.commandConsole_ = new CommandConsole(*tab.overlay_, *tab.viewportHost_);
    projectManager_.setCommandConsoleEngine(tab.commandConsole_);

    QtViewportHost* qt_host = static_cast<QtViewportHost*>(tab.viewportHost_);
    projectManager_.setQWindowRender(qt_host);

    tab.name_ = fileName;



    tabs_.push_back(tab);

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
            QtViewportHost* qt_host = static_cast<QtViewportHost*>(tab.viewportHost_);
            projectManager_.setQWindowRender(qt_host);
            activeTabName_ = fileName;
            return;
        }
    }
    throw "didn't find active document with this file name";
}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}











