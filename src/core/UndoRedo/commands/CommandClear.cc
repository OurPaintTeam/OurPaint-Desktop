#include "CommandClear.h"

UndoRedo::CommandClear::CommandClear(core::Scene*& scene) : _scene(scene) {}

UndoRedo::CommandClear::~CommandClear() {
    // TODO исправить, после лимита истории сцена может удалиться.
    delete _sceneAfter;
}

bool UndoRedo::CommandClear::doExecute() noexcept {
    _sceneBefore = _scene;
    _sceneAfter = new core::Scene();
    _scene = _sceneAfter;
    return true;
}
bool UndoRedo::CommandClear::doUndo() noexcept {
    _scene = _sceneBefore;
    return true;
}

bool UndoRedo::CommandClear::doRedo() noexcept {
    _scene = _sceneAfter;
    return true;
}
