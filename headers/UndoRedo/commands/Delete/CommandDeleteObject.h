#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_
#define OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_

#include "Command.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

namespace UndoRedo {

    // Command -> CommandDeleteObject -> CommandDeleteX
    class CommandDeleteObject : public Command {
    protected:
        Scene &_scene;
        ID _id;
        ObjectData _data;
        std::vector<RequirementData> _reqData;

        bool Execute() override;

        bool Undo() override;

    public:
        CommandDeleteObject(Scene &scene, ID id) : _scene(scene), _id(id) {}

        virtual std::string description() const override { return "Delete scene object"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_