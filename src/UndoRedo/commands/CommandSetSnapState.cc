#include "CommandSetSnapState.h"
#include "GridSnap.h"

bool UndoRedo::CommandSetSnapState::Execute() {
    _oldSnapGridState = _gridSnap.getSnapGridState();
    _oldSnapObjectsState = _gridSnap.getSnapObjectsState();
    _gridSnap.setSnapGridState(_newSnapGridState);
    _gridSnap.setSnapObjectsState(_newSnapObjectsState);
    return true;
}

bool UndoRedo::CommandSetSnapState::Undo() {
    _gridSnap.setSnapGridState(_oldSnapGridState);
    _gridSnap.setSnapObjectsState(_oldSnapObjectsState);
    return true;
}
UndoRedo::CommandSetSnapState::CommandSetSnapState(GridSnap& gridSnap, bool snapGrid, bool snapObjects)
    : _gridSnap(gridSnap), _newSnapGridState(snapGrid), _newSnapObjectsState(snapObjects) {}