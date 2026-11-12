#include "OpenGLRenderTarget.h"

namespace render {

OpenGLRenderTarget::~OpenGLRenderTarget() { release(); }

bool OpenGLRenderTarget::resize(int width, int height) {
    // A collapsed/minimized panel has no useful image to allocate. The caller skips it
    // rather than attempting to create an incomplete zero-sized framebuffer.
    if (width <= 0 || height <= 0) {
        return false;
    }
    if (framebuffer_ != 0 && width == width_ && height == height_) {
        return true;
    }

    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    if (width > maxTextureSize || height > maxTextureSize) {
        return false;
    }

    // OpenGL bindings are context state shared by all renderers using this context.
    // Save the active texture unit's 2D binding and the draw framebuffer so allocating
    // this target does not accidentally redirect Qt, ImGui, or another render pass.
    GLint previousDrawFramebuffer = 0;
    GLint previousReadFramebuffer = 0;
    GLint previousTexture = 0;
    GLint previousUnpackBuffer = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDrawFramebuffer);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousReadFramebuffer);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &previousUnpackBuffer);

    // Replacing a resource while a caller has it bound would invalidate that caller's
    // saved state. resize() belongs before the pass, not inside a pass using this target.
    if ((framebuffer_ != 0 && (static_cast<GLuint>(previousDrawFramebuffer) == framebuffer_ || static_cast<GLuint>(previousReadFramebuffer) == framebuffer_)) ||
        (colorTexture_ != 0 && static_cast<GLuint>(previousTexture) == colorTexture_)) {
        return false;
    }

    // Build a replacement first. A failed allocation must not destroy the last usable
    // image; only a framebuffer-complete replacement is committed to the member fields.
    GLuint newFramebuffer = 0;
    GLuint newTexture = 0;
    glGenFramebuffers(1, &newFramebuffer);
    glGenTextures(1, &newTexture);

    bool complete = false;
    if (newFramebuffer != 0 && newTexture != 0) {
        glBindTexture(GL_TEXTURE_2D, newTexture);

        // The default minification filter expects mipmaps. This texture has only level 0,
        // so use ordinary linear filtering and clamp at the edges of the viewport image.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // With no unpack buffer bound, nullptr means "allocate without initial pixels".
        // Otherwise OpenGL interprets it as byte offset 0 into that buffer and tries to
        // upload its contents. The renderer will fill the texture in a later pass.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // GL_DRAW_FRAMEBUFFER changes only the write target; the caller's read framebuffer
        // remains untouched. A single color attachment is enough for this 2D demo: no
        // depth/stencil renderbuffer, multisample texture, or resolve pass is needed yet.
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, newFramebuffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, newTexture, 0);
        complete = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    // Restore actual previous bindings, not an assumed framebuffer 0. Qt and other
    // hosts can have a different default target. Allocation does not touch the viewport.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previousDrawFramebuffer));
    glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(previousTexture));
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, static_cast<GLuint>(previousUnpackBuffer));

    if (!complete) {
        glDeleteFramebuffers(1, &newFramebuffer);
        glDeleteTextures(1, &newTexture);
        return false;
    }

    release();
    framebuffer_ = newFramebuffer;
    colorTexture_ = newTexture;
    width_ = width;
    height_ = height;
    return true;
}

void OpenGLRenderTarget::clear(float red, float green, float blue, float alpha) const {
    if (framebuffer_ == 0) {
        return;
    }

    GLint previousDrawFramebuffer = 0;
    GLboolean previousColorMask[4];
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDrawFramebuffer);
    glGetBooleani_v(GL_COLOR_WRITEMASK, 0, previousColorMask);
    const GLboolean scissorWasEnabled = glIsEnabled(GL_SCISSOR_TEST);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_);

    // A clear covers the attachment independently of glViewport(), but scissor and color
    // masks still restrict it. Temporarily allow the whole image and all RGBA components.
    // Use the indexed mask for attachment 0: the non-indexed glColorMask() would also
    // overwrite masks belonging to other draw buffers in a caller's multi-target pass.
    glDisable(GL_SCISSOR_TEST);
    glColorMaski(0, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Clear color attachment 0 directly. Unlike glClearColor() + glClear(), this leaves
    // the window/UI clear color unchanged. Alpha 1 makes the viewport image opaque.
    const GLfloat color[] = {red, green, blue, alpha};
    glClearBufferfv(GL_COLOR, 0, color);

    glColorMaski(0, previousColorMask[0], previousColorMask[1], previousColorMask[2], previousColorMask[3]);
    if (scissorWasEnabled) {
        glEnable(GL_SCISSOR_TEST);
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previousDrawFramebuffer));
}

void OpenGLRenderTarget::release() {
    // OpenGL ignores name 0, so releasing an unallocated target is harmless. The caller
    // must still keep the creating context current whenever real resources are deleted.
    if (framebuffer_ != 0) {
        glDeleteFramebuffers(1, &framebuffer_);
    }
    if (colorTexture_ != 0) {
        glDeleteTextures(1, &colorTexture_);
    }
    framebuffer_ = 0;
    colorTexture_ = 0;
    width_ = 0;
    height_ = 0;
}

}  // namespace render
