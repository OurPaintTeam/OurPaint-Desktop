#include "DocumentView.h"

DocumentView::DocumentView(Document& document, IViewportHost& host)
    :
      document_(document),
      host_(host),
      camera2D_(),
      overlay_(),
      editorSession_(document_, camera2D_, overlay_),
      viewportController_(camera2D_, overlay_, editorSession_, document_) {}

Document& DocumentView::document() noexcept {
    return document_;
}

const Document& DocumentView::document() const noexcept {
    return document_;
}


