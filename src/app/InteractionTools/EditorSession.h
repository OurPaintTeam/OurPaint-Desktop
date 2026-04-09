#ifndef OURPAINT_APPLICATION_EDITORSESSION_H_
#define OURPAINT_APPLICATION_EDITORSESSION_H_

#include "DocumentManager.h"
#include "IInteractionTool.h"
#include "ToolId.h"
#include "Camera2D.h"
#include "RenderData.h"

class EditorSession {
public:
    EditorSession(DocumentManager& manager, Camera2D& camera, renderer::RenderData& renderData);
    ~EditorSession();

    void select(ToolId id);
    IInteractionTool* activeTool();

private:
    IInteractionTool* activeTool_ = nullptr;
    DocumentManager& documentManager_;
    Camera2D& camera_;
    renderer::RenderData& renderData_;
};

#endif // ! OURPAINT_APPLICATION_EDITORSESSION_H_

