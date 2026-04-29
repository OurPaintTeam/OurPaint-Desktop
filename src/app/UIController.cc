#include "UIController.h"
#include "DocumentManager.h"
#include "saveload/DocumentSaveLoad.h"
#include "Transaction.h"
#include "ConsoleManager.h"
#include "UndoRedo.h"
#include "Document.h"
#include "IViewportHost.h"

#include <filesystem>
#include <iostream>

#include "QDebug"
UIController::UIController(EditorSession& editorSession,
                           DocumentManager& manager,
                           IViewportHost& host,
                           RenderDataBuilder& builder,
                           Cpu2dPicker& picker,
                           OverlayModel& overlay)
    : editorSession_(editorSession),
      docManager_(manager),
      host_(host),
      builder_(builder),
      picker_(picker),
      overlay_(overlay) {}

void UIController::selectTool(ToolId tool) {
    editorSession_.select(tool);
}

void UIController::executeConsoleCommand(std::string str) {
    try {
        Document* document = docManager_.getActiveDocument();
        UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
        CommandManager& cm = document->commandManager();
        Transaction* txn = cm.invoke(str);
        urm.push(std::move(*txn));

        host_.requestRedraw();
    }catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

void UIController::openProjectInNewWindow() {}

void UIController::createProjectInNewWindow() {}

void UIController::openProjectInCurrentWindow() {}

void UIController::createProjectInCurrentWindow() {}

std::string UIController::openFile(const std::string& filePath) {
    const int index = docManager_.openDocument(filePath);
    if (index < 0) {
        return {};
    }

    syncActiveDocument();
    const Document* document = docManager_.getActiveDocument();
    return document ? document->name() : std::string{};
}

bool UIController::saveActiveDocument() {
    Document* document = docManager_.getActiveDocument();
    if (document == nullptr) {
        return false;
    }
    if (!saveDocumentToFile(document->path(), *document)) {
        return false;
    }
    document->markClean();
    return true;
}

std::string UIController::saveActiveDocumentAs(const std::string& filePath) {
    Document* document = docManager_.getActiveDocument();
    if (document == nullptr || filePath.empty()) {
        return {};
    }

    const std::string oldName = document->name();
    const std::string oldPath = document->path();
    std::string newName = std::filesystem::path(filePath).stem().string();
    if (newName.empty()) {
        newName = oldName;
    }

    document->setName(newName);
    document->setPath(filePath);
    if (!saveDocumentToFile(filePath, *document)) {
        document->setName(oldName);
        document->setPath(oldPath);
        return {};
    }

    document->markClean();
    return document->name();
}

bool UIController::setActiveDocument(const std::string& documentName) {
    if (!docManager_.setActiveDocument(documentName)) {
        return false;
    }
    syncActiveDocument();
    return true;
}

std::string UIController::activeDocumentName() {
    const Document* document = docManager_.getActiveDocument();
    return document ? document->name() : std::string{};
}

void UIController::renameTab() {}

void UIController::removeTab() {}

std::string UIController::createFile(const std::string& documentName) {
    const int index = docManager_.createNewDocument(documentName);
    if (index < 0) {
        return {};
    }

    syncActiveDocument();
    const Document* document = docManager_.getActiveDocument();
    return document ? document->name() : std::string{};
}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}

void UIController::syncActiveDocument() {
    Document* document = docManager_.getActiveDocument();
    if (document == nullptr) {
        return;
    }

    overlay_.clear();
    const Document& activeDocument = *document;
    builder_.setScene(activeDocument.scene());
    picker_.setScene(activeDocument.scene());
    host_.requestRedraw();
}











