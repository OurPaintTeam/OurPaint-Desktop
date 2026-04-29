#include "Document.h"
#include "Scene.h"
#include "UndoRedo.h"
#include "ConsoleManager.h"

Document::Document(const std::string& documentName) {
    _documentName = documentName;
    _scene = new Scene();
    _undoRedo = new UndoRedo::UndoRedoManager(100);
    _commandManager = new CommandManager();

    _commandManager->registerFactory(new PointFactory(*_scene));
    _commandManager->registerFactory(new LineFactory(*_scene));
    _commandManager->registerFactory(new CircleFactory(*_scene));
    _commandManager->registerFactory(new ArcFactory(*_scene));
    _commandManager->registerFactory(new ReqFactory(*_scene));
    _commandManager->registerFactory(new DelFactory(*_scene));
    _commandManager->registerFactory(new ClearFactory(_scene));
    _commandManager->registerFactory(new CubicBezierFactory(*_scene));
}
Document::~Document() {
    delete _scene;
    delete _undoRedo;
    delete _commandManager;
}

std::string& Document::name() {
    _isDirty = true;
    return _documentName;
}
const std::string& Document::name() const {
    return _documentName;
}

void Document::setName(const std::string& documentName) {
    _documentName = documentName;
    _isDirty = true;
}

std::string& Document::path() {
    _isDirty = true;
    return _filePath;
}
const std::string& Document::path() const {
    return _filePath;
}

void Document::setPath(const std::string& filePath) {
    _filePath = filePath;
    _isDirty = true;
}

Scene& Document::scene() {
    _isDirty = true;
    return *_scene;
}
const Scene& Document::scene() const {
    return *_scene;
}

UndoRedo::UndoRedoManager& Document::undoRedoManager() {
    _isDirty = true;
    return *_undoRedo;
}
const UndoRedo::UndoRedoManager& Document::undoRedoManager() const {
    return *_undoRedo;
}

CommandManager& Document::commandManager() {
    _isDirty = true;
    return *_commandManager;
}
const CommandManager& Document::commandManager() const {
    return *_commandManager;
}

bool Document::isDirty() const {
    return _isDirty;
}

void Document::markDirty() {
    _isDirty = true;
}

void Document::markClean() {
    _isDirty = false;
}
