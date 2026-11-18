#ifndef OURPAINT_APPLICATION_EDITOR_I_UI_OBSERVER_H_
#define OURPAINT_APPLICATION_EDITOR_I_UI_OBSERVER_H_

#include <cstdint>

#include "tool/ToolId.h"

namespace app {
// Strongly typed so a stable document ID cannot accidentally address a view.
enum class ViewId : std::uint64_t {};
inline constexpr ViewId InvalidViewId{0};

// Optional synchronous notifications from App. During a notification the client
// may query the controller, but must not re-enter a workspace mutation.
class IUIObserver {
public:
    virtual ~IUIObserver() = default;
    // Last chance to detach a borrowed viewport, before its document is released.
    virtual void viewAboutToClose(ViewId view) = 0;
    virtual void activeViewChanged(ViewId view) = 0;
    virtual void toolChanged(ViewId view, ToolId tool) = 0;
    virtual void redrawRequested(ViewId view) = 0;
};
}

#endif // ! OURPAINT_APPLICATION_EDITOR_I_UI_OBSERVER_H_