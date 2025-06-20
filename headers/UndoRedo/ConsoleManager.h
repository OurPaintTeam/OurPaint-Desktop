#ifndef OURPAINT_HEADERS_UNDOREDO_COMMAND_MANAGER_H_
#define OURPAINT_HEADERS_UNDOREDO_COMMAND_MANAGER_H_

#include <algorithm>   // transform
#include <cctype>      // std::toupper
#include <sstream>     // istringstream
#include <string>
#include <unordered_map>
#include <vector>
#include "Transaction.h"
#include "CommandFactory.h"
#include "Scene.h"

using namespace UndoRedo;

class CommandManager {
    struct ParsedInvocation {
        std::string id{};
        std::vector<std::string> args{};
    };

public:
    void registerFactory(ICommandFactory* f) {
        _factories[f->id()] = std::move(f);
    }

    UndoRedo::Transaction* invoke(const std::string &raw) {
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

    UndoRedo::Transaction* invoke(std::string id, std::vector<double> args) {
        auto it = _factories.find(id);
        if (it == _factories.end()) {
            throw std::runtime_error("Unknown command: " + id);
        }

        Transaction* txn = new Transaction(std::move(id));
        it->second->createCommands(args, *txn);
        txn->commit();
        return txn;
    }

private:
    std::unordered_map<std::string, ICommandFactory*> _factories;

    ParsedInvocation parse(const std::string& raw) const {
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
    };
};

#endif // ! OURPAINT_HEADERS_UNDOREDO_COMMAND_MANAGER_H_