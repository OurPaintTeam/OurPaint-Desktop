#ifndef OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_OBJECT_
#define OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_OBJECT_

#include "Command.h"
#include "ID.h"
#include "objects/Objects.h"
#include "Scene.h"
using namespace core;

namespace UndoRedo {

    // Command -> CommandAddObject -> CommandAddX
    class CommandAddObject : public Command {
    protected:
        core::Scene &_scene;
        core::ObjectData _data;
        core::ID _id;

        bool Execute() override;

        bool Undo() override;

        bool Redo() override;

    public:
        CommandAddObject(core::Scene& scene, core::ObjectData data) : _scene(scene), _data(data) {}

        virtual std::string description() const override { return "Add object to scene"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_OBJECT_