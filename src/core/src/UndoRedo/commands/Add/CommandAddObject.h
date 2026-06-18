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
        Scene& scene_;
        ObjectData data_;
        ID id_;

        bool doExecute() noexcept override;

        bool doUndo() noexcept override;

        bool doRedo() noexcept override;

    public:
        CommandAddObject(Scene& scene, ObjectData data) : scene_(scene), data_(data) {}

        virtual std::string description() const override { return "Add object to scene"; };
    };

}

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_COMMAND_ADD_OBJECT_