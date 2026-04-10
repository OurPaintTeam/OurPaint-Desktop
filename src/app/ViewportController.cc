#include "ViewportController.h"

#include "Document.h"
#include "Scene.h"

ViewportController::ViewportController(DocumentManager& documentManager, Camera2D& camera2D, EditorSession& editorSession_, IRenderer& renderer, renderer::RenderData& renderScene)
    : documentManager_(documentManager), camera2D_(camera2D), editorSession_(editorSession_), renderer_(renderer), renderScene_(renderScene) {}

void ViewportController::initialize() {
    //auto target = host_.getRenderTargetDesc();
    //bool result = renderer_.initialize();
    //if (!result) {
    //    mLog("initialize renderer failed");
    //}

    //auto s = host_.size();
    //renderer_.resize(s.width, s.height);

    dirty_ = true;
}

bool ViewportController::onResize(const input::ResizeEvent& e) {
    camera2D_.setViewport(e.width, e.height, e.devicePixelRatio);
    renderer_.resize(e.width * e.devicePixelRatio,e.height * e.devicePixelRatio);
    return true;
}

bool ViewportController::onMouseMove(const input::MouseMoveEvent& e) {
    if ((e.buttons & input::MouseButton::Right) != input::MouseButton::None) {
        const double dx = e.x - lastX_;
        const double dy = e.y - lastY_;

        camera2D_.panScreen(dx, dy);

        lastX_ = e.x;
        lastY_ = e.y;
        return true;
    }

    lastX_ = e.x;
    lastY_ = e.y;

    editorSession_.activeTool()->onMouseMove(e);

    return true;
}

bool ViewportController::onMouseButton(const input::MouseButtonEvent& e) {
    editorSession_.activeTool()->onMouseButton(e);

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
        camera2D_.zoomAtScreen(factor, {e.x, e.y});
    }

    return true;
}

bool ViewportController::onKey(const input::KeyEvent& e) {
    if (e.key == input::KeyCode::Escape && e.action == input::KeyAction::Press) {
        bool canceled = editorSession_.activeTool()->cancel();
        if (!canceled) {
            editorSession_.select(ToolId::Cursor);
        }
        return true;
    }

    return false;
}

void ViewportController::render() {
    if (!_ini) {
        renderer_.initialize();
        _ini = true;
    }
    renderer_.render(renderScene_, camera2D_);
}

void ViewportController::requestRedraw() {
    dirty_ = true;
}

void ViewportController::setContinuousRedraw(bool enable) {
    continuousRedraw_ = enable;
}



