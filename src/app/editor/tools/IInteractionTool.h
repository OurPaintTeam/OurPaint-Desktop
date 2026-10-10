#ifndef OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
#define OURPAINT_APPLICATION_IINTERACTIONTOOL_H_

#include "../../platform/InputEvents.h"
#include "../ActionReport.h"

class IInteractionTool {
public:
    virtual ~IInteractionTool() = default;

    virtual std::optional<ActionReport> onMouseMove(const input::MouseMoveEvent& e) = 0;
    virtual std::optional<ActionReport> onMouseButton(const input::MouseButtonEvent& e) = 0;
    virtual std::optional<ActionReport> onKey(const input::KeyEvent& e) = 0;

    // Pending input is discarded, not rolled back. A gesture may return its
    // outcome here; handled=false allows the editor to exit an empty tool.
    virtual ToolCancellation cancel() = 0;
};

#endif // ! OURPAINT_APPLICATION_IINTERACTIONTOOL_H_
