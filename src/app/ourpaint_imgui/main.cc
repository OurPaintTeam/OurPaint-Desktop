#include <stdio.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <optional>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION

// Keep GLFW from including the system OpenGL headers, regardless of include sorting.
// GLAD supplies the OpenGL declarations and loads the function pointers instead.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <nanosvg.h>
#include <nanosvgrast.h>

#include <iostream>  // TEST

#include "Camera2D.h"
#include "IViewportController.h"
#include "OpenGLRenderTarget.h"
#include "UIController.h"
#include "ViewportController.h"
#include "imgui_internal.h"

constexpr ImVec4 kWindowBg{151/255.f, 136/255.f, 151/255.f, 1.0f}; // #978897
constexpr float kRounding = 3.0f;
constexpr ImVec2 kViewportPadding(10.0f, 10.0f);

// Background of the main window framebuffer.
constexpr float kClearColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
constexpr float kIconLogical = 32.0f;

render::OpenGLRenderTarget target_;

GLuint LoadTextureFromSVG(const char* filename, int* out_width = nullptr, int* out_height = nullptr, float scale = 2.0f) {
    NSVGimage* image = nsvgParseFromFile(filename, "px", 96.0f);
    if (!image) {
        return 0;
    }

    const int w = static_cast<int>(image->width  * scale);
    const int h = static_cast<int>(image->height * scale);
    if (w <= 0 || h <= 0) {
        nsvgDelete(image);
        return 0;
    }

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (!rast) {
        nsvgDelete(image);
        return 0;
    }

    std::vector<unsigned char> buffer(static_cast<size_t>(w) * static_cast<size_t>(h) * 4);
    nsvgRasterize(rast, image, 0, 0, scale, buffer.data(), w, h, w * 4);

    GLuint texture = 0;
    glGenTextures(1, &texture);
    if (texture == 0) {
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    if (out_width) {
        *out_width  = w;
    }
    if (out_height) {
        *out_height = h;
    }
    return texture;
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void draw(app::ViewportController* controller);

int main(int argc, char** argv) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window without title bar and borders
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // glfwGetPrimaryMonitor returns a pointer (GLFWmonitor*) to the OS-designated primary monitor (or NULL if none are found).
    // ImGui_ImplGlfw_GetContentScaleForMonitor takes a monitor pointer (GLFWmonitor*)
    // and returns its DPR as a float (1.5f for 150%).
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    std::cout << "DPR: " << main_scale << '\n';

    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "OurPaint", nullptr, nullptr);
    if (window == nullptr) {
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glfwSwapInterval(0);  // Disable vsync

    // Setup icons and tools
    const float bakeScale = 2.0f * main_scale; // supersample * DPI
    const auto loadIcon = [&](const char* filename) {
        int dummyW{}, dummyH{};
        return LoadTextureFromSVG(filename, &dummyH, &dummyW, bakeScale);
    };

    struct Tool {
        const char* label;
        ToolId id;
        GLuint icon;
    };
    const Tool primitives[] = {
        {"Point"  ,ToolId::Point          ,loadIcon("icons/prim/point.svg"          )},
        {"Line"   ,ToolId::Line           ,loadIcon("icons/prim/line.svg"           )},
        {"Circle" ,ToolId::CircleByRadius ,loadIcon("icons/prim/circleByRadius.svg" )},
        {"Arc"    ,ToolId::ArcByRadius    ,loadIcon("icons/prim/arcByRadius.svg"    )},
        {"Nurbs"  ,ToolId::Nurbs          ,loadIcon("icons/prim/beze.svg"           )},
    };
    const Tool constraints[] = {
        {"Distance"      ,ToolId::Distance           ,loadIcon("icons/Distance.svg"      )},
        {"VDistance"     ,ToolId::VerticalDistance   ,loadIcon("icons/VDistance.svg"     )},
        {"HDistance"     ,ToolId::HorizontalDistance ,loadIcon("icons/HDistance.svg"     )},
        {"Angle"         ,ToolId::Angle              ,loadIcon("icons/Angle.svg"         )},
        {"Coincident"    ,ToolId::Coincident         ,loadIcon("icons/Coincident.svg"    )},
        {"Horizontal"    ,ToolId::Horizontal         ,loadIcon("icons/Horizontal.svg"    )},
        {"Vertical"      ,ToolId::Vertical           ,loadIcon("icons/Vertical.svg"      )},
        {"Parallel"      ,ToolId::Parallel           ,loadIcon("icons/Parallel.svg"      )},
        {"Perpendicular" ,ToolId::Perpendicular      ,loadIcon("icons/Perpendicular.svg" )},
        {"Tangent"       ,ToolId::Tangent            ,loadIcon("icons/Tangent.svg"       )},
        {"Equal"         ,ToolId::Equal              ,loadIcon("icons/Equal.svg"         )},
        {"FixUnfix"      ,ToolId::FixUnfix           ,loadIcon("icons/FixUnfix.svg"      )},
        {"Concentric"    ,ToolId::Concentric         ,loadIcon("icons/Concentric.svg"    )},
        {"Collinear"     ,ToolId::Collinear          ,loadIcon("icons/Collinear.svg"     )},
        {"Midpoint"      ,ToolId::Midpoint           ,loadIcon("icons/Midpoint.svg"      )},
        {"Symmetric"     ,ToolId::Symmetric          ,loadIcon("icons/Symmetric.svg"     )},
        {"Curvature"     ,ToolId::Curvature          ,loadIcon("icons/Curvature.svg"     )},
    };

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;


    // Setup Dear ImGui style
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = kWindowBg;
    // style.FontSizeBase = 16.0f;           // Default to 13.0f when unspecified, which allows crisp legacy font for non-4K screens.
    style.FrameRounding = kRounding;
    style.GrabRounding = kRounding;


    // Setup scaling
    style.ScaleAllSizes(main_scale);  // Bake a fixed style scale. (until we have a solution for dynamic style scaling,
                                      // changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;  // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically
                                      // overrides this for every window depending on the current monitor)
    io.ConfigDpiScaleFonts = true;    // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes.
                                      // This will scale fonts but NOT scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;// [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    io.Fonts->AddFontDefaultVector();


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // This scope deliberately ends before GLFW destroys the OpenGL context below.
    // The target owns GPU resources, so its destructor must run with that context current.
    {
        // Destruction is in reverse order: view first, then host/FBO, then document.
        // All of them leave this scope before the GLFW context is destroyed.

        app::UIController controller;
        const app::DocumentId document_id = controller.createDocument("Untitled");
        const app::ViewId view_id = controller.createDocumentView(document_id);
        controller.setActiveDocumentView(view_id);
        app::ViewportController* viewport_controller = controller.findViewport(view_id);
        if (!viewport_controller) {
            return 1;
        }

        std::optional<ToolId> parameterTool;
        double parameterValue = 1.0;
        bool openParameterPopup = false;
        const auto toolButton = [&](const char* label, ToolId tool, GLuint icon) {
            const auto selected = controller.activeToolId(view_id);

            ImGui::PushID(label);
            if (selected == tool) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            }

            const float uiSize = kIconLogical * main_scale;
            const bool clicked = (icon != 0)
                ? ImGui::ImageButton("##icon", static_cast<ImTextureID>(static_cast<intptr_t>(icon)), {uiSize, uiSize})
                : ImGui::Button(label);

            if (selected == tool) {
                ImGui::PopStyleColor();
            }

            ImGui::SetItemTooltip("%s", label);
            ImGui::PopID();

            if (!clicked) {
                return;
            }
            if (tool == ToolId::Distance ||
                tool == ToolId::HorizontalDistance ||
                tool == ToolId::VerticalDistance ||
                tool == ToolId::Angle) {
                parameterTool = tool;
                parameterValue = tool == ToolId::Angle ? 45.0 : 1.0;
                openParameterPopup = true;
            } else {
                controller.selectTool(view_id, tool);
                draw(viewport_controller);
            }
        };

        // Rendering and ImGui share the same context and frame schedule.
        while (!glfwWindowShouldClose(window)) {
            // GLFW's ImGui backend receives all native events. The embedded CAD region
            // then forwards only its own interaction through ImGuiViewportHost: globally
            // discarding WantCaptureMouse would discard input to our own image as well.
            glfwWaitEvents();
            if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
                ImGui_ImplGlfw_Sleep(10);
                continue;
            }

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // This ImGui window is only a layout host for the render image. Follow the
            // entire GLFW client area every frame instead of loading a movable panel's
            // position/size from imgui.ini. The native OS title bar is unaffected.
            const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(mainViewport->Pos);
            ImGui::SetNextWindowSize(mainViewport->Size);

            // Outer layout padding is specified in ImGui UI units.
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, kViewportPadding);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));

            constexpr ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoDecoration |
                                                       ImGuiWindowFlags_NoMove |
                                                       ImGuiWindowFlags_NoSavedSettings |
                                                       ImGuiWindowFlags_NoDocking |
                                                       ImGuiWindowFlags_NoScrollWithMouse;

            if (ImGui::Begin("Viewport", nullptr, viewportFlags)) {
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f)); // Normal
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f)); // Hovered
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 1.0f, 1.0f, 0.3f)); // Pressed
                ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Text
                ImGui::PushStyleColor(ImGuiCol_Separator,     ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);          // Rounded tool buttons
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);        // No button border
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 5));  // Inner padding

                if (ImGui::BeginChild("Toolbar", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar)) {
                    // Group primitives
                    ImGui::BeginGroup();
                    for (auto& e : primitives) {
                        toolButton(e.label, e.id, e.icon);
                        ImGui::SameLine(0,0);
                    }
                    ImGui::EndGroup();

                    ImGui::SameLine(0.0f, 10.0f);
                    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                    ImGui::SameLine(0.0f, 10.0f);

                    // Group constraints
                    ImGui::BeginGroup();
                    for (auto& e : constraints) {
                        toolButton(e.label, e.id, e.icon);
                        ImGui::SameLine(0,0);
                    }
                    ImGui::EndGroup();
                }

                ImGui::EndChild();
                ImGui::PopStyleColor(5);
                ImGui::PopStyleVar(3);

                // Open the modal at the same ImGui ID scope where it is displayed.
                if (openParameterPopup) {
                    ImGui::OpenPopup("Constraint value");
                    openParameterPopup = false;
                }
                if (ImGui::BeginPopupModal("Constraint value", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::TextUnformatted(parameterTool == ToolId::Angle ? "Angle (degrees)" : "Distance (sketch units)");
                    const bool entered = ImGui::InputDouble("Value", &parameterValue, 0.0, 0.0, "%.6g", ImGuiInputTextFlags_EnterReturnsTrue);
                    const bool valid = std::isfinite(parameterValue);
                    ImGui::BeginDisabled(!valid);
                    if ((ImGui::Button("Apply") || entered) && valid && parameterTool) {
                        const double value = *parameterTool == ToolId::Angle ? parameterValue * std::numbers::pi / 180.0 : parameterValue;
                        controller.selectTool(view_id, *parameterTool, value);
                        parameterTool.reset();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        parameterTool.reset();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::BeginChild("RenderArea", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
                    draw(viewport_controller);
                }
                ImGui::EndChild();
            }

            // End() is required even if Begin() skips the contents. Keep these style
            // overrides local so future UI elements can still use the regular style.
            ImGui::End();
            ImGui::PopStyleVar(4);
            // Rendering
            ImGui::Render();
            int display_w{}, display_h{};
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(kClearColor[0], kClearColor[1], kClearColor[2], kClearColor[3]);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            glfwSwapBuffers(window);
        }

    }


    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void dispatchInput(float originX, float originY, float imageWidth, float imageHeight, bool hovered, app::ViewportController* controller, render::Viewport2D);

void draw(app::ViewportController* controller) {
    const ImVec2 available = ImGui::GetContentRegionAvail();
    if (available.x <= 0.0f || available.y <= 0.0f) {
        return;
    }
    const ImGuiViewport* viewport = ImGui::GetWindowViewport();
    const double uiScale = viewport->DpiScale;
    const int width = std::max(1, static_cast<int>(std::lround(available.x / uiScale)));
    const int height = std::max(1, static_cast<int>(std::lround(available.y / uiScale)));

    // Compute a universal scaling ratio for rendering.
    // uiScale handles content scaling. Typically > 1 on Windows/Linux when the OS uses physical pixels as logical.
    // FramebufferScale handles the framebuffer-to-window ratio (DPR: physical pixels per logical pixel)
    // typically > 1 on macOS/iOS/Android Retina displays.
    // Multiplying them gives the final ratio needed
    // to convert ImGui logical units into actual framebuffer pixels, regardless of platform.
    const double ratio = uiScale * viewport->FramebufferScale.x;

    const render::Viewport2D newSize{width, height, ratio};
    if (!target_.resize(newSize.framebufferWidth(), newSize.framebufferHeight())) {
        ImGui::TextUnformatted("Could not allocate the CAD render target.");
        return;
    }

    // A real ImGui item provides hit testing and mouse ownership. A popup can block
    // new presses, while a drag that started here may continue outside the image.
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("CAD image", available, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
    const bool hovered = ImGui::IsItemHovered();

    GLint framebuffer_ = 0;
    GLint viewport_[4]{};
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer_);
    glGetIntegerv(GL_VIEWPORT, viewport_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target_.framebuffer());
    glViewport(0, 0, target_.width(), target_.height());

    controller->onResize({width, height, static_cast<float>(ratio)});

    dispatchInput(origin.x, origin.y, available.x, available.y, hovered, controller, newSize);

    controller->render();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(framebuffer_));
    glViewport(viewport_[0], viewport_[1], viewport_[2], viewport_[3]);


    // AddImage uses the already reserved rectangle. Image() would reserve another.
    // This texture remains alive until the UI backend executes the queued draw data.
    ImGui::GetWindowDrawList()->AddImage(ImTextureRef(target_.colorTexture()),
                                   origin,
                                   ImVec2(origin.x + available.x, origin.y + available.y),
                                  ImVec2(0, 1),
                                  ImVec2(1, 0));


    // std::cout << "====================== New Frame started" << '\n';
    // std::cout << "frameBuffer (w=" << available.x << ", h=" << available.y << ')' << '\n';
    // std::cout << "logical size (w=" << width << ", h=" << height << ')' << '\n';
    // std::cout << "Ratio=" << ratio << '\n';
}

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

input::MouseButton capturedButtons_ = input::MouseButton::None;
double lastMouseX_ = 0.0;
double lastMouseY_ = 0.0;
input::Modifiers lastModifiers_ = input::Modifiers::None;
bool wasHovered_ = false;

void dispatchInput(float originX, float originY, float imageWidth, float imageHeight, bool hovered, app::ViewportController* controller_, render::Viewport2D size_) {
    const auto& io = ImGui::GetIO();
    const auto mods = modifiers(io);
    const auto time = static_cast<uint64_t>(ImGui::GetTime() * 1000.0);
    // On focus loss ImGui can invalidate MousePos. Release at the last known local
    // position instead of feeding its sentinel coordinates into camera/geometry math.
    const bool mouseValid = ImGui::IsMousePosValid(&io.MousePos);

    bool mouseChanged = mods != lastModifiers_ || hovered != wasHovered_;
    if (mouseValid) {
        const double newX = (io.MousePos.x - originX) *  size_.logicalWidth / imageWidth;
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

