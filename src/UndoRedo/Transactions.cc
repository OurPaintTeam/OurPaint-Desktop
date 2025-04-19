#include "Transactions.h"

Transaction::Transaction(std::string &&name) : name(std::move(name)), commands() {}

Transaction::~Transaction() {
    for (Command *cmd: commands) {
        delete cmd;
    }
}

Transaction::Transaction(Transaction &&other)
        : commands(std::move(other.commands)),
          name(std::move(other.name)),
          committed(other.committed) {
    other.committed = false;
}

Transaction &Transaction::operator=(Transaction &&other) {
    if (this != &other) {
        for (Command *cmd: commands) {
            delete cmd;
        }
        commands = std::move(other.commands);
        name = std::move(other.name);
        committed = other.committed;
        other.committed = false;
    }
    return *this;
}

void Transaction::addCommand(Command *cmd) {
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
    try {
        for (auto &cmd: commands) {
            cmd->execute();
        }
        committed = true;
    } catch (...) {
        rollback();
        throw std::runtime_error("Transaction failed to commit");
    }
}

bool Transaction::undo() {
    if (!committed) {
        return false;
    }
    for (auto it = commands.rbegin(); it != commands.rend(); ++it) {
        (*it)->undo();
    }
    return true;
}

bool Transaction::redo() {
    if (!committed) {
        return false;
    }
    for (Command *cmd: commands) {
        cmd->redo();
    }
    return true;
}

void Transaction::rollback() {
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