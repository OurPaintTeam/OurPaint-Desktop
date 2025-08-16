#ifndef OURPAINT_HEADERS_COMMANDS_CLEAR_H_
#define OURPAINT_HEADERS_COMMANDS_CLEAR_H_

#include "Command.h"
#include "Scene.h"

namespace UndoRedo {

// Command -> CommandClear
class CommandClear : public Command {
protected:
    Scene*& _scene;
    Scene* _sceneBefore{};
    Scene* _sceneAfter{};

    bool Execute() override;

    bool Undo() override;

    bool Redo() override;

public:
    CommandClear(Scene*& scene);
    ~CommandClear();

    std::string description() const override { return "Clear scene"; };
};

}  // namespace UndoRedo

#endif  // ! OURPAINT_HEADERS_COMMANDS_CLEAR_H_