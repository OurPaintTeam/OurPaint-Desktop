#pragma once

#include <QObject>
#include <string>

#include "../editor/IUIObserver.h"

namespace UI {
class ProjectManager;
}

// Translates synchronous App notifications to Qt signals on the GUI thread.
// viewClosing must be delivered directly while findViewport(view) is still valid.
class UIObserver final : public QObject, public IUIObserver {
    Q_OBJECT
public:
    void viewAboutToClose(ViewId view) override { emit viewClosing(view); }
    void activeViewChanged(ViewId view) override { emit activeChanged(view); }
    void toolChanged(ViewId view, ToolId tool) override { emit selectedToolChanged(view, tool); }
    void redrawRequested(ViewId view) override { emit redraw(view); }

    static void presentActiveTool(UI::ProjectManager& window, const std::string& tabName, ToolId tool, bool active);

signals:
    void viewClosing(ViewId view);
    void activeChanged(ViewId view);
    void selectedToolChanged(ViewId view, ToolId tool);
    void redraw(ViewId view);
};
