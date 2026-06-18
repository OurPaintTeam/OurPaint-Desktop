#ifndef OURPAINT_OPENGL_RENDERER_H_
#define OURPAINT_OPENGL_RENDERER_H_

#include <map>
#include <string>

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "Camera2D.h"
#include "IRenderer.h"
#include "RenderScene.h"

namespace render {

class OpenGL2dRenderer : public IRenderer {
public:
    bool initialize() override;
    void resize(int w, int h) override;
    void shutdown() override;
    void render(const RenderScene& scene, const Camera2D& camera) override;

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

    // Markers
    GLuint pointProgram_ = 0;
    GLuint pointVao_ = 0;

    GLuint pointQuadVbo_ = 0;
    GLuint pointInstanceVbo_ = 0;

    GLint pointColorLoc_ = -1;
    GLint pointTransformLoc_ = -1;
    GLint pointPadLoc_ = -1;
    GLint pointEdgeSoftnessLoc_ = -1;

    float pointEdgeSoftnessPx_ = 1.5f;

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

    float lineEdgeSoftnessPx_ = 1.5f;

    // Circles / arcs
    GLuint circleProgram_ = 0;
    GLuint circleVao_ = 0;

    GLuint circleQuadVbo_ = 0;
    GLuint circleInstanceVbo_ = 0;

    GLint circleTransformLoc_ = -1;
    GLint circleColorLoc_ = -1;
    GLint circleZoomLoc_ = -1;
    GLint circleCurveHalfWidthPxLoc_ = -1;
    GLint circleCurveEdgeSoftnessPxLoc_ = -1;

    float circleCurveEdgeSoftnessPx_ = 1.0f;

    // Rectangles
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
    void renderGrid(const RenderScene& scene, const Camera2D& camera);
    void renderLayers(const RenderScene& scene, const Camera2D& camera);
    void renderLayer(const DrawLayer& layer, const Camera2D& camera);

    void renderMarkerBatch(
        const MarkerBatch& batch,
        CoordinateSpace coordinateSpace,
        const Camera2D& camera,
        const glm::mat4& transform
    );

    void renderLineBatch(
        const LineBatch& batch,
        CoordinateSpace coordinateSpace,
        const Camera2D& camera,
        const glm::mat4& transform
    );

    void renderCircleBatch(
        const CircleBatch& batch,
        const Camera2D& camera,
        const glm::mat4& transform
    );

    void renderArcBatch(
        const ArcBatch& batch,
        const Camera2D& camera,
        const glm::mat4& transform
    );

    void renderRectBatch(
        const RectBatch& batch,
        const glm::mat4& transform
    );

    glm::mat4 transformFor(CoordinateSpace coordinateSpace, const Camera2D& camera) const;

private:
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

    // Keep text rendering logic unchanged for now.
    void renderText(const render::text::TextObject& textObj);

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

} // namespace render

#endif // ! OURPAINT_OPENGL_RENDERER_H_