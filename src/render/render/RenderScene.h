#ifndef OURPAINT_RENDER_RENDERSCENE_H_
#define OURPAINT_RENDER_RENDERSCENE_H_

#include <optional>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "text/Types.h"

/*
 * RenderScene is a backend-independent, frame-local description of a visual scene.
 *
 * This file defines WHAT should be rendered, not HOW it should be rendered.
 *
 * RenderScene is the contract between scene builders and rendering backends:
 *
 *   CAD / FEA / UI / other builders
 *              ->
 *        render::RenderScene
 *              ->
 *   OpenGL / Vulkan / WebGPU / Software / other renderers
 *
 * RenderScene must contain only generic visual primitives, styles, layers,
 * coordinate-space information, and other data that can be interpreted by
 * different rendering backends.
 *
 * Do not add backend-specific concepts here:
 *   - OpenGL / Vulkan objects
 *   - GPU buffers
 *   - descriptor sets
 *   - shader programs
 *   - pipelines
 *
 * Do not add domain-specific concepts here:
 *   - CAD entities
 *   - FEA elements
 *   - CFD fields
 *   - UI widgets
 *   - solver objects
 *   - application commands
 *
 * Domain-specific code should convert its own model into RenderScene through
 * a dedicated builder. Rendering backends should only consume RenderScene and
 * should not know where the primitives came from.
 *
 * Before adding a new field or primitive, ask:
 *
 *   Is this a generic visual concept that multiple backends can render?
 *
 * If not, it does not belong in RenderScene.
 */

namespace render {

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

struct Marker {
    float x{};
    float y{};
};

struct Line {
    float x1{};
    float y1{};
    float x2{};
    float y2{};
};

struct Circle {
    float x = 0.0f;
    float y = 0.0f;
    float r = 1.0f;
};

struct Arc {
    float x = 0.0f;
    float y = 0.0f;
    float r = 1.0f;

    // Radians.
    float startAngle = 0.0f;
    float endAngle = 0.0f;
};

struct Rect {
    float xMin{};
    float yMin{};
    float xMax{};
    float yMax{};
};

struct StrokeStyle {
    Color color{};
    float width = 1.0f;
    std::optional<float> edgeSoftnessPx;
};

struct MarkerStyle {
    Color color{};
    float radius = 3.0f;
    std::optional<float> edgeSoftnessPx;
};

struct MarkerBatch {
    std::vector<Marker> markers;
    MarkerStyle style;
};

struct LineBatch {
    std::vector<Line> lines;
    StrokeStyle style;
};

struct CircleBatch {
    std::vector<Circle> circles;
    StrokeStyle stroke;
    std::optional<Color> fill;
};

struct ArcBatch {
    std::vector<Arc> arcs;
    StrokeStyle stroke;
};

struct RectBatch {
    std::vector<Rect> rects;
    StrokeStyle stroke;
    std::optional<Color> fill;
};

struct TextBatch {
    std::vector<text::TextObject> textObjects;
};

enum class CoordinateSpace {
    World,
    Screen
};

struct DrawLayer {
    std::string name;
    int order = 0;
    CoordinateSpace coordinateSpace = CoordinateSpace::World;

    std::vector<MarkerBatch> markerBatches;
    std::vector<LineBatch> lineBatches;
    std::vector<CircleBatch> circleBatches;
    std::vector<ArcBatch> arcBatches;
    std::vector<RectBatch> rectBatches;
    std::vector<TextBatch> textBatches;

    void clear() {
        markerBatches.clear();
        lineBatches.clear();
        circleBatches.clear();
        arcBatches.clear();
        rectBatches.clear();
        textBatches.clear();
    }

    bool empty() const {
        return markerBatches.empty()
            && lineBatches.empty()
            && circleBatches.empty()
            && arcBatches.empty()
            && rectBatches.empty()
            && textBatches.empty();
    }
};

struct Grid {
    float cellSize;
    float subCellSize;

    Color majorColor{};
    Color minorColor{};
    Color axisColor{};
};

struct RenderScene {
    std::vector<DrawLayer> layers;
    std::optional<Grid> grid;

    void clear() {
        layers.clear();
        grid.reset();
    }
};

}

#endif // ! OURPAINT_RENDER_RENDERSCENE_H_