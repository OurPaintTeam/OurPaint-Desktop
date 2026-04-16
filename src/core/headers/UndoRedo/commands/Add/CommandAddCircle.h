#ifndef OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_CIRCLE_
#define OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_CIRCLE_

#include "CommandAddObject.h"
#include "objects/Scene_ID.h"
#include "objects/Objects.h"
#include "Scene.h"

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddCircle
    class CommandAddCircle : public CommandAddObject {
    protected:
        using CommandAddObject::CommandAddObject;

    public:
        SceneObjects::ID getCircleID() {
            if (_id == SceneObjects::ID{}) {
                return Scene::_errorID;
            }
            return _id;
        }

        std::string description() const override { return "Add circle to scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_CIRCLE_