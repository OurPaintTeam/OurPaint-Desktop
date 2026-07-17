#include "ViewportController.h"

#include "Document.h"
#include "Scene.h"
#include "UndoRedo.h"

ViewportController::ViewportController(Camera2D& camera,
                                       OverlayModel& overlay,
                                       EditorSession& editorSession_,
                                       Document& document)
    :   camera2D_(camera),
        overlay_(overlay),
        editorSession_(editorSession_),
        document_(document),
        axisTexts_(camera2D_),
        viewportStyle_(app::ViewportStyle::makeDefault()),
        builder_(document_.scene(), overlay_, axisTexts_, viewportStyle_, renderScene_),
        renderScene_(),
        renderer_() {}

bool ViewportController::onResize(const input::ResizeEvent& e) {
    camera2D_.setViewport(e.width, e.height, e.devicePixelRatio);
    renderer_.resize(e.width * e.devicePixelRatio,e.height * e.devicePixelRatio);
    return true;
}

bool ViewportController::onMouseMove(const input::MouseMoveEvent& e) {
    if (input::has_flag(e.buttons, input::MouseButton::Right)) {
        const double dx = e.x - lastX_;
        const double dy = e.y - lastY_;

        camera2D_.panScreenLogical(dx, dy);
    }

    lastX_ = e.x;
    lastY_ = e.y;

    editorSession_.activeTool()->onMouseMove(e);

    builder_.rebuild();

    requestRedraw();
    return true;
}

bool ViewportController::onMouseButton(const input::MouseButtonEvent& e) {
    editorSession_.activeTool()->onMouseButton(e);

    builder_.rebuild();

    requestRedraw();
    return true;
}

bool ViewportController::onWheel(const input::WheelEvent& e) {
    double steps = 0.0;

    if (e.isPixelPrecise) {
        steps = e.deltaY / 100.0;
    } else {
        steps = e.deltaY;
    }

    if (steps != 0.0) {
        constexpr double base = 1.2;
        const double factor = std::pow(base, steps);
        camera2D_.zoomAtScreenLogical(factor, {e.x, e.y});
    }

    requestRedraw();
    return true;
}

bool ViewportController::onKey(const input::KeyEvent& e) {
    // escape
    if (e.key == input::KeyCode::Escape && e.action == input::KeyAction::Press) {
        editorSession_.select(ToolId::Cursor);
        return true;
    }

    UndoRedo::UndoRedoManager& mgr = document_.undoRedoManager();
    // redo
    if (e.key == input::KeyCode::Z &&
             input::has_flag(e.modifiers, input::Modifiers::Ctrl) &&
             input::has_flag(e.modifiers, input::Modifiers::Shift) &&
             e.action == input::KeyAction::Press) {
        mgr.redo();
    }
    // undo
    else if (e.key == input::KeyCode::Z &&
        input::has_flag(e.modifiers, input::Modifiers::Ctrl) &&
        e.action == input::KeyAction::Press) {
        mgr.undo();
    }

    editorSession_.activeTool()->onKey(e);

    requestRedraw();
    return true;
}

void ViewportController::render() {
    if (!_ini) {
        renderer_.initialize();
        _ini = true;
    }
    builder_.rebuild();
    renderer_.render(renderScene_, camera2D_);
}

void ViewportController::requestRedraw() {
    dirty_ = true;
}

void ViewportController::setContinuousRedraw(bool enable) {
    continuousRedraw_ = enable;
}



