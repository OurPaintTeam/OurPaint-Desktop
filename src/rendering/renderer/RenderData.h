#ifndef OURPAINT_RENDERING_RENDERSCENE_H_
#define OURPAINT_RENDERING_RENDERSCENE_H_

#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "text/Types.h"

namespace renderer {
struct Point {
    float x;
    float y;
};

struct Line {
    float x1;
    float y1;
    float x2;
    float y2;
};
struct CircleArc {
    float x;
    float y;
    float r;
    float startAngle = 0.0;
    float endAngle = 0.0;
};
struct Rect {
    float xMin;
    float yMin;
    float xMax;
    float yMax;
};

#include <vector>

class RenderData {
public:
    // Base
    std::vector<Point> points;
    std::vector<Line> lines;
    std::vector<CircleArc> circles;

    struct Layer {
        std::vector<Point> points;
        std::vector<Line> lines;
        std::vector<CircleArc> circles;

        void clear() {
            points.clear();
            lines.clear();
            circles.clear();
        }

        bool empty() const {
            return points.empty() && lines.empty() && circles.empty();
        }
    };

    Layer overlay;
    Layer selected;
    Layer special;

    std::optional<Rect> selectionRect;

    void clear() {
        points.clear();
        lines.clear();
        circles.clear();
        overlay.clear();
        selected.clear();
        selectionRect.reset();
    }


    std::vector<rendering::text::TextObject> textObjects_;

    struct GridInfo {
        float cellSize;
        float subCellSize;
        glm::vec3 gridColor{};
        glm::vec3 axisColor{};
    } gridInfo;

};
}

#endif // ! OURPAINT_RENDERING_RENDERSCENE_H_