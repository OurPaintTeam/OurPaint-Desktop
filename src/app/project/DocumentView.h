#ifndef OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_
#define OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_

#include "Camera2D.h"
#include "OverlayModel.h"
#include "ViewportController.h"
#include "render/AxisTexts.h"
#include "render/ViewportStyle.h"
#include "render/constraints/ConstraintLayout.h"
#include "render/constraints/ConstraintPresentationState.h"
#include "pick/Cpu2dPicker.h"
#include "render/RenderSceneBuilder.h"
#include "IRenderer.h"

// UIController owns the view and its document. The host borrows only the viewport
// controller and detaches while the view is still alive.
// Explicit GPU shutdown requires this view's OpenGL context to still be current.
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
    DocumentId id;

    render::Camera2D camera2D_;
    OverlayModel overlay_;
    app::ViewportStyle viewportStyle_;
    app::ConstraintPresentationState constraintPresentation_;
    app::ConstraintLayout constraintLayout_;
    app::SketchEditor editorSession_;
    ViewportController viewportController_;
};


#endif // OURPAINT_APPLICATION_DOCUMENT_WORKSPACE_H_
