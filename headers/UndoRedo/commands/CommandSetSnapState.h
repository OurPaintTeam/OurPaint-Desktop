#ifndef OURPAINT_HEADERS_COMMANDS_SET_SNAP_STATE_H_
#define OURPAINT_HEADERS_COMMANDS_SET_SNAP_STATE_H_

#include "Command.h"
class GridSnap;

namespace UndoRedo {

class CommandSetSnapState : public Command {
protected:
    GridSnap& _gridSnap;
    bool _newSnapGridState;
    bool _oldSnapGridState;
    bool _newSnapObjectsState;
    bool _oldSnapObjectsState;

    bool Execute() override;

    bool Undo() override;

public:
    CommandSetSnapState(GridSnap& gridSnap, bool snapGrid, bool snapObjects);

    std::string description() const override { return "Snap state"; };
};

}

#endif // ! OURPAINT_HEADERS_COMMANDS_SET_SNAP_STATE_H_