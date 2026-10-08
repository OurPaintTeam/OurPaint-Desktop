#include "ViewportController.h"
#include "../../core/Document.h"
#include "constraints/ConstraintLayout.h"

ViewportController::ViewportController(Camera2D& camera,
                                       OverlayModel& overlay,
                                       SketchEditor& editorSession_,
                                       Document& document,
                                       const app::ViewportStyle& style,
                                       app::ConstraintLayout& constraintLayout)
    :   camera2D_(camera),
        overlay_(overlay),
        editorSession_(editorSession_),
        document_(document),
        constraintLayout_(constraintLayout),
        axisTexts_(camera2D_),
        viewportStyle_(style),
        renderScene_(),
        builder_(document_, overlay_, axisTexts_, viewportStyle_, renderScene_, camera, constraintLayout_),
        renderer_() {}

bool ViewportController::onResize(const input::ResizeEvent& e) {
    camera2D_.setViewport(e.width, e.height, e.devicePixelRatio);
    renderer_.resize(camera2D_.wFramebuffer(), camera2D_.hFramebuffer());
    updateConstraintLayout();
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

    updateConstraintLayout();
    editorSession_.activeTool()->onMouseMove(e);

    requestRedraw();
    return true;
}

bool ViewportController::onMouseButton(const input::MouseButtonEvent& e) {
    updateConstraintLayout();
    editorSession_.activeTool()->onMouseButton(e);

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

    updateConstraintLayout();
    requestRedraw();
    return true;
}

bool ViewportController::onKey(const input::KeyEvent& e) {
    //UndoRedo::UndoRedoManager& mgr = document_.undoRedoManager();
    // redo
    if (e.key == input::KeyCode::Z &&
             input::has_flag(e.modifiers, input::Modifiers::Ctrl) &&
             input::has_flag(e.modifiers, input::Modifiers::Shift) &&
             e.action == input::KeyAction::Press) {
        //mgr.redo();
    }
    // undo
    else if (e.key == input::KeyCode::Z &&
        input::has_flag(e.modifiers, input::Modifiers::Ctrl) &&
        e.action == input::KeyAction::Press) {
        //mgr.undo();
    }

    updateConstraintLayout();
    editorSession_.onKey(e);

    requestRedraw();
    return true;
}

void ViewportController::render() {
    if (!_ini) {
        renderer_.initialize();
        _ini = true;
    }
    updateConstraintLayout();
    builder_.rebuild();
    renderer_.render(renderScene_, camera2D_);
}

void ViewportController::updateConstraintLayout() {
    constraintLayout_.rebuild(document_.sketch(), camera2D_, viewportStyle_.constraintMarker);
}

void ViewportController::requestRedraw() {
    dirty_ = true;
}

void ViewportController::setContinuousRedraw(bool enable) {
    continuousRedraw_ = enable;
}



