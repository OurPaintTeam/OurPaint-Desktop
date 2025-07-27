#include "CommandSetGridState.h"
#include "UndoRedoObserver.h"

bool UndoRedo::CommandSetGridState::Execute() {
    _oldState = _grid;
    _grid = _newState;
    return true;
}

bool UndoRedo::CommandSetGridState::Undo() {
    _grid = _oldState;
    return true;
}

UndoRedo::CommandSetGridState::CommandSetGridState(bool& grid, bool state)
    : _grid(grid), _newState(state), _oldState(grid) {}