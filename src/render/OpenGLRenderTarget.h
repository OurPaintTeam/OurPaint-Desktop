#ifndef OURPAINT_RENDER_OPENGL_RENDER_TARGET_H_
#define OURPAINT_RENDER_OPENGL_RENDER_TARGET_H_

#include <glad/gl.h>

namespace render {

// Owns an offscreen framebuffer and its single RGBA8 color texture. The FBO describes
// where OpenGL writes; the attached texture actually stores the resulting pixels.
// This OpenGL-specific resource belongs to the renderer, not to the CAD document or UI.
//
// Construction does not call OpenGL. resize(), clear(), and destruction require the
// owning OpenGL context to be current and GLAD to have been initialized. In particular,
// destroy this object BEFORE destroying the window/context that created its resources.
class OpenGLRenderTarget {
public:
    OpenGLRenderTarget() = default;
    ~OpenGLRenderTarget();

    // A GLuint is only an object name, not shared ownership. Copying these names would
    // make two owners delete the same resources. Moving is unnecessary for this first use.
    OpenGLRenderTarget(const OpenGLRenderTarget&) = delete;
    OpenGLRenderTarget& operator=(const OpenGLRenderTarget&) = delete;
    OpenGLRenderTarget(OpenGLRenderTarget&&) = delete;
    OpenGLRenderTarget& operator=(OpenGLRenderTarget&&) = delete;

    // Allocates on first use and replaces the attachments only when the pixel size changes.
    // Returns false for invalid/unsupported sizes or an incomplete allocation, retaining
    // the previous valid target. Call before submitting the texture to ImGui::Image(),
    // outside any pass that binds the old framebuffer or samples its texture.
    [[nodiscard]] bool resize(int width, int height);

    // Clears the color attachment without changing the caller's framebuffer binding,
    // viewport, clear color, scissor enable, or color write mask. This is the first demo
    // pass; later, CAD geometry can be rendered into framebuffer() instead of just clearing.
    void clear(float red, float green, float blue, float alpha = 1.0f) const;

    // These are borrowed OpenGL names. Do not delete them or retain them across resize().
    // The UI passes colorTexture() to ImGui; framebuffer() is for a future CAD render pass.
    GLuint framebuffer() const { return framebuffer_; }
    GLuint colorTexture() const { return colorTexture_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    void release();

    GLuint framebuffer_ = 0;
    GLuint colorTexture_ = 0;
    int width_ = 0;
    int height_ = 0;
};

}  // namespace render

#endif  // OURPAINT_RENDER_OPENGL_RENDER_TARGET_H_
