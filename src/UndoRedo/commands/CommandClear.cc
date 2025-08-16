#include "CommandClear.h"

UndoRedo::CommandClear::CommandClear(Scene*& scene) : _scene(scene) {}

UndoRedo::CommandClear::~CommandClear() {
    // TODO исправить, после лимита истории сцена может удалиться.
    delete _sceneAfter;
}

bool UndoRedo::CommandClear::Execute() {
    _sceneBefore = _scene;
    _sceneAfter = new Scene(nullptr);
    _scene = _sceneAfter;
    return true;
}
bool UndoRedo::CommandClear::Undo() {
    _scene = _sceneBefore;
    return true;
}

bool UndoRedo::CommandClear::Redo() {
    _scene = _sceneAfter;
    return true;
}
