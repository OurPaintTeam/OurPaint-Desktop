#ifndef OURPAINT_HEADERS_COMMANDS_SET_GRID_STATE_H_
#define OURPAINT_HEADERS_COMMANDS_SET_GRID_STATE_H_

#include "Command.h"
class IUndoRedoObserver;

namespace UndoRedo {

class CommandSetGridState : public Command {
protected:
    IUndoRedoObserver* _observer = nullptr;
    bool& _grid;
    bool _newState;
    bool _oldState;

    bool Execute() override;

    bool Undo() override;

public:
    CommandSetGridState(bool& grid, bool state);

    std::string description() const override { return "Grid on"; };
};

}

#endif // ! OURPAINT_HEADERS_COMMANDS_SET_GRID_STATE_H_