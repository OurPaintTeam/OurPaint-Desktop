# ImGui document viewport

The current ImGui executable hosts one `Document` and one `DocumentView`. The
view already owns the camera, overlay, `SketchEditor`, and `ViewportController`.
The latter owns the existing `OpenGL2dRenderer`. No duplicate editor or renderer
is needed for the toolbar.

`ourpaint_editor` builds the shared document-view, editor, tool, picking, and
scene-building sources. Both desktop executables link this library; Qt widgets
and ImGui platform code stay in their respective executables. Core does not
depend on either UI framework.

## Frame and input flow

`ImGuiViewportHost` implements `IViewportHost`. It borrows controllers and owns
an `OpenGLRenderTarget`, but creates no window, context, or independent loop.
Call `draw()` inside the `RenderArea` child window with the GLFW context current.

1. Calculate the logical area size and framebuffer pixel dimensions.
2. Resize the FBO before submitting its texture to ImGui.
3. Reserve an `InvisibleButton` covering the image for ImGui hit testing.
4. Bind the FBO and notify the active controller of size/density changes.
5. Convert ImGui input to local `input::*Event` values and dispatch it.
6. Render a dirty/continuous controller, restore the previous draw framebuffer
   and viewport, and queue the texture using `AddImage()` in the reserved area.
7. The application renders ImGui and swaps the GLFW window buffers once.

Mouse positions are relative to the image, excluding the toolbar and padding.
They are converted from ImGui coordinates to CAD logical units using the image's
logical size. ImGui's viewport `DpiScale` and `FramebufferScale` are different:
on Windows at 200% they are typically 2 and 1, respectively. CAD logical dimensions
are the image dimensions divided by `DpiScale`; CAD DPR is the product of both
scales. Thus a 2000-pixel-wide image represents 1000 CAD logical units at DPR 2,
without allocating a 4000-pixel FBO. Density changes also trigger a controller resize.

The CAD renderer clears to opaque white and blends alpha separately from RGB.
This keeps the completed image opaque even where grid lines or antialiased edges
use partial coverage; ImGui must not blend those pixels against its background again.

Only presses that start on the image reach CAD. Matching releases are delivered
even outside the image; invalid mouse positions on focus loss use the last valid
local position. A visible, unobstructed image or focused RenderArea receives
keyboard events unless ImGui requests text input. Modals and other active items
block new image interactions. This is an embedded editor, so the global
`WantCaptureMouse`/`WantCaptureKeyboard` flags alone cannot route CAD input.

Toolbar buttons invoke the view's existing `SketchEditor::selectTool()`, request
a redraw, and read `activeToolId()` for highlighting. Dimension tools request a
numeric value first; the angle UI accepts degrees and converts to radians before
calling the editor. This adapter does not add new geometry or constraint modes.

## Ownership and resources

Create the document and host before the view, and keep all three inside a scope
that ends before GLFW destroys the context. Before removing the view, cancel its
active tool, call `ViewportController::shutdown()` with its context current, and
remove its borrowed controller from the host. Destroying the host then releases
its FBO while the context is still valid. Qt continues to manage its own contexts;
there is no implicit GL cleanup from the controller destructor.

CMake copies `shaders/` and `fonts/` next to `ourpaint_imgui`. Its startup selects
that directory when launched with an executable path, because the existing
renderer opens those resources using relative paths. The UI keeps docking and
additional platform windows disabled.
