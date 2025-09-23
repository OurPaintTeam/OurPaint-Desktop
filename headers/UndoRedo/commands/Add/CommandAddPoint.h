#ifndef OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_POINT_
#define OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_POINT_

#include "CommandAddObject.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddPoint
    class CommandAddPoint : public CommandAddObject {
    protected:
        using CommandAddObject::CommandAddObject;

    public:
        ID getPointID() {
            if (_id == ID{}) {
                return Scene::_errorID;
            }
            return _id;
        }

        std::string description() const override { return "Add point to Scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_POINT_