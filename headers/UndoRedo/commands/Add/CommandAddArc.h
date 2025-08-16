#ifndef OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_ARC_
#define OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_ARC_

#include "CommandAddObject.h"

#include "Scene.h"
#include "ID.h"
#include "Objects.h"

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddArc
    class CommandAddArc : public CommandAddObject {
    protected:
        using CommandAddObject::CommandAddObject;

    public:
        ID getArcID() {
            if (_id == ID{}) {
                return Scene::_errorID;
            }
            return _id;
        }

        std::string description() const override { return "Add arc to scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_ARC_