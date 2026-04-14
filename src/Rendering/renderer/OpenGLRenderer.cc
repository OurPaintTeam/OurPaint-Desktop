#include "OpenGLRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera2D.h"
#include "RenderData.h"
#include "shaders/shader_utils.h"

using namespace renderer;

namespace {
struct PointInstance {
    float x;
    float y;
    float size;
};
struct CircleInstance {
    float x;
    float y;
    float r;
};
struct LineInstance {
    float x1;
    float y1;
    float x2;
    float y2;
    float halfWidthWorld;
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
    if (!initCirclePipeline()) {
        return false;
    }

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
    renderPoints(rd, camera, mvp);
    renderLines(rd, camera, mvp);
    renderCircles(rd, camera, mvp);
}

void OpenGLRenderer::renderGrid(const RenderData& scene, const Camera2D& camera, const glm::mat4& mvp) {
    if (!gridProgram_ || !gridVao_ || !gridQuadVbo_) {
        return;
    }

    //std::cout << "Camera center: " << camera.center().x << ' ' << camera.center().y << ", zoom: " << camera.zoom() << '\n';

    glUseProgram(gridProgram_);

    glm::mat4 viewProj = camera.viewProjectionMatrix();
    glm::mat4 invViewProj = glm::inverse(viewProj);

    double zoom = camera.zoom() / 100.0;
    double zoomLevel = std::log2(zoom);
    double zoomFactor = std::pow(2.0, std::floor(zoomLevel));

    double cellSize = 1.0 / zoomFactor;
    double subCellSize = cellSize / 5.0;

    double camX = camera.center().x;
    double camY = camera.center().y;

    double originX = std::floor(camX / cellSize) * cellSize;
    double originY = std::floor(camY / cellSize) * cellSize;

    glUniform1f(gridCellSizeLoc_, static_cast<float>(cellSize));
    glUniform1f(gridSubCellSizeLoc_, static_cast<float>(subCellSize));
    glUniform2f(gridOriginLoc_, static_cast<float>(originX), static_cast<float>(originY));

    if (gridColorLoc_ >= 0) {
        glUniform3f(gridColorLoc_, 0.5f, 0.5f, 0.5f);
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

    const size_t count = scene.points.size() + scene.overlay.points.size();
    if (count == 0) {
        return;
    }

    float worldPerPixel = 1.0f / camera.zoom();
    float pointSizeWorld = pointRadiusPx * worldPerPixel;
    pointRadiusPx = glm::max(pointRadiusPx, 1e-6f);
    float edgeSoftness = pointEdgeSoftnessPx / pointRadiusPx;
    float pointPad = 1.0f + edgeSoftness;

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

    if (pointColorLoc_ >= 0) {
        glUniform3f(pointColorLoc_, 0.0f, 0.0f, 0.0f);
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

    const size_t count = scene.lines.size() + scene.overlay.lines.size();
    if (count == 0) {
        return;
    }

    float worldPerPixel = 1.0f / camera.zoom();
    float lineHalfWidthWorld = lineHalfWidthPx * worldPerPixel;
    lineHalfWidthPx = glm::max(lineHalfWidthPx, 1e-6f);
    float lineEdgeSoftness = lineEdgeSoftnessPx / lineHalfWidthPx;
    float linePad = 1.0f + lineEdgeSoftness;

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

    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(instances.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::renderCircles(const RenderData& renderData, const Camera2D& camera, const glm::mat4& mvp) {
    if (!circleProgram_ || !circleVao_ || !circleQuadVbo_ || !circleInstanceVbo_) {
        return;
    }

    //float radiusPx = c.r * camera.zoom()

    size_t circlesCount = renderData.circles.size() + renderData.overlay.circles.size();
    if (circlesCount == 0) {
        return;
    }

    std::vector<CircleInstance> instances;
    instances.reserve(circlesCount);

    for (const auto& c : renderData.overlay.circles) {
        instances.push_back(CircleInstance{ c.x, c.y, c.r });
    }

    for (const auto& c : renderData.circles) {
        instances.push_back(CircleInstance{ c.x, c.y, c.r });
    }

    glUseProgram(circleProgram_);
    glBindVertexArray(circleVao_);
    glBindBuffer(GL_ARRAY_BUFFER, circleInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instances.size() * sizeof(CircleInstance)),
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

void OpenGLRenderer::initGlobalState() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
}

bool OpenGLRenderer::initGridPipeline() {
    createProgramFromFiles("shaders/grid.vert", "shaders/grid.frag", gridProgram_);

    gridColorLoc_ = glGetUniformLocation(gridProgram_, "uColor");
    gridZoomLoc_ = glGetUniformLocation(gridProgram_, "uZoom");
    gridInvViewProjLoc_ = glGetUniformLocation(gridProgram_, "uInvViewProj");

    gridCellSizeLoc_ = glGetUniformLocation(gridProgram_, "uCellSize");
    gridSubCellSizeLoc_ = glGetUniformLocation(gridProgram_, "uSubCellSize");
    gridOriginLoc_ = glGetUniformLocation(gridProgram_, "uGridOrigin");

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

bool OpenGLRenderer::initCirclePipeline() {
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
        1,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(CircleInstance),
        reinterpret_cast<void*>(offsetof(CircleInstance, x))
    );
    glVertexAttribDivisor(1, 1);

    // location = 2 -> aRadius
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(CircleInstance),
        reinterpret_cast<void*>(offsetof(CircleInstance, r))
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


