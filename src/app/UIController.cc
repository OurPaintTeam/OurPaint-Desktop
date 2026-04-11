#include "UIController.h"
#include "DocumentManager.h"
#include "Transaction.h"
#include "ConsoleManager.h"
#include "UndoRedo.h"
#include "Document.h"

#include <iostream>

#include "QDebug"
UIController::UIController(EditorSession& editorSession, DocumentManager& manager)
    : editorSession_(editorSession), docManager_(manager) {}

void UIController::selectTool(ToolId tool) {
    editorSession_.select(tool);
}

void UIController::executeConsoleCommand(std::string str) {
    Document* document = docManager_.getActiveDocument();
    UndoRedo::UndoRedoManager& urm = document->undoRedoManager();
    CommandManager& cm = document->commandManager();
    Transaction* txn = cm.invoke(str);
    urm.push(std::move(*txn));
}

void UIController::openProjectInNewWindow() {}

void UIController::createProjectInNewWindow() {}

void UIController::openProjectInCurrentWindow() {}

void UIController::createProjectInCurrentWindow() {}

void UIController::openFile() {}

void UIController::renameTab() {}

void UIController::removeTab() {}

void UIController::createFile() {}

void UIController::renameProject() {}

void UIController::deleteProject() {}

void UIController::closeApplication() {}











