#include "Transaction.h"

Transaction::Transaction(std::string&& name) : name(std::move(name)), commands() {}

Transaction::~Transaction() {
    for (Command* cmd: commands) {
        delete cmd;
    }
}

Transaction::Transaction(Transaction&& other)
        : commands(std::move(other.commands)),
          name(std::move(other.name)),
          committed(other.committed) {
    other.committed = false;
}

Transaction& Transaction::operator=(Transaction&& other) {
    if (this != &other) {
        for (Command* cmd: commands) {
            delete cmd;
        }
        commands = std::move(other.commands);
        other.commands.clear();
        name = std::move(other.name);
        committed = other.committed;
    }
    return *this;
}

void Transaction::addCommand(Command* cmd) {
    if (!cmd) {
        throw std::invalid_argument("Command cannot be null");
    }
    if (committed) {
        throw std::logic_error("Transaction already committed");
    }
    commands.push_back(cmd);
}

void Transaction::commit() {
    if (committed) {
        throw std::logic_error("Transaction already committed");
    }
    std::vector<Command *> executed;
    for (auto& cmd: commands) {
        if (!cmd->execute()) {
            for (auto it = executed.rbegin(); it != executed.rend(); ++it) {
                (*it)->undo();
            }
            throw std::runtime_error("Transaction failed to commit");
        }
        executed.push_back(cmd);
    }
    committed = true;
}

bool Transaction::undo() {
    if (!committed) {
        return false;
    }
    std::vector<Command*> undone;
    for (auto it = commands.rbegin(); it != commands.rend(); ++it) {
        if (!(*it)->undo()) {
            for (auto rit = undone.rbegin(); rit != undone.rend(); ++rit) {
                (*rit)->redo();
            }
            return false;
        }
        undone.push_back(*it);
    }
    return true;
}

bool Transaction::redo() {
    if (!committed) {
        return false;
    }
    std::vector<Command*> redone;
    for (Command* cmd: commands) {
        if (!cmd->redo()) {
            for (auto it = redone.rbegin(); it != redone.rend(); ++it) {
                (*it)->undo();
            }
            return false;
        }
        redone.push_back(cmd);
    }
    return true;
}

void Transaction::rollback() noexcept {
    for (auto it = commands.rbegin(); it != commands.rend(); ++it) {
        (*it)->undo();
    }
}

std::string Transaction::label() const {
    return name;
}

bool Transaction::isCommitted() const {
    return committed;
}