#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_ARC_
#define OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_ARC_

#include "CommandDeleteObject.h"
#include "ID.h"
#include "objects/Objects.h"
#include "Scene.h"

namespace UndoRedo {

    // Command -> CommandDeleteObject -> CommandDeleteArc
    class CommandDeleteArc : public CommandDeleteObject {
    protected:
        using CommandDeleteObject::CommandDeleteObject;

    public:
        std::string description() const override { return "Delete arc from scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_ARC_