#ifndef OURPAINT_HEADERS_COMMANDS_MOVE_
#define OURPAINT_HEADERS_COMMANDS_MOVE_

#include "Command.h"
#include "objects/Objects.h"
namespace core {class Scene;}

namespace UndoRedo {

    // Command -> CommandMove
    class CommandMove : public Command {
    protected:
        core::Scene& _scene;
        std::vector<core::ObjectData> _pre_move_object_states;
        std::vector<core::ObjectData> _post_move_object_states;

        bool doExecute() noexcept override;

        bool doUndo() noexcept override;

        bool doRedo() noexcept override;

    public:
        CommandMove(core::Scene& scene, std::vector<core::ObjectData> pre_move_object_states) : _scene(scene), _pre_move_object_states(pre_move_object_states) {}

        std::string description() const override { return "Move objects"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_MOVE_