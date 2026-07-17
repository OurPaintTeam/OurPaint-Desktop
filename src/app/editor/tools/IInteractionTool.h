#ifndef OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
#define OURPAINT_APPLICATION_IINTERACTIONTOOL_H_

#include "../../platform/InputEvents.h"

class IInteractionTool {
public:
    virtual ~IInteractionTool() = default;

    virtual void onMouseMove(const input::MouseMoveEvent& e) = 0;
    virtual void onMouseButton(const input::MouseButtonEvent& e) = 0;
    virtual void onKey(const input::KeyEvent& e) = 0;

    // true => cancel handled by tool
    // false => nothing to cancel, caller may switch tool
    virtual bool cancel() = 0;
};

#endif // ! OURPAINT_APPLICATION_IINTERACTIONTOOL_H_