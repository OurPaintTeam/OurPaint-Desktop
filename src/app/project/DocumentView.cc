#include "DocumentView.h"

using namespace app;

DocumentView::DocumentView(core::Document& document, DocumentId id)
    : document_(document),
      id_(id),
      camera2D_(),
      overlay_(),
      viewportStyle_(app::ViewportStyle::makeDefault()),
      constraintPresentation_(),
      constraintLayout_(),
      editorSession_(document_, camera2D_, overlay_, constraintLayout_, constraintPresentation_),
      viewportController_(camera2D_, overlay_, editorSession_, document_, viewportStyle_, constraintLayout_, constraintPresentation_) {}

core::Document& DocumentView::document() noexcept {
    return document_;
}

const core::Document& DocumentView::document() const noexcept {
    return document_;
}


