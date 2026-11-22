#ifndef OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_POINT_
#define OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_POINT_

#include "CommandAddObject.h"
#include "objects/ID.h"
#include "objects/Objects.h"
#include "Scene.h"

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddPoint
    class CommandAddPoint : public CommandAddObject {
    protected:
        using CommandAddObject::CommandAddObject;

    public:
        ID getPointID() {
            if (id_ == ID{}) {
                return ID(-1);
            }
            return id_;
        }

        std::string description() const override { return "Add point to Scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_POINT_