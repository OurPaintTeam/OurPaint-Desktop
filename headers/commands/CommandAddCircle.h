#ifndef OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_CIRCLE_
#define OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_CIRCLE_

#include "CommandAddObject.h"

#include "Scene.h"
#include "ID.h"
#include "Objects.h"

// Command -> CommandAddObject -> CommandAddCircle

class CommandAddCircle : public CommandAddObject {
protected:
    using CommandAddObject::CommandAddObject;

public:
    ID getCircleID() {
        if (_id == ID{}) {
            return Scene::_errorID;
        }
        return _id;
    }

    std::string description() const override { return "Add circle to scene"; }
};

#endif // ! OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_CIRCLE_