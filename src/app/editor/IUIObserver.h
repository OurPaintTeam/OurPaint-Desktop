#pragma once

#include <cstdint>

#include "tool/ToolId.h"

// Strongly typed so a stable document ID cannot accidentally address a view.
enum class ViewId : std::uint64_t {};
inline constexpr ViewId InvalidViewId{0};

// Optional synchronous notifications from App. During a notification the client
// may query the controller, but must not re-enter a workspace mutation.
class IUIObserver {
public:
    virtual ~IUIObserver() = default;
    // Last chance to detach a borrowed viewport, including on project replacement.
    virtual void viewAboutToClose(ViewId view) = 0;
    virtual void activeViewChanged(ViewId view) = 0;
    virtual void toolChanged(ViewId view, ToolId tool) = 0;
    virtual void redrawRequested(ViewId view) = 0;
};
