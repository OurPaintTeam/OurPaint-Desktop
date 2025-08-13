#ifndef OURPAINT_HEADERS_COMMANDS_SET_GRID_STATE_H_
#define OURPAINT_HEADERS_COMMANDS_SET_GRID_STATE_H_

#include "Command.h"
class GridSnap;

namespace UndoRedo {

class CommandSetGridState : public Command {
protected:
    GridSnap& _gridSnap;
    bool newState;
    bool oldState;

    bool Execute() override;

    bool Undo() override;

public:
    CommandSetGridState(GridSnap& gridSnap, bool state);

    std::string description() const override { return "Grid state"; };
};

}

#endif // ! OURPAINT_HEADERS_COMMANDS_SET_GRID_STATE_H_