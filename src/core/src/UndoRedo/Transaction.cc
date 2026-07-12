#include "Transaction.h"

namespace UndoRedo {

Transaction::Transaction(std::string name)
    : name_(std::move(name)) {}

bool Transaction::addCommand(std::unique_ptr<Command> command) {
    if (state_ != State::Building) {
        return false;
    }

    if (!command) {
        return false;
    }

    commands_.push_back(std::move(command));
    return true;
}

bool Transaction::commit() noexcept {
    if (state_ != State::Building) {
        return false;
    }

    if (commands_.empty()) {
        return false;
    }

    std::size_t executedCount = 0;

    for (; executedCount < commands_.size(); ++executedCount) {
        Command* command = commands_[executedCount].get();

        if (!command->execute()) {
            while (executedCount > 0) {
                --executedCount;
                commands_[executedCount]->rollback();
            }

            return false;
        }
    }

    state_ = State::Applied;
    return true;
}

bool Transaction::undo() noexcept {
    if (state_ != State::Applied) {
        return false;
    }

    const std::size_t commandCount = commands_.size();

    for (std::size_t i = commandCount; i > 0; --i) {
        const std::size_t index = i - 1;
        Command* command = commands_[index].get();

        if (!command->undo()) {
            // Restore commands that were already undone.
            for (std::size_t j = index + 1; j < commandCount; ++j) {
                commands_[j]->redo();
            }

            return false;
        }
    }

    state_ = State::Reverted;
    return true;
}

bool Transaction::redo() noexcept {
    if (state_ != State::Reverted) {
        return false;
    }

    for (std::size_t i = 0; i < commands_.size(); ++i) {
        Command* command = commands_[i].get();

        if (!command->redo()) {
            // Revert commands that were already redone.
            while (i > 0) {
                --i;
                commands_[i]->undo();
            }

            return false;
        }
    }

    state_ = State::Applied;
    return true;
}

const std::string& Transaction::label() const noexcept {
    return name_;
}

bool Transaction::isApplied() const noexcept {
    return state_ == State::Applied;
}

bool Transaction::isReverted() const noexcept {
    return state_ == State::Reverted;
}

bool Transaction::isBuilding() const noexcept {
    return state_ == State::Building;
}

}