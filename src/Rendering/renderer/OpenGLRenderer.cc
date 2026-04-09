#include "OpenGLRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera2D.h"
#include "shaders/shader_utils.h"
#include "RenderData.h"

using namespace renderer;

bool OpenGLRenderer::initialize() {
    initGlobalState();

    if (!initializePrimitivePipeline()) {
        return false;
    }
    if (!initializeCirclePipeline()) {
        return false;
    }

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    return true;
}

void OpenGLRenderer::resize(int w, int h)
{
    width_ = w;
    height_ = h;
    glViewport(0, 0, w, h);
}

void OpenGLRenderer::shutdown() {

    // Primitive states
    if (primitiveVbo_) {
        glDeleteBuffers(1, &primitiveVbo_);
    }
    if (primitiveVao_) {
        glDeleteVertexArrays(1, &primitiveVao_);
    }
    if (primitiveProgram_) {
        glDeleteProgram(primitiveProgram_);
    }

    primitiveVbo_ = 0;
    primitiveVao_ = 0;
    primitiveProgram_ = 0;
    primitiveColorLoc_ = -1;

    // Circle states
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
}

void OpenGLRenderer::render(const RenderData& rd, const Camera2D& camera)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 mvp = camera.viewProjectionMatrix();

    renderPointsAndLines(rd, mvp);
    renderCircles(rd, mvp);
}

void OpenGLRenderer::renderPointsAndLines(const RenderData& rd, const glm::mat4& mvp) {
    if (!primitiveProgram_ || !primitiveVao_ || !primitiveVbo_) {
        return;
    }

    if (rd.points.empty() && rd.lines.empty() && rd.overlay.empty()) {
        return;
    }

    size_t pointsCount = rd.points.size() + rd.overlay.points.size();
    size_t linesCount = rd.lines.size() + rd.overlay.lines.size();

    std::vector<float> verts;
    verts.reserve(pointsCount * 2 + linesCount * 4);

    // Points
    for (const auto& p : rd.points) {
        verts.push_back(p.x);
        verts.push_back(p.y);
    }
    for (const auto& p : rd.overlay.points) {
        verts.push_back(p.x);
        verts.push_back(p.y);
    }

    // Lines
    for (const auto& l : rd.lines) {
        verts.push_back(l.x1);
        verts.push_back(l.y1);
        verts.push_back(l.x2);
        verts.push_back(l.y2);
    }
    for (const auto& l : rd.overlay.lines) {
        verts.push_back(l.x1);
        verts.push_back(l.y1);
        verts.push_back(l.x2);
        verts.push_back(l.y2);
    }


    glUseProgram(primitiveProgram_);

    if (primitiveTransformLoc_ >= 0) {
        glUniformMatrix4fv(primitiveTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (primitiveColorLoc_ >= 0) {
        glUniform3f(primitiveColorLoc_, 0.0f, 0.0f, 0.0f);
    }

    glBindVertexArray(primitiveVao_);
    glBindBuffer(GL_ARRAY_BUFFER, primitiveVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(verts.size() * sizeof(float)),
        verts.data(),
        GL_DYNAMIC_DRAW
    );

    glPointSize(4.0f);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(pointsCount));
    glDrawArrays(GL_LINES, static_cast<GLint>(pointsCount), static_cast<GLsizei>(linesCount * 2));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void OpenGLRenderer::renderCircles(const RenderData& renderData, const glm::mat4& mvp) {
    if (!circleProgram_ || !circleVao_ || !circleQuadVbo_ || !circleInstanceVbo_) {
        return;
    }

    size_t circlesCount = renderData.circles.size() + renderData.overlay.circles.size();
    if (circlesCount == 0) {
        return;
    }

    std::vector<Circle> instances;
    instances.reserve(circlesCount);

    for (const auto& c : renderData.overlay.circles) {
        instances.push_back(Circle{ c.x, c.y, c.r });
    }

    for (const auto& c : renderData.circles) {
        instances.push_back(Circle{ c.x, c.y, c.r });
    }

    glUseProgram(circleProgram_);
    glBindVertexArray(circleVao_);
    glBindBuffer(GL_ARRAY_BUFFER, circleInstanceVbo_);

    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(instances.size() * sizeof(Circle)),
        instances.data(),
        GL_DYNAMIC_DRAW
    );

    if (circleTransformLoc_ >= 0) {
        glUniformMatrix4fv(circleTransformLoc_, 1, GL_FALSE, glm::value_ptr(mvp));
    }

    if (circleColorLoc_ >= 0) {
        glUniform3f(circleColorLoc_, 0.0f, 0.0f, 0.0f);
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
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

bool OpenGLRenderer::initializePrimitivePipeline() {
    std::string vertexSource = ShaderUtils::readFile("shaders/shader.vert");
    std::string fragmentSource = ShaderUtils::readFile("shaders/shader.frag");

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

    primitiveProgram_ = glCreateProgram();
    glAttachShader(primitiveProgram_, vs);
    glAttachShader(primitiveProgram_, fs);
    glLinkProgram(primitiveProgram_);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!checkProgramLink(primitiveProgram_)) {
        glDeleteProgram(primitiveProgram_);
        primitiveProgram_ = 0;
        return false;
    }

    primitiveColorLoc_ = glGetUniformLocation(primitiveProgram_, "uColor");
    primitiveTransformLoc_ = glGetUniformLocation(primitiveProgram_, "uTransform");



    glGenVertexArrays(1, &primitiveVao_);
    glGenBuffers(1, &primitiveVbo_);

    glBindVertexArray(primitiveVao_);
    glBindBuffer(GL_ARRAY_BUFFER, primitiveVbo_);

    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (void*)0
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

bool OpenGLRenderer::initializeCirclePipeline() {
    std::string vertexSource = ShaderUtils::readFile("shaders/circle.vert");
    std::string fragmentSource = ShaderUtils::readFile("shaders/circle.frag");

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

    circleProgram_ = glCreateProgram();
    glAttachShader(circleProgram_, vs);
    glAttachShader(circleProgram_, fs);
    glLinkProgram(circleProgram_);

    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!checkProgramLink(circleProgram_)) {
        glDeleteProgram(circleProgram_);
        circleProgram_ = 0;
        return false;
    }

    circleColorLoc_ = glGetUniformLocation(circleProgram_, "uColor");
    circleTransformLoc_ = glGetUniformLocation(circleProgram_, "uTransform");



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
        sizeof(Circle),
        reinterpret_cast<void*>(offsetof(Circle, x))
    );
    glVertexAttribDivisor(1, 1);

    // location = 2 -> aRadius
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Circle),
        reinterpret_cast<void*>(offsetof(Circle, r))
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


