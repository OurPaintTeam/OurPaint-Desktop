#include "ConsoleManager.h"

void CommandManager::registerFactory(ICommandFactory* f) {
    _factories[f->id()] = std::move(f);
}

UndoRedo::Transaction* CommandManager::invoke(const std::string& raw) {
    ParsedInvocation inv = parse(raw);

    auto it = _factories.find(inv.id);
    if (it == _factories.end()) {
        throw std::runtime_error("Unknown command: " + inv.id);
    }

    Transaction* txn = new Transaction(std::move(inv.id));
    it->second->createCommands(inv.args, *txn);
    txn->commit();
    return txn;
}

UndoRedo::Transaction* CommandManager::invoke(std::string id, std::vector<double> args) {
    auto it = _factories.find(id);
    if (it == _factories.end()) {
        throw std::runtime_error("Unknown command: " + id);
    }

    Transaction* txn = new Transaction(std::move(id));
    it->second->createCommands(args, *txn);
    txn->commit();
    return txn;
}

CommandManager::ParsedInvocation CommandManager::parse(const std::string& raw) const {
    ParsedInvocation out;

    std::istringstream iss(raw);
    std::string token;

    if (!(iss >> token)) {
        return out; // only spaces
    }

    std::transform(token.begin(), token.end(), token.begin(),
                   [](unsigned char c) {
                       return static_cast<char>(std::toupper(c));
                   });
    out.id = std::move(token);

    while (iss >> token) {
        out.args.emplace_back(std::move(token));
    }

    return out;
}


