#ifndef OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_SECTION_
#define OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_SECTION_

#include "CommandAddObject.h"

#include "Scene.h"
#include "ID.h"
#include "Objects.h"

// Command -> CommandAddObject -> CommandAddSection

class CommandAddSection : public CommandAddObject {
protected:
    using CommandAddObject::CommandAddObject;

public:
    ID getSectionID() {
        if (_id == ID{}) {
            return Scene::_errorID;
        }
        return _id;
    }

    std::string description() const override { return "Add section to scene"; }
};

#endif // ! OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_SECTION_