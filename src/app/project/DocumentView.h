#ifndef OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_
#define OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_

#include "OverlayModel.h"
#include "ViewportController.h"
#include "ViewportStyle.h"
#include "ConstraintLayout.h"
#include "ConstraintPresentationState.h"
#include "RenderSceneBuilder.h"
#include "Camera2D.h"
#include "IRenderer.h"

// Application owns each view. Document and viewport host outlive it; UIController
// disconnects synchronous editor callbacks and the viewport sink before removal.
class DocumentView {
public:
    explicit DocumentView(Document& document);

    DocumentView(const DocumentView&) = delete;
    DocumentView& operator=(const DocumentView&) = delete;
    DocumentView(DocumentView&&) = delete;
    DocumentView& operator=(DocumentView&&) = delete;

    Document& document() noexcept;
    const Document& document() const noexcept;

public:

    Document& document_;

    Camera2D camera2D_;
    OverlayModel overlay_;
    app::ViewportStyle viewportStyle_;
    app::ConstraintPresentationState constraintPresentation_;
    app::ConstraintLayout constraintLayout_;
    app::SketchEditor editorSession_;
    ViewportController viewportController_;
};


#endif // OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_
