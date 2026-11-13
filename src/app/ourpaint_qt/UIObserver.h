#ifndef OURPAINT_QT_UI_OBSERVER_H_
#define OURPAINT_QT_UI_OBSERVER_H_

#include <string>
#include "../editor/tool/ToolId.h"

namespace UI {
class ProjectManager;
}

// Presents the editor's actual tool, including changes initiated by viewport input.
class UIObserver {
public:
    static void presentActiveTool(UI::ProjectManager& window, const std::string& tabName, ToolId tool, bool active);
};

#endif  // OURPAINT_QT_UI_OBSERVER_H_
