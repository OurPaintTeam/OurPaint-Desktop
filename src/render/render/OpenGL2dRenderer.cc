#include "OpenGL2dRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera2D.h"
#include "RenderScene.h"
#include "shaders/shader_utils.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library  library;   /* handle to library     */
FT_Face     face;      /* handle to face object */

using namespace render;

namespace {

constexpr float kMinPixelSize = 1e-6f;

// Keep the old shader convention for now:
// startAngle == 0 and endAngle == 0 means "full circle" in the existing circle shader.
constexpr float kCircleStartAngle = 0.0f;
constexpr float kCircleEndAngle = 0.0f;

struct MarkerInstance {
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
    float halfWidth;
};

struct RectInstance {
    float cx;
    float cy;
    float hx;
    float hy;
};

float unitsPerPixel(CoordinateSpace coordinateSpace, const Camera2D& camera) {
    if (coordinateSpace == CoordinateSpace::World) {
        return 1.0f / camera.zoom();
    }

    return 1.0f;
}

} // namespace

bool OpenGL2dRenderer::initialize() {
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

void OpenGL2dRenderer::resize(int w, int h) {
    width_ = std::max(w, 1);
    height_ = std::max(h, 1);

    glViewport(0, 0, width_, height_);
}

void OpenGL2dRenderer::shutdown() {
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

void OpenGL2dRenderer::render(const RenderScene& scene, const Camera2D& camera) {
    glClear(GL_COLOR_BUFFER_BIT);

    renderGrid(scene, camera);
    renderLayers(scene, camera);
}

void OpenGL2dRenderer::renderGrid(const RenderScene& scene, const Camera2D& camera) {
    if (!scene.grid.has_value()) {
        return;
    }

    if (!gridProgram_ || !gridVao_ || !gridQuadVbo_) {
        return;
    }

    const Grid& grid = *scene.grid;

    glUseProgram(gridProgram_);

    const glm::mat4 viewProj = camera.viewProjectionMatrix();
    const glm::mat4 invViewProj = glm::inverse(viewProj);

    if (gridCellSizeLoc_ >= 0) {
        glUniform1f(gridCellSizeLoc_, grid.cellSize);
    }

    if (gridSubCellSizeLoc_ >= 0) {
        glUniform1f(gridSubCellSizeLoc_, grid.subCellSize);
    }

    if (gridColorLoc_ >= 0) {
        glUniform3f(
            gridColorLoc_,
            grid.minorColor.r,
            grid.minorColor.g,
            grid.minorColor.b
        );
    }

    if (axisColorLoc_ >= 0) {
        glUniform3f(
            axisColorLoc_,
            grid.axisColor.r,
            grid.axisColor.g,
            grid.axisColor.b
        );
    }

    if (gridZoomLoc_ >= 0) {
        glUniform1f(gridZoomLoc_, camera.zoom());
    }

    if (gridInvViewProjLoc_ >= 0) {
        glUniformMatrix4fv(
            gridInvViewProjLoc_,
            1,
            GL_FALSE,
            glm::value_ptr(invViewProj)
        );
    }

    glBindVertexArray(gridVao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGL2dRenderer::renderLayers(const RenderScene& scene, const Camera2D& camera) {
    std::vector<const DrawLayer*> layers;
    layers.reserve(scene.layers.size());

    for (const DrawLayer& layer : scene.layers) {
        if (!layer.empty()) {
            layers.push_back(&layer);
        }
    }

    std::stable_sort(
        layers.begin(),
        layers.end(),
        [](const DrawLayer* a, const DrawLayer* b) {
            return a->order < b->order;
        }
    );

    for (const DrawLayer* layer : layers) {
        renderLayer(*layer, camera);
    }
}

void OpenGL2dRenderer::renderLayer(const DrawLayer& layer, const Camera2D& camera) {
    const glm::mat4 transform = transformFor(layer.coordinateSpace, camera);

    for (const MarkerBatch& batch : layer.markerBatches) {
        renderMarkerBatch(batch, layer.coordinateSpace, camera, transform);
    }

    for (const LineBatch& batch : layer.lineBatches) {
        renderLineBatch(batch, layer.coordinateSpace, camera, transform);
    }

    for (const CircleBatch& batch : layer.circleBatches) {
        renderCircleBatch(batch, camera, transform);
    }

    for (const ArcBatch& batch : layer.arcBatches) {
        renderArcBatch(batch, camera, transform);
    }

    for (const RectBatch& batch : layer.rectBatches) {
        renderRectBatch(batch, camera, transform);
    }

    for (const TextBatch& batch : layer.textBatches) {
        for (const text::TextObject& textObject : batch.textObjects) {
            renderText(textObject);
        }
    }
}

glm::mat4 OpenGL2dRenderer::transformFor(CoordinateSpace coordinateSpace, const Camera2D& camera) const {
    if (coordinateSpace == CoordinateSpace::World) {
        return camera.viewProjectionMatrix();
    }

    return glm::ortho(
        0.0f,
        static_cast<float>(width_),
        0.0f,
        static_cast<float>(height_)
    );
}

void OpenGL2dRenderer::renderMarkerBatch(
    const MarkerBatch& batch,
    CoordinateSpace coordinateSpace,
    const Camera2D& camera,
    const glm::mat4& transform
) {
    if (batch.markers.empty()) {
        return;
    }

    if (!pointProgram_ || !pointVao_ || !pointQuadVbo_ || !pointInstanceVbo_) {
        return;
    }

    const float radiusPx = std::max(batch.style.radius, kMinPixelSize);
    const float markerSize = radiusPx * unitsPerPixel(coordinateSpace, camera);

    const float edgeSoftnessPx = batch.style.edgeSoftnessPx.value_or(pointEdgeSoftnessPx_);
    const float edgeSoftness = edgeSoftnessPx / radiusPx;
    const float pointPad = 1.0f + edgeSoftness;

    std::vector<MarkerInstance> instances;
    instances.reserve(batch.markers.size());

    for (const Marker& marker : batch.markers) {
        instances.push_back({marker.x, marker.y, markerSize});
    }

    glUseProgram(pointProgram_);
    glBindVertexArray(pointVao_);
    glBindBuffer(GL_ARRAY_BUFFER, pointInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instances.size() * sizeof(MarkerInstance)),
        instances.data(),
        GL_DYNAMIC_DRAW
    );

    if (pointTransformLoc_ >= 0) {
        glUniformMatrix4fv(
            pointTransformLoc_,
            1,
            GL_FALSE,
            glm::value_ptr(transform)
        );
    }

    if (pointColorLoc_ >= 0) {
        glUniform3f(
            pointColorLoc_,
            batch.style.color.r,
            batch.style.color.g,
            batch.style.color.b
        );
    }

    if (pointPadLoc_ >= 0) {
        glUniform1f(pointPadLoc_, pointPad);
    }

    if (pointEdgeSoftnessLoc_ >= 0) {
        glUniform1f(pointEdgeSoftnessLoc_, edgeSoftness);
    }

    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,
        6,
        static_cast<GLsizei>(instances.size())
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGL2dRenderer::renderLineBatch(
    const LineBatch& batch,
    CoordinateSpace coordinateSpace,
    const Camera2D& camera,
    const glm::mat4& transform) {

    if (batch.lines.empty()) {
        return;
    }

    if (!lineProgram_ || !lineVao_ || !lineQuadVbo_ || !lineInstanceVbo_) {
        return;
    }

    const float halfWidthPx = std::max(batch.style.widthPx * 0.5f, kMinPixelSize);
    const float halfWidth = halfWidthPx * unitsPerPixel(coordinateSpace, camera);

    const float edgeSoftnessPx = batch.style.edgeSoftnessPx.value_or(lineEdgeSoftnessPx_);
    const float edgeSoftness = edgeSoftnessPx / halfWidthPx;
    const float linePad = 1.0f + edgeSoftness;

    std::vector<LineInstance> instances;
    instances.reserve(batch.lines.size());

    for (const Line& line : batch.lines) {
        instances.push_back({
            line.x1,
            line.y1,
            line.x2,
            line.y2,
            halfWidth
        });
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
        glUniformMatrix4fv(
            lineTransformLoc_,
            1,
            GL_FALSE,
            glm::value_ptr(transform)
        );
    }

    if (lineColorLoc_ >= 0) {
        glUniform3f(
            lineColorLoc_,
            batch.style.color.r,
            batch.style.color.g,
            batch.style.color.b
        );
    }

    if (lineAlphaLoc_ >= 0) {
        glUniform1f(lineAlphaLoc_, batch.style.color.a);
    }

    if (linePadLoc_ >= 0) {
        glUniform1f(linePadLoc_, linePad);
    }

    if (lineEdgeSoftnessLoc_ >= 0) {
        glUniform1f(lineEdgeSoftnessLoc_, edgeSoftness);
    }

    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,
        6,
        static_cast<GLsizei>(instances.size())
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void OpenGL2dRenderer::renderCircleBatch(
    const CircleBatch& batch,
    const Camera2D& camera,
    const glm::mat4& transform
) {
    if (batch.circles.empty()) {
        return;
    }

    if (!circleProgram_ || !circleVao_ || !circleQuadVbo_ || !circleInstanceVbo_) {
        return;
    }

    // Current circle shader renders stroked circles/arcs.
    // batch.fill is intentionally not used here yet.
    const float halfWidthPx = std::max(batch.stroke.widthPx * 0.5f, kMinPixelSize);

    std::vector<CircleArcInstance> instances;
    instances.reserve(batch.circles.size());

    for (const Circle& circle : batch.circles) {
        instances.push_back({
            circle.x,
            circle.y,
            circle.r,
            kCircleStartAngle,
            kCircleEndAngle
        });
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
        glUniformMatrix4fv(
            circleTransformLoc_,
            1,
            GL_FALSE,
            glm::value_ptr(transform)
        );
    }

    if (circleColorLoc_ >= 0) {
        glUniform3f(
            circleColorLoc_,
            batch.stroke.color.r,
            batch.stroke.color.g,
            batch.stroke.color.b
        );
    }

    if (circleZoomLoc_ >= 0) {
        glUniform1f(circleZoomLoc_, camera.zoom());
    }

    if (circleCurveHalfWidthPxLoc_ >= 0) {
        glUniform1f(circleCurveHalfWidthPxLoc_, halfWidthPx);
    }

    const float edgeSoftnessPx = batch.stroke.edgeSoftnessPx.value_or(circleCurveEdgeSoftnessPx_);
    if (circleCurveEdgeSoftnessPxLoc_ >= 0) {
        glUniform1f(circleCurveEdgeSoftnessPxLoc_, edgeSoftnessPx);
    }

    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,
        6,
        static_cast<GLsizei>(instances.size())
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}


void OpenGL2dRenderer::renderArcBatch(
    const ArcBatch& batch,
    const Camera2D& camera,
    const glm::mat4& transform
) {
    if (batch.arcs.empty()) {
        return;
    }

    if (!circleProgram_ || !circleVao_ || !circleQuadVbo_ || !circleInstanceVbo_) {
        return;
    }

    const float halfWidthPx = std::max(batch.stroke.widthPx * 0.5f, kMinPixelSize);

    std::vector<CircleArcInstance> instances;
    instances.reserve(batch.arcs.size());

    for (const Arc& arc : batch.arcs) {
        instances.push_back({
            arc.x,
            arc.y,
            arc.r,
            arc.startAngle,
            arc.endAngle
        });
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
        glUniformMatrix4fv(
            circleTransformLoc_,
            1,
            GL_FALSE,
            glm::value_ptr(transform)
        );
    }

    if (circleColorLoc_ >= 0) {
        glUniform3f(
            circleColorLoc_,
            batch.stroke.color.r,
            batch.stroke.color.g,
            batch.stroke.color.b
        );
    }

    if (circleZoomLoc_ >= 0) {
        glUniform1f(circleZoomLoc_, camera.zoom());
    }

    if (circleCurveHalfWidthPxLoc_ >= 0) {
        glUniform1f(circleCurveHalfWidthPxLoc_, halfWidthPx);
    }

    const float edgeSoftnessPx = batch.stroke.edgeSoftnessPx.value_or(circleCurveEdgeSoftnessPx_);
    if (circleCurveEdgeSoftnessPxLoc_ >= 0) {
        glUniform1f(circleCurveEdgeSoftnessPxLoc_, edgeSoftnessPx);
    }

    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,
        6,
        static_cast<GLsizei>(instances.size())
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGL2dRenderer::renderRectBatch(
    const RectBatch& batch,
    const Camera2D& camera,
    const glm::mat4& transform
) {
    if (batch.rects.empty()) {
        return;
    }

    if (!rectProgram_ || !rectVao_ || !rectQuadVbo_ || !rectInstanceVbo_) {
        return;
    }

    if (!batch.fill.has_value() && !batch.stroke.has_value()) {
        return;
    }

    std::vector<RectInstance> instances;
    instances.reserve(batch.rects.size());

    for (const Rect& rect : batch.rects) {
        const float xMin = std::min(rect.xMin, rect.xMax);
        const float xMax = std::max(rect.xMin, rect.xMax);
        const float yMin = std::min(rect.yMin, rect.yMax);
        const float yMax = std::max(rect.yMin, rect.yMax);

        RectInstance instance{};
        instance.cx = 0.5f * (xMin + xMax);
        instance.cy = 0.5f * (yMin + yMax);
        instance.hx = std::max(0.5f * (xMax - xMin), kMinPixelSize);
        instance.hy = std::max(0.5f * (yMax - yMin), kMinPixelSize);

        instances.push_back(instance);
    }

    glUseProgram(rectProgram_);
    glBindVertexArray(rectVao_);
    glBindBuffer(GL_ARRAY_BUFFER, rectInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instances.size() * sizeof(RectInstance)),
        instances.data(),
        GL_DYNAMIC_DRAW
    );

    if (rectHasFillLoc_ >= 0) {
        glUniform1i(rectHasFillLoc_, batch.fill.has_value() ? 1 : 0);
    }

    if (rectHasStrokeLoc_ >= 0) {
        glUniform1i(rectHasStrokeLoc_, batch.stroke.has_value() ? 1 : 0);
    }

    if (rectTransformLoc_ >= 0) {
        glUniformMatrix4fv(rectTransformLoc_, 1, GL_FALSE, glm::value_ptr(transform));
    }

    if (batch.fill.has_value()) {
        const Color& fill = *batch.fill;

        glUniform4f(
            rectFillColorLoc_,
            fill.r,
            fill.g,
            fill.b,
            fill.a
        );
    }

    if (batch.stroke.has_value()) {
        const StrokeStyle& stroke = *batch.stroke;

        glUniform4f(
            rectStrokeColorLoc_,
            stroke.color.r,
            stroke.color.g,
            stroke.color.b,
            stroke.color.a
        );

        glUniform1f(
            rectStrokeWidthPxLoc_,
            stroke.widthPx * camera.viewport().devicePixelRatio
        );
    }


    glDrawArraysInstanced(
        GL_TRIANGLES,
        0,
        6,
        static_cast<GLsizei>(instances.size())
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGL2dRenderer::initGlobalState() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
}

bool OpenGL2dRenderer::initGridPipeline() {
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

bool OpenGL2dRenderer::initPointPipeline() {
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
        sizeof(MarkerInstance),
        reinterpret_cast<void*>(offsetof(MarkerInstance, x))
    );
    glVertexAttribDivisor(1, 1);

    // location = 2 : size
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 1, GL_FLOAT, GL_FALSE,
        sizeof(MarkerInstance),
        reinterpret_cast<void*>(offsetof(MarkerInstance, size))
    );
    glVertexAttribDivisor(2, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGL2dRenderer::initLinePipeline() {
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
        reinterpret_cast<void*>(offsetof(LineInstance, halfWidth))
    );
    glVertexAttribDivisor(3, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGL2dRenderer::initCircleArcPipeline() {
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

bool OpenGL2dRenderer::initRectPipeline() {
    if (!createProgramFromFiles("shaders/rect.vert", "shaders/rect.frag", rectProgram_)) {
        return false;
    }

    rectTransformLoc_ = glGetUniformLocation(rectProgram_, "uTransform");
    rectFillColorLoc_ = glGetUniformLocation(rectProgram_, "uFillColor");
    rectStrokeColorLoc_ = glGetUniformLocation(rectProgram_, "uStrokeColor");
    rectHasFillLoc_ = glGetUniformLocation(rectProgram_, "uHasFill");
    rectHasStrokeLoc_ = glGetUniformLocation(rectProgram_, "uHasStroke");
    rectStrokeWidthPxLoc_ = glGetUniformLocation(rectProgram_, "uStrokeWidthPx");

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

GLuint OpenGL2dRenderer::compileShader(GLenum type, const char* src)
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

bool OpenGL2dRenderer::checkProgramLink(GLuint prog)
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

bool OpenGL2dRenderer::createProgramFromFiles(const char* vertPath, const char* fragPath, GLuint& outProgram) {
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

void OpenGL2dRenderer::initRenderText() {
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
          3*64,   /* char_height in 1/64 of points */
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

void OpenGL2dRenderer::renderText(const text::TextObject& textObj) {
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

    using namespace render::text;

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


