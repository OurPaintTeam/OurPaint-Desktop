#ifndef OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_CUBIC_BEZIER_
#define OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_CUBIC_BEZIER_

#include "CommandAddObject.h"
#include "Scene.h"
#include "objects/ID.h"
#include "objects/Objects.h"

namespace UndoRedo {

// Command -> CommandAddObject -> CommandAddCubicBezier
class CommandAddCubicBezier : public CommandAddObject {
protected:
    using CommandAddObject::CommandAddObject;

public:
    ID getSectionID() {
        if (_id == ID{}) {
            return ID(0);
        }
        return _id;
    }

    std::string description() const override { return "Add cubic bezier to scene"; }
};

}

#endif // ! OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_CUBIC_BEZIER_