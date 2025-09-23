#ifndef OURPAINT_HEADERS_COMMANDS_MOVE_
#define OURPAINT_HEADERS_COMMANDS_MOVE_

#include "Command.h"
#include "Objects.h"
class Scene;

namespace UndoRedo {

    // Command -> CommandMove
    class CommandMove : public Command {
    protected:
        Scene& _scene;
        std::vector<ObjectData> _pre_move_object_states;
        std::vector<ObjectData> _post_move_object_states;

        bool Execute() override;

        bool Undo() override;

        bool Redo() override;

    public:
        CommandMove(Scene& scene, std::vector<ObjectData> pre_move_object_states) : _scene(scene), _pre_move_object_states(pre_move_object_states) {}

        std::string description() const override { return "Move objects"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_MOVE_