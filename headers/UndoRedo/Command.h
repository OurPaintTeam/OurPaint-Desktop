#ifndef OURPAINT_HEADERS_UNDOREDO_COMMAND_H_
#define OURPAINT_HEADERS_UNDOREDO_COMMAND_H_

#include <string>

class Command {
public:
    enum class State { READY, EXECUTED, UNDONE };

    virtual ~Command() = default;

    bool execute() {
        if (_state != State::READY || !Execute()) {
            return false;
        }
        _state = State::EXECUTED;
        return true;
    }

    bool undo() {
        if (_state != State::EXECUTED || !Undo()) {
            return false;
        }
        _state = State::UNDONE;
        return true;
    }

    bool redo() {
        if (_state != State::UNDONE || !Redo()) {
            return false;
        }
        _state = State::EXECUTED;
        return true;
    }

    virtual std::string description() const { return "Command"; }

protected:
    virtual bool Execute() = 0;
    virtual bool Undo() = 0;
    virtual bool Redo() { return Execute(); };
    State _state = State::READY;
};

#endif // ! OURPAINT_HEADERS_UNDOREDO_COMMAND_H_