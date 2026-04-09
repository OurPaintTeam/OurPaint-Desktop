#ifndef OURPAINT_MVK_OPENGL_RENDERER_H_
#define OURPAINT_MVK_OPENGL_RENDERER_H_

#include <glad/gl.h>

#include "IRenderer.h"
#include "RenderData.h"
#include "Logger.h"

#include <string>
#include "Camera2D.h"

// ------------------------------
// OpenGLRenderer (backend)`
// ------------------------------


namespace renderer {

class OpenGLRenderer : public IRenderer {
public:
    bool initialize() override;
    void resize(int w, int h) override;
    void shutdown() override;
    void render(const RenderData& scene, const Camera2D& camera) override;

private:
    GLuint primitiveProgram_ = 0;
    GLuint primitiveVao_ = 0;
    GLuint primitiveVbo_ = 0;
    GLint primitiveColorLoc_ = -1;
    GLint primitiveTransformLoc_ = -1;

    // Circles
    GLuint circleProgram_ = 0;
    GLuint circleVao_ = 0;
    GLuint circleQuadVbo_ = 0;
    GLuint circleInstanceVbo_ = 0;
    GLint circleTransformLoc_ = -1;
    GLint circleColorLoc_ = -1;

    int width_ = 1;
    int height_ = 1;

private:
    void renderPointsAndLines(const RenderData& scene, const glm::mat4& mvp);
    void renderCircles(const RenderData& scene, const glm::mat4& mvp);

    void initGlobalState();
    bool initializePrimitivePipeline();
    bool initializeCirclePipeline();

    static GLuint compileShader(GLenum type, const char* src);
    static bool checkProgramLink(GLuint prog);
};

}

#endif // ! OURPAINT_MVK_OPENGL_RENDERER_H_