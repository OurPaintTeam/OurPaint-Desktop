#ifndef OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_OBJECT_
#define OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_OBJECT_

#include "Command.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

// Command -> CommandAddObject -> CommandAddX

class CommandAddObject : public Command {
protected:
    Scene& _scene;
    ObjectData _data;
    ID _id;

    bool Execute() override;

    bool Undo() override;

    bool Redo() override;

public:
    CommandAddObject(Scene& scene, ObjectData data) : _scene(scene), _data(data) {}

    std::string description() const { return "Add object to scene"; };
};

#endif // ! OURPAINT_HEADERS_COMMANDS_COMMAND_ADD_OBJECT_