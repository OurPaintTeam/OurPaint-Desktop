#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_SECTION_
#define OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_SECTION_

#include "CommandDeleteObject.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

// Command -> CommandDeleteObject -> CommandDeleteSection

class CommandDeleteSection : public CommandDeleteObject {
protected:
    using CommandDeleteObject::CommandDeleteObject;

public:
    std::string description() const override { return "Delete section from scene"; }
};

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_SECTION_