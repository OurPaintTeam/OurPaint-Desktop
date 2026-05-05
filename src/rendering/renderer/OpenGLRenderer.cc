#include "OpenGLRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera2D.h"
#include "RenderData.h"
#include "shaders/shader_utils.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library  library;   /* handle to library     */
FT_Face     face;      /* handle to face object */

using namespace renderer;

namespace {
struct PointInstance {
    float x;
    float y;
    float size;
};
struct CircleArcInstance {
    float x;
    float y;
    float r;
    float startAngle;
    float endAngle;
};
struct LineInstance {
    float x1;
    float y1;
    float x2;
    float y2;
    float halfWidthWorld;
};
struct RectInstance {
    float cx;
    float cy;
    float hx;
    float hy;
};
}

bool OpenGLRenderer::initialize() {
    initGlobalState();
    if (!initGridPipeline()) {
        return false;
    }
    if (!initPointPipeline()) {
        return false;
    }
    if (!initLinePipeline()) {
        return false;
    }
    if (!initCircleArcPipeline()) {
        return false;
    }
    if (!initRectPipeline()) {
        return false;
    }
    initRenderText();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return true;
}

void OpenGLRenderer::resize(int w, int h) {
    width_ = w;
    height_ = h;
    glViewport(0, 0, w, h);
}

void OpenGLRenderer::shutdown() {
    // Grid states
    if (gridQuadVbo_) {
        glDeleteBuffers(1, &gridQuadVbo_);
    }
    if (gridVao_) {
        glDeleteVertexArrays(1, &gridVao_);
    }
    if (gridProgram_) {
        glDeleteProgram(gridProgram_);
    }
    gridQuadVbo_ = 0;
    gridVao_ = 0;
    gridProgram_ = 0;
    gridColorLoc_ = -1;

    // Points states
    if (pointProgram_) {
        glDeleteProgram(pointProgram_);
    }
    if (pointVao_) {
        glDeleteVertexArrays(1, &pointVao_);
    }
    if (pointQuadVbo_) {
        glDeleteBuffers(1, &pointQuadVbo_);
    }
    if (pointInstanceVbo_) {
        glDeleteBuffers(1, &pointInstanceVbo_);
    }
    pointProgram_ = 0;
    pointVao_ = 0;
    pointQuadVbo_ = 0;
    pointInstanceVbo_ = 0;
    pointColorLoc_ = -1;
    pointTransformLoc_ = -1;
    pointPadLoc_ = -1;
    pointEdgeSoftnessLoc_ = -1;

    // Line states
    if (lineInstanceVbo_) {
        glDeleteBuffers(1, &lineInstanceVbo_);
    }
    if (lineQuadVbo_) {
        glDeleteBuffers(1, &lineQuadVbo_);
    }
    if (lineVao_) {
        glDeleteVertexArrays(1, &lineVao_);
    }
    if (lineProgram_) {
        glDeleteProgram(lineProgram_);
    }
    lineInstanceVbo_ = 0;
    lineQuadVbo_ = 0;
    lineVao_ = 0;
    lineProgram_ = 0;
    lineColorLoc_ = -1;
    lineTransformLoc_ = -1;

    // Circles states
    if (circleInstanceVbo_) {
        glDeleteBuffers(1, &circleInstanceVbo_);
    }
    if (circleQuadVbo_) {
        glDeleteBuffers(1, &circleQuadVbo_);
    }
    if (circleVao_) {
        glDeleteVertexArrays(1, &circleVao_);
    }
    if (circleProgram_) {
        glDeleteProgram(circleProgram_);
    }
    circleInstanceVbo_ = 0;
    circleQuadVbo_ = 0;
    circleVao_ = 0;
    circleProgram_ = 0;
    circleColorLoc_ = -1;
    circleTransformLoc_ = -1;
}

void OpenGLRenderer::render(const RenderData& rd, const Camera2D& camera) {
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 mvp = camera.viewProjectionMatrix();

    renderGrid(rd, camera, mvp);
    renderLines(rd, camera, mvp);
    renderPoints(rd, camera, mvp);
    renderCircles(rd, camera, mvp);
    renderRect(rd, camera, mvp);

    for (const auto& t : rd.textObjects_) {
        renderText(t);
    }
}

void OpenGLRenderer::renderGrid(const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp) {
    if (!gridProgram_ || !gridVao_ || !gridQuadVbo_) {
        return;
    }

    glUseProgram(gridProgram_);

    glm::mat4 viewProj = camera.viewProjectionMatrix();
    glm::mat4 invViewProj = glm::inverse(viewProj);

    if (gridCellSizeLoc_ >= 0) {
        glUniform1f(gridCellSizeLoc_, static_cast<float>(scene.gridInfo.cellSize));
    }

    if (gridSubCellSizeLoc_ >= 0) {
        glUniform1f(gridSubCellSizeLoc_, static_cast<float>(scene.gridInfo.subCellSize));
    }

    glm::vec3 c = scene.gridInfo.gridColor;
    if (gridColorLoc_ >= 0) {
        glUniform3f(gridColorLoc_, c.r, c.g, c.b);
    }

    c = scene.gridInfo.axisColor;
    if (axisColorLoc_ >= 0) {
        glUniform3f(axisColorLoc_, c.r, c.g, c.b);
    }

    if (gridZoomLoc_ >= 0) {
        glUniform1f(gridZoomLoc_, camera.zoom());
    }

    if (gridInvViewProjLoc_ >= 0) {
        glUniformMatrix4fv(gridInvViewProjLoc_, 1, GL_FALSE, glm::value_ptr(invViewProj));
    }

    glBindVertexArray(gridVao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::renderPoints(const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp) {
    if (!pointProgram_ || !pointVao_ || !pointQuadVbo_ || !pointInstanceVbo_) {
        return;
    }


    float worldPerPixel = 1.0f / camera.zoom();


    // Selected points
    float pointSizeWorld = pointSelectedRadiusPx * worldPerPixel;
    pointSelectedRadiusPx = glm::max(pointSelectedRadiusPx, 1e-6f);
    float edgeSoftness = pointSelectedEdgeSoftnessPx / pointSelectedRadiusPx;
    float pointPad = 1.0f + edgeSoftness;

    const size_t selectedCount = scene.selected.points.size();
    if (selectedCount == 0) {
        //return;
    }

    std::vector<PointInstance> selectedInstances;
    selectedInstances.reserve(selectedCount);

    for (const auto& p : scene.selected.points) {
        selectedInstances.push_back({p.x, p.y, pointSizeWorld});
    }

    glUseProgram(pointProgram_);
    glBindVertexArray(pointVao_);
    glBindBuffer(GL_ARRAY_BUFFER, pointInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(selectedInstances.size() * sizeof(PointInstance)),
        selectedInstances.data(),
        GL_DYNAMIC_DRAW
    );

    if (pointTransformLoc_ >= 0) {
        glUniformMatrix4fv(pointTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (pointColorLoc_ >= 0) {
        glUniform3f(pointColorLoc_, 0.0f, 1.0f, 1.0f);
    }

    if (pointPadLoc_ >= 0) {
        glUniform1f(pointPadLoc_, pointPad);
    }

    if (pointEdgeSoftnessLoc_ >= 0) {
        glUniform1f(pointEdgeSoftnessLoc_, edgeSoftness);
    }

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(selectedInstances.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);




    // Base and overlay points
    pointSizeWorld = pointRadiusPx * worldPerPixel;
    pointRadiusPx = glm::max(pointRadiusPx, 1e-6f);
    edgeSoftness = pointEdgeSoftnessPx / pointRadiusPx;
    pointPad = 1.0f + edgeSoftness;

    const size_t count = scene.points.size() + scene.overlay.points.size();
    if (count == 0) {
        //return;
    }

    std::vector<PointInstance> instances;
    instances.reserve(count);

    for (const auto& p : scene.overlay.points) {
        instances.push_back({p.x, p.y, pointSizeWorld});
    }

    for (const auto& p : scene.points) {
        instances.push_back({p.x, p.y, pointSizeWorld});
    }

    glUseProgram(pointProgram_);
    glBindVertexArray(pointVao_);
    glBindBuffer(GL_ARRAY_BUFFER, pointInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instances.size() * sizeof(PointInstance)),
        instances.data(),
        GL_DYNAMIC_DRAW
    );

    if (pointTransformLoc_ >= 0) {
        glUniformMatrix4fv(pointTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    float r = 63.0 / 255.0;
    float g = 72.0 / 255.0;
    float b = 204.0 / 255.0;

    if (pointColorLoc_ >= 0) {
        glUniform3f(pointColorLoc_, r, g, b);
    }

    if (pointPadLoc_ >= 0) {
        glUniform1f(pointPadLoc_, pointPad);
    }

    if (pointEdgeSoftnessLoc_ >= 0) {
        glUniform1f(pointEdgeSoftnessLoc_, edgeSoftness);
    }

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instances.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::renderLines(const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp) {
    if (!lineProgram_ || !lineVao_ || !lineQuadVbo_ || !lineInstanceVbo_) {
        return;
    }

    float worldPerPixel = 1.0f / camera.zoom();


    // Selected lines
    float lineHalfWidthWorld = lineSelectedHalfWidthPx * worldPerPixel;
    lineSelectedHalfWidthPx = glm::max(lineSelectedHalfWidthPx, 1e-6f);
    float lineEdgeSoftness = lineSelectedEdgeSoftnessPx / lineSelectedHalfWidthPx;
    float linePad = 1.0f + lineEdgeSoftness;

    const size_t countSelected = scene.selected.lines.size();
    if (countSelected == 0) {
        //return;
    }

    std::vector<LineInstance> selectedInstances;
    selectedInstances.reserve(countSelected);

    for (const auto& l : scene.selected.lines) {
        selectedInstances.push_back({l.x1, l.y1, l.x2, l.y2, lineHalfWidthWorld});
    }

    glUseProgram(lineProgram_);
    glBindVertexArray(lineVao_);
    glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(selectedInstances.size() * sizeof(LineInstance)),
        selectedInstances.data(),
        GL_DYNAMIC_DRAW
    );

    if (lineTransformLoc_ >= 0) {
        glUniformMatrix4fv(lineTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (lineColorLoc_ >= 0) {
        glUniform3f(lineColorLoc_, 0.0f, 1.0f, 1.0f);
    }

    if (linePadLoc_ >= 0) {
        glUniform1f(linePadLoc_, linePad);
    }

    if (lineEdgeSoftnessLoc_ >= 0) {
        glUniform1f(lineEdgeSoftnessLoc_, lineEdgeSoftness);
    }

    if (lineAlphaLoc_ >= 0) {
        glUniform1f(lineAlphaLoc_, 1.0);
    }


    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(selectedInstances.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);


    // Base lines + overlay lines
    lineHalfWidthWorld = lineHalfWidthPx * worldPerPixel;
    lineHalfWidthPx = glm::max(lineHalfWidthPx, 1e-6f);
    lineEdgeSoftness = lineEdgeSoftnessPx / lineHalfWidthPx;
    linePad = 1.0f + lineEdgeSoftness;

    const size_t count = scene.lines.size() + scene.overlay.lines.size();
    if (count == 0) {
        //return;
    }

    std::vector<LineInstance> instances;
    instances.reserve(count);

    for (const auto& l : scene.overlay.lines) {
        instances.push_back({l.x1, l.y1, l.x2, l.y2, lineHalfWidthWorld});
    }

    for (const auto& l : scene.lines) {
        instances.push_back({l.x1, l.y1, l.x2, l.y2, lineHalfWidthWorld});
    }

    glUseProgram(lineProgram_);
    glBindVertexArray(lineVao_);
    glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instances.size() * sizeof(LineInstance)),
        instances.data(),
        GL_DYNAMIC_DRAW
    );

    if (lineTransformLoc_ >= 0) {
        glUniformMatrix4fv(lineTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (lineColorLoc_ >= 0) {
        glUniform3f(lineColorLoc_, 0.0f, 0.0f, 0.0f);
    }

    if (linePadLoc_ >= 0) {
        glUniform1f(linePadLoc_, linePad);
    }

    if (lineEdgeSoftnessLoc_ >= 0) {
        glUniform1f(lineEdgeSoftnessLoc_, lineEdgeSoftness);
    }

    if (lineAlphaLoc_ >= 0) {
        glUniform1f(lineAlphaLoc_, 1.0);
    }


    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instances.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);




    // Special lines
    lineHalfWidthWorld = lineHalfWidthPx * worldPerPixel;
    lineHalfWidthPx = glm::max(lineHalfWidthPx, 1e-6f);
    lineEdgeSoftness = lineEdgeSoftnessPx / lineHalfWidthPx;
    linePad = 1.0f + lineEdgeSoftness;

    const size_t countSpecial = scene.special.lines.size();
    if (countSpecial == 0) {
        //return;
    }

    std::vector<LineInstance> instancesSpecial;
    instancesSpecial.reserve(count);

    for (const auto& l : scene.special.lines) {
        instancesSpecial.push_back({l.x1, l.y1, l.x2, l.y2, lineHalfWidthWorld});
    }

    glUseProgram(lineProgram_);
    glBindVertexArray(lineVao_);
    glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instancesSpecial.size() * sizeof(LineInstance)),
        instancesSpecial.data(),
        GL_DYNAMIC_DRAW
    );

    if (lineTransformLoc_ >= 0) {
        glUniformMatrix4fv(lineTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (lineColorLoc_ >= 0) {
        glUniform3f(lineColorLoc_, 0.5f, 0.5f, 0.5f);
    }

    if (linePadLoc_ >= 0) {
        glUniform1f(linePadLoc_, linePad);
    }

    if (lineEdgeSoftnessLoc_ >= 0) {
        glUniform1f(lineEdgeSoftnessLoc_, lineEdgeSoftness);
    }

    if (lineAlphaLoc_ >= 0) {
        glUniform1f(lineAlphaLoc_, 0.4);
    }

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instancesSpecial.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::renderCircles(const RenderData& renderData, const Camera2D& camera, const glm::mat4& mvp) {
    if (!circleProgram_ || !circleVao_ || !circleQuadVbo_ || !circleInstanceVbo_) {
        return;
    }


    size_t circlesSelectedCount = renderData.selected.circles.size();
    if (circlesSelectedCount == 0) {
        //return;
    }

    std::vector<CircleArcInstance> selectedInstances;
    selectedInstances.reserve(circlesSelectedCount);

    for (const auto& c : renderData.selected.circles) {
        selectedInstances.push_back(CircleArcInstance{ c.x, c.y, c.r, c.startAngle, c.endAngle });
    }

    glUseProgram(circleProgram_);
    glBindVertexArray(circleVao_);
    glBindBuffer(GL_ARRAY_BUFFER, circleInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(selectedInstances.size() * sizeof(CircleArcInstance)),
        selectedInstances.data(),
        GL_DYNAMIC_DRAW
    );

    if (circleTransformLoc_ >= 0) {
        glUniformMatrix4fv(circleTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (circleColorLoc_ >= 0) {
        glUniform3f(circleColorLoc_, 0.0f, 1.0f, 1.0f);
    }

    if (circleZoomLoc_ >= 0) {
        glUniform1f(circleZoomLoc_, camera.zoom());
    }

    if (circleCurveHalfWidthPxLoc_ >= 0) {
        glUniform1f(circleCurveHalfWidthPxLoc_, circleSelectedCurveHalfWidthPx);
    }

    if (circleCurveEdgeSoftnessPxLoc_ >= 0) {
        glUniform1f(circleCurveEdgeSoftnessPxLoc_, circleSelectedCurveEdgeSoftnessPx);
    }

    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,                                      // first vertex
        6,                                      // 6 vertices in quad
        static_cast<GLsizei>(selectedInstances.size())  // number of circles
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    // Base + overlay
    size_t circlesCount = renderData.circles.size() + renderData.overlay.circles.size();
    if (circlesCount == 0) {
        //return;
    }

    std::vector<CircleArcInstance> instances;
    instances.reserve(circlesCount);

    for (const auto& c : renderData.overlay.circles) {
        instances.push_back(CircleArcInstance{ c.x, c.y, c.r, c.startAngle, c.endAngle });
    }

    for (const auto& c : renderData.circles) {
        instances.push_back(CircleArcInstance{ c.x, c.y, c.r, c.startAngle, c.endAngle });
    }

    glUseProgram(circleProgram_);
    glBindVertexArray(circleVao_);
    glBindBuffer(GL_ARRAY_BUFFER, circleInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instances.size() * sizeof(CircleArcInstance)),
        instances.data(),
        GL_DYNAMIC_DRAW
    );

    if (circleTransformLoc_ >= 0) {
        glUniformMatrix4fv(circleTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (circleColorLoc_ >= 0) {
        glUniform3f(circleColorLoc_, 0.0f, 0.0f, 0.0f);
    }

    if (circleZoomLoc_ >= 0) {
        glUniform1f(circleZoomLoc_, camera.zoom());
    }

    if (circleCurveHalfWidthPxLoc_ >= 0) {
        glUniform1f(circleCurveHalfWidthPxLoc_, circleCurveHalfWidthPx);
    }

    if (circleCurveEdgeSoftnessPxLoc_ >= 0) {
        glUniform1f(circleCurveEdgeSoftnessPxLoc_, circleCurveEdgeSoftnessPx);
    }

    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,                                      // first vertex
        6,                                      // 6 vertices in quad
        static_cast<GLsizei>(instances.size())  // number of circles
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::renderRect(const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp) {
    if (!rectProgram_ || !rectVao_ || !rectQuadVbo_ || !rectInstanceVbo_) {
        return;
    }

    if (!scene.selectionRect.has_value()) {
        return;
    }

    const auto& r = *scene.selectionRect;

    float xMin = std::min(r.xMin, r.xMax);
    float xMax = std::max(r.xMin, r.xMax);
    float yMin = std::min(r.yMin, r.yMax);
    float yMax = std::max(r.yMin, r.yMax);

    RectInstance inst;
    inst.cx = 0.5f * (xMin + xMax);
    inst.cy = 0.5f * (yMin + yMax);
    inst.hx = 0.5f * (xMax - xMin);
    inst.hy = 0.5f * (yMax - yMin);

    inst.hx = std::max(inst.hx, 1e-6f);
    inst.hy = std::max(inst.hy, 1e-6f);

    glUseProgram(rectProgram_);
    glBindVertexArray(rectVao_);
    glBindBuffer(GL_ARRAY_BUFFER, rectInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(RectInstance),
        &inst,
        GL_DYNAMIC_DRAW
    );

    if (rectTransformLoc_ >= 0) {
        glUniformMatrix4fv(rectTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (rectFillColorLoc_ >= 0) {
        glUniform4f(rectFillColorLoc_, 0.2f, 0.5f, 1.0f, 0.12f);
    }

    // In future use 0.2f, 0.5f, 1.0f, 0.9f color for border

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::initGlobalState() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
}

bool OpenGLRenderer::initGridPipeline() {
    createProgramFromFiles("shaders/grid.vert", "shaders/grid.frag", gridProgram_);

    gridColorLoc_ = glGetUniformLocation(gridProgram_, "uGridColor");
    axisColorLoc_ = glGetUniformLocation(gridProgram_, "uAxisColor");
    gridZoomLoc_ = glGetUniformLocation(gridProgram_, "uZoom");
    gridInvViewProjLoc_ = glGetUniformLocation(gridProgram_, "uInvViewProj");

    gridCellSizeLoc_ = glGetUniformLocation(gridProgram_, "uCellSize");
    gridSubCellSizeLoc_ = glGetUniformLocation(gridProgram_, "uSubCellSize");

    const float quadVerts[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &gridVao_);
    glBindVertexArray(gridVao_);

    glGenBuffers(1, &gridQuadVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, gridQuadVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGLRenderer::initPointPipeline() {
    createProgramFromFiles("shaders/point.vert", "shaders/point.frag", pointProgram_);

    pointColorLoc_          = glGetUniformLocation(pointProgram_, "uColor");
    pointTransformLoc_      = glGetUniformLocation(pointProgram_, "uTransform");
    pointPadLoc_            = glGetUniformLocation(pointProgram_, "uPad");
    pointEdgeSoftnessLoc_   = glGetUniformLocation(pointProgram_, "uEdgeSoftness");

    const float quadVerts[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &pointVao_);
    glBindVertexArray(pointVao_);

    glGenBuffers(1, &pointQuadVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, pointQuadVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    // location = 0 : quad local pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), reinterpret_cast<void*>(0));

    glGenBuffers(1, &pointInstanceVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, pointInstanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // location = 1 : center
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        sizeof(PointInstance),
        reinterpret_cast<void*>(offsetof(PointInstance, x))
    );
    glVertexAttribDivisor(1, 1);

    // location = 2 : size
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 1, GL_FLOAT, GL_FALSE,
        sizeof(PointInstance),
        reinterpret_cast<void*>(offsetof(PointInstance, size))
    );
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGLRenderer::initLinePipeline() {
    createProgramFromFiles("shaders/line.vert", "shaders/line.frag", lineProgram_);

    lineColorLoc_           = glGetUniformLocation(lineProgram_, "uColor");
    lineAlphaLoc_           = glGetUniformLocation(lineProgram_, "uAlpha");
    lineTransformLoc_       = glGetUniformLocation(lineProgram_, "uTransform");
    linePadLoc_             = glGetUniformLocation(lineProgram_, "uPad");
    lineEdgeSoftnessLoc_    = glGetUniformLocation(lineProgram_, "uEdgeSoftness");

    const float quadVerts[] = {
        0.0f, -1.0f,
        0.0f,  1.0f,
        1.0f, -1.0f,

        0.0f,  1.0f,
        1.0f, -1.0f,
        1.0f,  1.0f
    };

    glGenVertexArrays(1, &lineVao_);
    glBindVertexArray(lineVao_);

    glGenBuffers(1, &lineQuadVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, lineQuadVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    // location = 0 : local quad position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    glGenBuffers(1, &lineInstanceVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, lineInstanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // location = 1 : p0
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(LineInstance),
        reinterpret_cast<void*>(offsetof(LineInstance, x1))
    );
    glVertexAttribDivisor(1, 1);

    // location = 2 : p1
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(LineInstance),
        reinterpret_cast<void*>(offsetof(LineInstance, x2))
    );
    glVertexAttribDivisor(2, 1);

    // location = 3 : halfWidth
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(LineInstance),
        reinterpret_cast<void*>(offsetof(LineInstance, halfWidthWorld))
    );
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGLRenderer::initCircleArcPipeline() {
    createProgramFromFiles("shaders/circle.vert", "shaders/circle.frag", circleProgram_);

    circleColorLoc_                 = glGetUniformLocation(circleProgram_, "uColor");
    circleTransformLoc_             = glGetUniformLocation(circleProgram_, "uTransform");
    circleZoomLoc_                  = glGetUniformLocation(circleProgram_, "uZoom");
    circleCurveHalfWidthPxLoc_      = glGetUniformLocation(circleProgram_, "uCurveHalfWidthPx");
    circleCurveEdgeSoftnessPxLoc_   = glGetUniformLocation(circleProgram_, "uEdgeSoftnessPx");

    const float quadVerts[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &circleVao_);
    glBindVertexArray(circleVao_);

    // static quad VBO
    glGenBuffers(1, &circleQuadVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, circleQuadVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    // location = 0 -> aQuadPos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // location
        2, // vec2
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    // instance VBO
    glGenBuffers(1, &circleInstanceVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, circleInstanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // location = 1 -> aCenter
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // location
        2, // vec2
        GL_FLOAT,
        GL_FALSE,
        sizeof(CircleArcInstance),
        reinterpret_cast<void*>(offsetof(CircleArcInstance, x))
    );
    glVertexAttribDivisor(1, 1);

    // location = 2 -> aRadius
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // location
        1, // float
        GL_FLOAT,
        GL_FALSE,
        sizeof(CircleArcInstance),
        reinterpret_cast<void*>(offsetof(CircleArcInstance, r))
    );
    glVertexAttribDivisor(2, 1);

    // location = 3 -> aStartAngle
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3, // location
        1, // float
        GL_FLOAT,
        GL_FALSE,
        sizeof(CircleArcInstance),
        reinterpret_cast<void*>(offsetof(CircleArcInstance, startAngle))
    );
    glVertexAttribDivisor(3, 1);

    // location = 4 -> aEndAngle
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(
        4, // location
        1, // float
        GL_FLOAT,
        GL_FALSE,
        sizeof(CircleArcInstance),
        reinterpret_cast<void*>(offsetof(CircleArcInstance, endAngle))
    );
    glVertexAttribDivisor(4, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGLRenderer::initRectPipeline() {
    if (!createProgramFromFiles("shaders/rect.vert", "shaders/rect.frag", rectProgram_)) {
        return false;
    }

    rectTransformLoc_ = glGetUniformLocation(rectProgram_, "uTransform");
    rectFillColorLoc_ = glGetUniformLocation(rectProgram_, "uFillColor");

    const float quadVerts[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    glGenVertexArrays(1, &rectVao_);
    glBindVertexArray(rectVao_);

    glGenBuffers(1, &rectQuadVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, rectQuadVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    // location = 0 : quad local pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float),
        reinterpret_cast<void*>(0)
    );

    glGenBuffers(1, &rectInstanceVbo_);
    glBindBuffer(GL_ARRAY_BUFFER, rectInstanceVbo_);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // location = 1 : center
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        sizeof(RectInstance),
        reinterpret_cast<void*>(offsetof(RectInstance, cx))
    );
    glVertexAttribDivisor(1, 1);

    // location = 2 : half extents
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE,
        sizeof(RectInstance),
        reinterpret_cast<void*>(offsetof(RectInstance, hx))
    );
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

GLuint OpenGLRenderer::compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetShaderiv(s, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(s, len, nullptr, log.data());
        std::fprintf(stderr, "Shader compile error:\n%s\n", log.c_str());
        glDeleteShader(s);
        return 0;
    }
    return s;
}

bool OpenGLRenderer::checkProgramLink(GLuint prog)
{
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(prog, len, nullptr, log.data());
        std::fprintf(stderr, "Program link error:\n%s\n", log.c_str());
        return false;
    }
    return true;
}

bool OpenGLRenderer::createProgramFromFiles(const char* vertPath, const char* fragPath, GLuint& outProgram) {
    std::string vertexSource = ShaderUtils::readFile(vertPath);
    std::string fragmentSource = ShaderUtils::readFile(fragPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        return false;
    }

    int vs = compileShader(GL_VERTEX_SHADER, vertexSource.c_str());
    if (!vs) {
        return false;
    }

    int fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
    if (!fs) {
        glDeleteShader(vs);
        return false;
    }

    outProgram = glCreateProgram();
    glAttachShader(outProgram, vs);
    glAttachShader(outProgram, fs);
    glLinkProgram(outProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!checkProgramLink(outProgram)) {
        glDeleteProgram(outProgram);
        outProgram = 0;
        return false;
    }

    return true;
}

void OpenGLRenderer::initRenderText() {
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        // ... an error occurred during library initialization ...
        return;
    }

    // error = FT_New_Face(library,
    //                     "fonts/B612/B612-Regular.ttf",
    //                     0,
    //                     &face);
    // std::cout << face->num_faces << '\n';

    error = FT_New_Face(library,
                        "fonts/B612/B612-Regular.ttf",
                        0,
                        &face);
    if (error == FT_Err_Unknown_File_Format) {
        // ... the font file could be opened and read, but it appears
        // ... that its font format is unsupported
        return;
    } else if (error) {
        // ... another error code means that the font file could not
        // ... be opened or read, or that it is broken...
        return;
    }

    error = FT_Set_Char_Size(
          face,    /* handle to face object         */
          0,       /* char_width in 1/64 of points  */
          5*64,   /* char_height in 1/64 of points */
          300,     /* horizontal device resolution  */
          300 );   /* vertical device resolution    */

    // error = FT_Set_Pixel_Sizes(
    //     face,       // объект шрифта
    //     0,          // ширина в px, 0 = авто
    //     24          // высота в px
    // );

    if (error) {
        // ... an error occurred ...
        return;
    }


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (unsigned char c = 0; c < 128; c++)
    {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    glGenVertexArrays(1, &textVao_);
    glGenBuffers(1, &textQuadVbo_);
    glBindVertexArray(textVao_);
    glBindBuffer(GL_ARRAY_BUFFER, textQuadVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    if (!createProgramFromFiles("shaders/text.vert", "shaders/text.frag", textProgram_)) {
        std::cout << "ERROR::FREETYPE: Failed create program from files" << std::endl;
    }

    textTransformLoc_ = glGetUniformLocation(textProgram_, "uTransform");
}

void OpenGLRenderer::renderText(const rendering::text::TextObject& textObj) {
    if (!textProgram_) {
        std::cout << "Program not init!\n";
        return;
    }

    // activate corresponding render state
    glUseProgram(textProgram_);
    glUniform3f(glGetUniformLocation(textProgram_, "textColor"), textObj.style.r, textObj.style.g, textObj.style.b);

    glm::mat4 ortho = glm::ortho(0.0f, (float)width_, 0.0f, (float)height_);
    if (textTransformLoc_ >= 0) {
        glUniformMatrix4fv(textTransformLoc_, 1, GL_FALSE, glm::value_ptr(ortho));
    }

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVao_);

    const std::string& text = textObj.utf8Text;

    float textWidth = 0.0f;
    float textAscent = 0.0f;
    float textDescent = 0.0f;


    for (char c : text) {
        const Character& ch = Characters.at(c);
        textWidth += (ch.Advance >> 6);
        if (ch.Bearing.y > textAscent) textAscent = ch.Bearing.y;
        float depth = ch.Size.y - ch.Bearing.y;
        if (depth > textDescent) textDescent = depth;
    }

    using namespace rendering::text;

    double x = textObj.placement.screen.anchorPx.x;
    switch (textObj.style.hAlign) {
        case TextHorizontalAlign::Left:
            break;
        case TextHorizontalAlign::Center:
            x -= textWidth * 0.5f;
            break;
        case TextHorizontalAlign::Right:
            x -= textWidth;
            break;
    }

    double y = textObj.placement.screen.anchorPx.y;
    switch (textObj.style.vAlign) {
        case TextVerticalAlign::Top:
            y -= textAscent;
            break;
        case TextVerticalAlign::Middle:
            y -= (textAscent + textDescent) * 0.5f;
            break;
        case TextVerticalAlign::Baseline:
            // y = anchorPx.y - baseline и есть anchor
            break;
        case TextVerticalAlign::Bottom:
            y += textDescent;
            break;
    }

    for (char c : text) {
        Character ch = Characters.at(c);

        float xpos = x + ch.Bearing.x;
        float ypos = y - (ch.Size.y - ch.Bearing.y);

        float w = ch.Size.x;
        float h = ch.Size.y;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, textQuadVbo_);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}


