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

#include "DocumentView.h"
#include "Document.h"
#include "ImGuiViewportHost.h"
#include "imgui_internal.h"

constexpr ImVec4 kWindowBg{151/255.f, 136/255.f, 151/255.f, 1.0f}; // #978897
constexpr float kRounding = 3.0f;
constexpr ImVec2 kViewportPadding(10.0f, 10.0f);

// Background of the main window framebuffer.
constexpr float kClearColor[4] = {0.5f, 0.5f, 0.5f, 1.0f};
constexpr float kIconLogical = 32.0f;


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

    glfwSwapInterval(1);  // Enable vsync

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


    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);


    // This scope deliberately ends before GLFW destroys the OpenGL context below.
    // The target owns GPU resources, so its destructor must run with that context current.
    {
        // Destruction is in reverse order: view first, then host/FBO, then document.
        // All of them leave this scope before the GLFW context is destroyed.
        Document document("Untitled");
        ImGuiViewportHost host;
        DocumentView view(document, host);
        host.setEventSink(&view.viewportController_);

        std::optional<ToolId> parameterTool;
        double parameterValue = 1.0;
        bool openParameterPopup = false;
        const auto toolButton = [&](const char* label, ToolId tool, GLuint icon) {
            const bool selected = view.editorSession_.activeToolId() == tool;

            ImGui::PushID(label);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            }

            const float uiSize = kIconLogical * main_scale;
            const bool clicked = (icon != 0)
                ? ImGui::ImageButton("##icon", static_cast<ImTextureID>(static_cast<intptr_t>(icon)), {uiSize, uiSize})
                : ImGui::Button(label);

            if (selected) {
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
                view.editorSession_.selectTool(tool);
                host.requestRedraw();
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
                ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.0f, 0.0f, 0.0f, 0.0f));         // Normal
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));  // Hovered
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(1.0f, 1.0f, 1.0f, 0.3f));   // Pressed
                ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f));           // Text
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
                        view.editorSession_.selectTool(*parameterTool, value);
                        host.requestRedraw();
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
                    host.draw();
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

        // Detach borrowed pointers and release the controller's GPU objects while its
        // context is current. host's own FBO is released at the end of this scope.
        view.editorSession_.activeTool()->cancel();
        view.viewportController_.shutdown();
        host.removeController(&view.viewportController_);
    }


    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
