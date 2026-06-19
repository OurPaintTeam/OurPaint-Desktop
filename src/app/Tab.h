#ifndef OURPAINT_APPLICATION_TAB_H_
#define OURPAINT_APPLICATION_TAB_H_

#include "App/CustomConsole.h"
#include "Camera2D.h"
#include "Cpu2dPicker.h"
#include "IRenderer.h"
#include "InteractionTools/IInteractionTool.h"
#include "OverlayModel.h"

class ViewportController;
class RenderDataBuilder;
class CommandConsole;
class AxisTexts;
namespace renderer { class RenderData; }

struct Tab {
    std::string name_;

    IRenderer* renderer_;
    renderer::RenderData* renderData_;
    Camera2D* camera2D_;

    EditorSession* editorSession_;
    IInteractionTool* interactionTool_;
    Cpu2dPicker* picker_;
    OverlayModel* overlay_;
    CommandConsole* commandConsole_;
    AxisTexts* axisTexts_;
    RenderDataBuilder* builder_;

    ViewportController* viewportController_;
};


#endif // OURPAINT_APPLICATION_TAB_H_