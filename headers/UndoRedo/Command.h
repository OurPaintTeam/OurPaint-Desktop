#ifndef OURPAINT_HEADERS_UNDOREDO_COMMAND_H_
#define OURPAINT_HEADERS_UNDOREDO_COMMAND_H_

#include <string>

class Command {
public:
    virtual ~Command() = default;

    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;

    virtual std::string description() const { return "Command"; }
};

#endif // ! OURPAINT_HEADERS_UNDOREDO_COMMAND_H_