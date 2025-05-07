#ifndef OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_
#define OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_

#include "Command.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

// Command -> CommandDeleteObject -> CommandDeleteX

class CommandDeleteObject : public Command {
protected:
    Scene& _scene;
    ID _id;
    ObjectData _data;

    bool Execute() override;

    bool Undo() override;

    bool Redo() override;

public:
    CommandDeleteObject(Scene& scene, ID id) : _scene(scene), _id(id) {}

    std::string description() const override { return "Delete scene object"; };
};

#endif // ! OURPAINT_HEADERS_COMMANDS_DELETE_COMMAND_DELETE_OBJECT_