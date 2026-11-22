#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_
#define OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_

#include "Command.h"
#include "Scene.h"
#include "ID.h"
#include "objects/Objects.h"

namespace UndoRedo {

    // Command -> CommandDeleteObject -> CommandDeleteX
    class CommandDeleteObject : public Command {
    protected:
        core::Scene& _scene;
        core::ID _id;
        core::ObjectData _data;
        std::vector<core::Requirement> _reqData;

        bool doExecute() noexcept override;

        bool doUndo() noexcept override;

        bool doRedo() noexcept override;

    public:
        CommandDeleteObject(core::Scene& scene, core::ID id) : _scene(scene), _id(id), _data() {}

        virtual std::string description() const override { return "Delete scene object"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_