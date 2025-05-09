#ifndef OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_OBJECT_
#define OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_OBJECT_

#include "Command.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddX
    class CommandAddObject : public Command {
    protected:
        Scene &_scene;
        ObjectData _data;
        ID _id;

        bool Execute() override;

        bool Undo() override;

        bool Redo() override;

    public:
        CommandAddObject(Scene &scene, ObjectData data) : _scene(scene), _data(data) {}

        virtual std::string description() const override { return "Add object to scene"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_OBJECT_