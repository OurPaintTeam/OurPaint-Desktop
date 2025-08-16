#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_POINT_
#define OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_POINT_

#include "CommandDeleteObject.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

namespace UndoRedo {

    // Command -> CommandDeleteObject -> CommandDeletePoint
    class CommandDeletePoint : public CommandDeleteObject {
    protected:
        using CommandDeleteObject::CommandDeleteObject;

    public:
        std::string description() const override { return "Delete point from Scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_POINT_