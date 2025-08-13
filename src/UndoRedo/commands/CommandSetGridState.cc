#include "CommandSetGridState.h"
#include "GridSnap.h"

bool UndoRedo::CommandSetGridState::Execute() {
    oldState = _gridSnap.getGridState();
    _gridSnap.setGridState(newState);
    return true;
}

bool UndoRedo::CommandSetGridState::Undo() {
    _gridSnap.setGridState(oldState);
    return true;
}
UndoRedo::CommandSetGridState::CommandSetGridState(GridSnap& gridSnap, bool state) : _gridSnap(gridSnap), newState(state) {}