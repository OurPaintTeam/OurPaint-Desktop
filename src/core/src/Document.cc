#include "Document.h"
#include "Scene.h"
#include "UndoRedo.h"
#include "ConsoleManager.h"

Document::Document(const std::string& documentName) {
    documentName_ = documentName;
    scene_ = new core::Scene();
    undoRedo_ = new UndoRedo::UndoRedoManager(100);
    commandManager_ = new CommandManager();

    commandManager_->registerFactory(new PointFactory(*scene_));
    commandManager_->registerFactory(new LineFactory(*scene_));
    commandManager_->registerFactory(new CircleFactory(*scene_));
    commandManager_->registerFactory(new ArcFactory(*scene_));
    commandManager_->registerFactory(new ReqFactory(*scene_));
    commandManager_->registerFactory(new DelFactory(*scene_));
    commandManager_->registerFactory(new ClearFactory(scene_));
    commandManager_->registerFactory(new CubicBezierFactory(*scene_));
}
Document::~Document() {
    delete scene_;
    delete undoRedo_;
    delete commandManager_;
}

std::string& Document::name() {
    isDirty_ = true;
    return documentName_;
}
const std::string& Document::name() const {
    return documentName_;
}

std::string& Document::path() {
    isDirty_ = true;
    return filePath_;
}
const std::string& Document::path() const {
    return filePath_;
}

core::Scene& Document::scene() {
    isDirty_ = true;
    return *scene_;
}
const core::Scene& Document::scene() const {
    return *scene_;
}

UndoRedo::UndoRedoManager& Document::undoRedoManager() {
    isDirty_ = true;
    return *undoRedo_;
}
const UndoRedo::UndoRedoManager& Document::undoRedoManager() const {
    return *undoRedo_;
}

CommandManager& Document::commandManager() {
    isDirty_ = true;
    return *commandManager_;
}
const CommandManager& Document::commandManager() const {
    return *commandManager_;
}

bool Document::isDirty() const {
    return isDirty_;
}
