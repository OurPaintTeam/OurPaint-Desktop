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
        core::Scene& scene_;
        core::Requirement reqData_;
        core::ID reqID_;

        bool doExecute() noexcept override;

        bool doUndo() noexcept override;

        bool doRedo() noexcept override;

    public:
        CommandDeleteRequirement(core::Scene &scene, core::ID reqID) : scene_(scene), reqID_(reqID) {}

        std::string description() const override { return "Delete requirement to scene"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_REQUIREMENT_`