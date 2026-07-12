#ifndef OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_ARC_
#define OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_ARC_

#include "CommandAddObject.h"
#include "objects/ID.h"
#include "objects/Objects.h"
#include "Scene.h"

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddArc
    class CommandAddArc : public CommandAddObject {
    protected:
        using CommandAddObject::CommandAddObject;

    public:
        ID getArcID() {
            if (id_ == ID{}) {
                return ID(0);
            }
            return id_;
        }

        std::string description() const override { return "Add arc to scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_ARC_