#ifndef OURPAINT_HEADERS_COMMANDS_ADD_REQUIREMENT_
#define OURPAINT_HEADERS_COMMANDS_ADD_REQUIREMENT_

#include "Command.h"
#include "Scene.h"
#include "ID.h"
#include "Objects.h"

// Command -> CommandAddRequirement

class CommandAddRequirement : public Command {
protected:
    Scene& _scene;
    RequirementData _reqData;
    ID _reqID;

    bool Execute() override;

    bool Undo() override;

    bool Redo() override;

public:
    CommandAddRequirement(Scene& scene, RequirementData data) : _scene(scene), _reqData(data) {}

    ID getReqID() const {
        return _reqID;
    }

    std::string description() const override { return "Add requirement to scene"; };
};

#endif // ! OURPAINT_HEADERS_COMMANDS_ADD_REQUIREMENT_`