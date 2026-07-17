#ifndef OURPAINT_HEADERS_COMMANDS_ADD_REQUIREMENT_
#define OURPAINT_HEADERS_COMMANDS_ADD_REQUIREMENT_

#include "Command.h"
#include "Scene.h"
#include "objects/ID.h"
#include "objects/Objects.h"

namespace UndoRedo {

    // Command -> CommandAddRequirement
    class CommandAddRequirement : public Command {
    protected:
        core::Scene &_scene;
        core::Requirement _reqData;
        core::ID _reqID;

        bool doExecute() noexcept override;

        bool doUndo() noexcept override;

        bool doRedo() noexcept override;

    public:
        CommandAddRequirement(core::Scene &scene, core::Requirement data) : _scene(scene), _reqData(data) {}

        core::ID getReqID() const {
            return _reqID;
        }

        std::string description() const override { return "Add requirement to scene"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_REQUIREMENT_