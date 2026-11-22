#ifndef OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_SECTION_
#define OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_SECTION_

#include "CommandAddObject.h"
#include "Scene.h"
#include "objects/ID.h"
#include "objects/Objects.h"

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddSection
    class CommandAddSection : public CommandAddObject {
    protected:
        using CommandAddObject::CommandAddObject;

    public:
        ID getSectionID() {
            if (id_ == ID{}) {
                return ID(0);
            }
            return id_;
        }

        std::string description() const override { return "Add section to scene"; }
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_SECTION_