#ifndef OURPAINT_HEADERS_UNDOREDO_COMMAND_H_
#define OURPAINT_HEADERS_UNDOREDO_COMMAND_H_

#include <string>

namespace UndoRedo {

class Command {
public:
    enum class State {
        Ready,
        Applied,
        Reverted
    };

    Command() = default;
    virtual ~Command() = default;

    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    Command(Command&&) = delete;
    Command& operator=(Command&&) = delete;

    [[nodiscard]] bool execute() noexcept {
        if (state_ != State::Ready) {
            return false;
        }

        if (!doExecute()) {
            return false;
        }

        state_ = State::Applied;
        return true;
    }

    [[nodiscard]] bool undo() noexcept {
        if (state_ != State::Applied) {
            return false;
        }

        if (!doUndo()) {
            return false;
        }

        state_ = State::Reverted;
        return true;
    }

    [[nodiscard]] bool redo() noexcept {
        if (state_ != State::Reverted) {
            return false;
        }

        if (!doRedo()) {
            return false;
        }

        state_ = State::Applied;
        return true;
    }

    // Rolls back a command that was executed during a failed transaction commit.
    // Unlike undo(), rollback() returns the command to the Ready state.
    [[nodiscard]] bool rollback() noexcept {
        if (state_ != State::Applied) {
            return false;
        }

        if (!doUndo()) {
            return false;
        }

        state_ = State::Ready;
        return true;
    }

    [[nodiscard]] State state() const noexcept {
        return state_;
    }

    [[nodiscard]] bool isReady() const noexcept {
        return state_ == State::Ready;
    }

    [[nodiscard]] bool isApplied() const noexcept {
        return state_ == State::Applied;
    }

    [[nodiscard]] bool isReverted() const noexcept {
        return state_ == State::Reverted;
    }

    [[nodiscard]] virtual std::string description() const {
        return "Command";
    }

protected:
    [[nodiscard]] virtual bool doExecute() noexcept = 0;
    [[nodiscard]] virtual bool doUndo() noexcept = 0;
    [[nodiscard]] virtual bool doRedo() noexcept = 0;

private:
    State state_ = State::Ready;
};

}

#endif // ! OURPAINT_HEADERS_UNDOREDO_COMMAND_H_