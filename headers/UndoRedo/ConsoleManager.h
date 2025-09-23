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
    void registerFactory(ICommandFactory* f);

    UndoRedo::Transaction* invoke(const std::string &raw);

    UndoRedo::Transaction* invoke(std::string id, std::vector<double> args);

private:
    std::unordered_map<std::string, ICommandFactory*> _factories;

    ParsedInvocation parse(const std::string& raw) const;
};

#endif // ! OURPAINT_HEADERS_UNDOREDO_COMMAND_MANAGER_H_