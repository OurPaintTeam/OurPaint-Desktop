#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_REQUIREMENT_
#define OURPAINT_HEADERS_COMMANDS_DELETE_REQUIREMENT_

#include "Command.h"
#include "Scene.h"
#include "ID.h"
#include "objects/Objects.h"

namespace UndoRedo {

    // Command -> CommandAddRequirement
    class CommandDeleteRequirement : public Command {
    protected:
        core::Scene &_scene;
        core::Requirement _reqData;
        core::ID _reqID;

        bool Execute() override;

        bool Undo() override;

        bool Redo() override;

    public:
        CommandDeleteRequirement(core::Scene &scene, core::ID reqID) : _scene(scene), _reqID(reqID) {}

        std::string description() const override { return "Delete requirement to scene"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_REQUIREMENT_`