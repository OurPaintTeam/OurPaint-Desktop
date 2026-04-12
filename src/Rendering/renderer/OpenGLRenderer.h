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
    // Grid
    GLuint gridProgram_ = 0;
    GLuint gridVao_ = 0;
    GLuint gridQuadVbo_ = 0;
    GLint gridColorLoc_ = -1;
    GLint gridTransformLoc_ = -1;
    GLint gridZoomLoc_ = -1;
    GLint gridInvViewProjLoc_ = -1;

    // Points
    GLuint pointProgram_ = 0;
    GLuint pointVao_ = 0;
    GLuint pointQuadVbo_ = 0;
    GLuint pointInstanceVbo_ = 0;
    GLint pointColorLoc_ = -1;
    GLint pointTransformLoc_ = -1;

    // Lines
    GLuint lineProgram_ = 0;
    GLuint lineVao_ = 0;
    GLuint lineQuadVbo_ = 0;
    GLuint lineInstanceVbo_ = 0;
    GLint lineColorLoc_ = -1;
    GLint lineTransformLoc_ = -1;

    // Circles
    GLuint circleProgram_ = 0;
    GLuint circleVao_ = 0;
    GLuint circleQuadVbo_ = 0;
    GLuint circleInstanceVbo_ = 0;
    GLint circleTransformLoc_ = -1;
    GLint circleColorLoc_ = -1;

    int width_ = 1;
    int height_ = 1;

    float pointSizeWorld = 1.0f;
    float halfWidthWorld = 1.0f;

private:
    void renderGrid     (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);
    void renderPoints   (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);
    void renderLines    (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);
    void renderCircles  (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);

    void initGlobalState();
    bool initGridPipeline();
    bool initPointPipeline();
    bool initLinePipeline();
    bool initCirclePipeline();

    static GLuint compileShader(GLenum type, const char* src);
    static bool checkProgramLink(GLuint prog);

    bool createProgramFromFiles(const char* vertPath, const char* fragPath, GLuint& outProgram);
};

}

#endif // ! OURPAINT_MVK_OPENGL_RENDERER_H_