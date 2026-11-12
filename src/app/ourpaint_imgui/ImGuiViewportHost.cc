#include "ImGuiViewportHost.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

#include "../editor/IViewportController.h"
#include "imgui.h"

namespace {

// Restore the caller's actual framebuffer and viewport, including on early exit.
// The CAD renderer establishes its drawing state; ImGui establishes its own UI state.
class ScopedRenderTarget {
public:
    explicit ScopedRenderTarget(const render::OpenGLRenderTarget& target) {
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer_);
        glGetIntegerv(GL_VIEWPORT, viewport_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.framebuffer());
        glViewport(0, 0, target.width(), target.height());
    }
    ~ScopedRenderTarget() {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(framebuffer_));
        glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]);
    }
    ScopedRenderTarget(const ScopedRenderTarget&) = delete;
    ScopedRenderTarget& operator=(const ScopedRenderTarget&) = delete;

private:
    GLint framebuffer_ = 0;
    GLint viewport_[4]{};
};

input::Modifiers modifiers(const ImGuiIO& io) {
    auto result = input::Modifiers::None;
    if (io.KeyShift) {
        result = result | input::Modifiers::Shift;
    }
    if (io.KeyCtrl) {
        result = result | input::Modifiers::Ctrl;
    }
    if (io.KeyAlt) {
        result = result | input::Modifiers::Alt;
    }
    if (io.KeySuper) {
        result = result | input::Modifiers::Meta;
    }
    return result;
}

}  // namespace

ImGuiViewportHost::ImGuiViewportHost(uint32_t id) : id_(id) {}
uint32_t ImGuiViewportHost::id() const { return id_; }
Viewport2D ImGuiViewportHost::getSize() const { return size_; }
float ImGuiViewportHost::getDevicePixelRatio() const { return static_cast<float>(size_.devicePixelRatio); }

void ImGuiViewportHost::requestRedraw() {
    if (controller_) {
        controller_->dirty_ = true;
    }
}
void ImGuiViewportHost::setContinuousRedraw(bool enabled) {
    if (controller_) {
        controller_->continuousRedraw_ = enabled;
    }
}
int ImGuiViewportHost::setEventSink(IViewportController* sink) {
    if (!sink) {
        controller_ = nullptr;
        capturedButtons_ = input::MouseButton::None;
        return -1;
    }
    const int index = addController(sink);
    setActiveController(index);
    return index;
}
void ImGuiViewportHost::presentFrame() {
    // The target is a texture, not an OS window. main.cc presents the complete UI once.
}
int ImGuiViewportHost::addController(IViewportController* controller) {
    if (!controller) {
        return -1;
    }
    const auto found = std::find(controllers_.begin(), controllers_.end(), controller);
    if (found != controllers_.end()) {
        return static_cast<int>(found - controllers_.begin());
    }
    controllers_.push_back(controller);
    return static_cast<int>(controllers_.size() - 1);
}
void ImGuiViewportHost::setActiveController(int index) {
    if (index < 0 || index >= static_cast<int>(controllers_.size())) {
        return;
    }
    controller_ = controllers_[index];
    capturedButtons_ = input::MouseButton::None;
    wasHovered_ = false;
    needsResize_ = true;
    requestRedraw();
}
bool ImGuiViewportHost::removeController(IViewportController* controller) {
    const auto found = std::find(controllers_.begin(), controllers_.end(), controller);
    if (found == controllers_.end()) {
        return false;
    }
    const bool active = controller_ == controller;
    controllers_.erase(found);
    if (active) {
        setEventSink(nullptr);
        if (!controllers_.empty()) {
            setActiveController(0);
        }
    }
    return true;
}

void ImGuiViewportHost::draw() {
    if (!controller_) {
        return;
    }
    const ImVec2 available = ImGui::GetContentRegionAvail();
    if (available.x <= 0.0f || available.y <= 0.0f) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetWindowViewport();
    // ImGui coordinates are not necessarily CAD logical pixels. On Windows at 200%,
    // DpiScale is 2 but FramebufferScale is still 1 (window units already are pixels).
    // On Retina, DpiScale can be 1 while FramebufferScale is 2. Keep both conversions:
    // CAD logical size = ImGui size / UI scale; DPR = UI scale * framebuffer density.
    // This preserves native FBO resolution instead of allocating a double-sized image.
    const double uiScale = viewport->DpiScale;
    const int width = std::max(1, static_cast<int>(std::lround(available.x / uiScale)));
    const int height = std::max(1, static_cast<int>(std::lround(available.y / uiScale)));
    const double ratio = uiScale * viewport->FramebufferScale.x;
    const bool changed = needsResize_ || width != size_.logicalWidth || height != size_.logicalHeight || ratio != size_.devicePixelRatio;
    const Viewport2D newSize{width, height, ratio};
    if (!target_.resize(newSize.framebufferWidth(), newSize.framebufferHeight())) {
        ImGui::TextUnformatted("Could not allocate the CAD render target.");
        return;
    }
    size_ = newSize;
    if (changed) {
        // A failed renderer initialization must still leave a defined image to display.
        target_.clear(1.0f, 1.0f, 1.0f);
    }

    // A real ImGui item provides hit testing and mouse ownership. A popup can block
    // new presses, while a drag that started here may continue outside the image.
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("CAD image", available, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
    const bool hovered = ImGui::IsItemHovered();
    {
        ScopedRenderTarget binding(target_);
        if (changed) {
            controller_->onResize({width, height, static_cast<float>(ratio)});
            needsResize_ = false;
            requestRedraw();
        }
        dispatchInput(origin.x, origin.y, available.x, available.y, hovered);
        if (controller_->dirty_ || controller_->continuousRedraw_) {
            controller_->render();
            controller_->dirty_ = false;
        }
    }
    // AddImage uses the already reserved rectangle. Image() would reserve another.
    // This texture remains alive until the UI backend executes the queued draw data.
    ImGui::GetWindowDrawList()->AddImage(ImTextureRef(static_cast<ImTextureID>(target_.colorTexture())), origin,
                                         ImVec2(origin.x + available.x, origin.y + available.y), ImVec2(0, 1), ImVec2(1, 0));
}

void ImGuiViewportHost::dispatchInput(float originX, float originY, float imageWidth, float imageHeight, bool hovered) {
    const auto& io = ImGui::GetIO();
    const auto mods = modifiers(io);
    const auto time = static_cast<uint64_t>(ImGui::GetTime() * 1000.0);
    // On focus loss ImGui can invalidate MousePos. Release at the last known local
    // position instead of feeding its sentinel coordinates into camera/geometry math.
    const bool mouseValid = ImGui::IsMousePosValid(&io.MousePos);
    bool mouseChanged = mods != lastModifiers_ || hovered != wasHovered_;
    if (mouseValid) {
        const double newX = (io.MousePos.x - originX) * size_.logicalWidth / imageWidth;
        const double newY = (io.MousePos.y - originY) * size_.logicalHeight / imageHeight;
        mouseChanged = mouseChanged || newX != lastMouseX_ || newY != lastMouseY_;
        lastMouseX_ = newX;
        lastMouseY_ = newY;
    }
    wasHovered_ = hovered;
    lastModifiers_ = mods;
    const double x = lastMouseX_;
    const double y = lastMouseY_;
    constexpr std::array buttons{input::MouseButton::Left, input::MouseButton::Right, input::MouseButton::Middle};
    // Seed the position before a new press, avoiding a camera jump on right-button pan.
    // A stationary mouse must not force expensive scene rebuilding every UI frame.
    // A dirty controller still gets a refresh, e.g. after choosing a different tool.
    if (mouseValid && (hovered || capturedButtons_ != input::MouseButton::None) && (mouseChanged || controller_->dirty_)) {
        controller_->onMouseMove({x, y, capturedButtons_, mods, time});
    }
    for (int button = 0; button < static_cast<int>(buttons.size()); ++button) {
        const auto flag = buttons[button];
        if (hovered && ImGui::IsMouseClicked(button)) {
            capturedButtons_ = capturedButtons_ | flag;
            controller_->onMouseButton({x, y, flag, input::MouseButtonAction::Press, capturedButtons_, mods, time});
        }
        // MouseDown is cleared on focus loss too. Always finish a captured gesture,
        // including releases outside the image, so tools cannot remain in dragging state.
        if (input::has_flag(capturedButtons_, flag) && (ImGui::IsMouseReleased(button) || !io.MouseDown[button])) {
            capturedButtons_ = static_cast<input::MouseButton>(static_cast<uint32_t>(capturedButtons_) & ~static_cast<uint32_t>(flag));
            controller_->onMouseButton({x, y, flag, input::MouseButtonAction::Release, capturedButtons_, mods, time});
        }
    }
    if (hovered && (io.MouseWheel != 0.0f || io.MouseWheelH != 0.0f)) {
        controller_->onWheel({x, y, io.MouseWheelH, io.MouseWheel, mods, time, false});
    }
    // WantCaptureKeyboard is normally true for the embedded viewport itself. Accept
    // its child focus or an unobstructed hover, so Escape works after choosing a tool
    // without requiring a geometry-creating click first. Protect text entry elsewhere.
    if ((!hovered && !ImGui::IsWindowFocused()) || io.WantTextInput) {
        return;
    }
    const auto dispatchKey = [&](ImGuiKey key, input::KeyCode code) {
        if (ImGui::IsKeyPressed(key, false)) {
            controller_->onKey({code, input::KeyAction::Press, mods, 0, false, time});
        } else if (ImGui::IsKeyPressed(key, true)) {
            controller_->onKey({code, input::KeyAction::Repeat, mods, 0, true, time});
        }
        if (ImGui::IsKeyReleased(key)) {
            controller_->onKey({code, input::KeyAction::Release, mods, 0, false, time});
        }
    };
    constexpr std::array keys{std::pair{ImGuiKey_Escape, input::KeyCode::Escape},
                              std::pair{ImGuiKey_Enter, input::KeyCode::Enter},
                              std::pair{ImGuiKey_Tab, input::KeyCode::Tab},
                              std::pair{ImGuiKey_Space, input::KeyCode::Space},
                              std::pair{ImGuiKey_Backspace, input::KeyCode::Backspace},
                              std::pair{ImGuiKey_Delete, input::KeyCode::Delete},
                              std::pair{ImGuiKey_LeftArrow, input::KeyCode::Left},
                              std::pair{ImGuiKey_RightArrow, input::KeyCode::Right},
                              std::pair{ImGuiKey_UpArrow, input::KeyCode::Up},
                              std::pair{ImGuiKey_DownArrow, input::KeyCode::Down}};
    for (const auto& [key, code] : keys) {
        dispatchKey(key, code);
    }
    for (int letter = 0; letter < 26; ++letter) {
        dispatchKey(static_cast<ImGuiKey>(ImGuiKey_A + letter), static_cast<input::KeyCode>(static_cast<int>(input::KeyCode::A) + letter));
    }
    // Modifiers carry no separate KeyCode in the existing event API, but tools still
    // need their press/release events (for example Shift's temporary line-axis lock).
    constexpr std::array modifierKeys{ImGuiKey_LeftShift, ImGuiKey_RightShift, ImGuiKey_LeftCtrl,  ImGuiKey_RightCtrl,
                                      ImGuiKey_LeftAlt,   ImGuiKey_RightAlt,   ImGuiKey_LeftSuper, ImGuiKey_RightSuper};
    for (const auto key : modifierKeys) {
        dispatchKey(key, input::KeyCode::Unknown);
    }
    for (int digit = 0; digit < 10; ++digit) {
        dispatchKey(static_cast<ImGuiKey>(ImGuiKey_0 + digit), static_cast<input::KeyCode>(static_cast<int>(input::KeyCode::Num0) + digit));
    }
    for (int function = 0; function < 12; ++function) {
        dispatchKey(static_cast<ImGuiKey>(ImGuiKey_F1 + function), static_cast<input::KeyCode>(static_cast<int>(input::KeyCode::F1) + function));
    }
}
