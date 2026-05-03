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

#include <map>

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
    GLint axisColorLoc_ = -1;
    GLint gridTransformLoc_ = -1;
    GLint gridZoomLoc_ = -1;
    GLint gridInvViewProjLoc_ = -1;

    GLint gridCellSizeLoc_ = -1;
    GLint gridSubCellSizeLoc_ = -1;
    GLint gridOriginLoc_ = -1;



    // Points
    GLuint pointProgram_ = 0;
    GLuint pointVao_ = 0;

    GLuint pointQuadVbo_ = 0;
    GLuint pointInstanceVbo_ = 0;

    GLint pointColorLoc_ = -1;
    GLint pointTransformLoc_ = -1;
    GLint pointPadLoc_ = -1;
    GLint pointEdgeSoftnessLoc_ = -1;

    float pointRadiusPx = 1.5f;
    float pointEdgeSoftnessPx = 1.5f;

    float pointSelectedRadiusPx = 3.0f;
    float pointSelectedEdgeSoftnessPx = 3.0f;



    // Lines
    GLuint lineProgram_ = 0;
    GLuint lineVao_ = 0;

    GLuint lineQuadVbo_ = 0;
    GLuint lineInstanceVbo_ = 0;

    GLint lineColorLoc_ = -1;
    GLint lineAlphaLoc_ = -1;
    GLint lineTransformLoc_ = -1;
    GLint linePadLoc_ = -1;
    GLint lineEdgeSoftnessLoc_ = -1;

    float lineHalfWidthPx = 0.0;
    float lineEdgeSoftnessPx = 1.0;

    float lineSelectedHalfWidthPx = 1.0;
    float lineSelectedEdgeSoftnessPx = 2.0;



    // Circles
    GLuint circleProgram_ = 0;
    GLuint circleVao_ = 0;

    GLuint circleQuadVbo_ = 0;
    GLuint circleInstanceVbo_ = 0;

    GLint circleTransformLoc_ = -1;
    GLint circleColorLoc_ = -1;
    GLint circleZoomLoc_ = -1;
    GLint circleCurveHalfWidthPxLoc_ = -1;
    GLint circleCurveEdgeSoftnessPxLoc_ = -1;

    float circleCurveHalfWidthPx = 0.1;
    float circleCurveEdgeSoftnessPx = 1.0;

    float circleSelectedCurveHalfWidthPx = .5;
    float circleSelectedCurveEdgeSoftnessPx = 2.0;


    // Overlay rectangles
    GLuint rectProgram_ = 0;
    GLuint rectVao_ = 0;

    GLuint rectQuadVbo_ = 0;
    GLuint rectInstanceVbo_ = 0;

    GLint rectTransformLoc_ = -1;
    GLint rectFillColorLoc_ = -1;


    // Viewport size
    int width_ = 1;
    int height_ = 1;

private:
    void renderGrid         (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);
    void renderPoints       (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);
    void renderLines        (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);
    void renderCircles      (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);
    void renderRect         (const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp);


    void initGlobalState();
    bool initGridPipeline();
    bool initPointPipeline();
    bool initLinePipeline();
    bool initCircleArcPipeline();
    bool initRectPipeline();

    static GLuint compileShader(GLenum type, const char* src);
    static bool checkProgramLink(GLuint prog);

    bool createProgramFromFiles(const char* vertPath, const char* fragPath, GLuint& outProgram);

    void initRenderText();
    void renderText(const rendering::text::TextObject& textObj);

    struct Character {
        unsigned int TextureID;  // ID handle of the glyph texture
        glm::ivec2   Size;       // Size of glyph
        glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
        unsigned int Advance;    // Offset to advance to next glyph
    };

    std::map<char, Character> Characters;

    GLuint textProgram_ = 0;
    GLuint textVao_ = 0;
    GLuint textQuadVbo_ = 0;
    GLint textTransformLoc_ = -1;
};

}

#endif // ! OURPAINT_MVK_OPENGL_RENDERER_H_