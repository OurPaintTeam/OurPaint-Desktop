#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_CIRCLE_
#define OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_CIRCLE_

#include "CommandDeleteObject.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

// Command -> CommandDeleteObject -> CommandDeleteCircle

class CommandDeleteCircle : public CommandDeleteObject {
protected:
    using CommandDeleteObject::CommandDeleteObject;

public:

    std::string description() const override { return "Delete circle from scene"; }
};

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_CIRCLE_