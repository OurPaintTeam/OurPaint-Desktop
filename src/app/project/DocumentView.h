#ifndef OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_
#define OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_

#include "../editor/CommandConsole.h"
#include "../viewport/AxisTexts.h"
#include "../viewport/OverlayModel.h"
#include "../viewport/ViewportController.h"
#include "../viewport/ViewportStyle.h"
#include "../viewport/constraints/ConstraintLayout.h"
#include "../viewport/picking/Cpu2dPicker.h"
#include "../viewport/render/RenderSceneBuilder.h"
#include "Camera2D.h"
#include "IRenderer.h"

// Application owns each view. Document and viewport host outlive it; UIController
// disconnects synchronous editor callbacks and the viewport sink before removal.
class DocumentView {
public:
    explicit DocumentView(Document& document, IViewportHost& host);

    DocumentView(const DocumentView&) = delete;
    DocumentView& operator=(const DocumentView&) = delete;
    DocumentView(DocumentView&&) = delete;
    DocumentView& operator=(DocumentView&&) = delete;

    Document& document() noexcept;
    const Document& document() const noexcept;

public:

    Document& document_;
    IViewportHost& host_;

    Camera2D camera2D_;
    OverlayModel overlay_;
    app::ViewportStyle viewportStyle_;
    app::ConstraintLayout constraintLayout_;
    SketchEditor editorSession_;
    ViewportController viewportController_;
};


#endif // OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_
