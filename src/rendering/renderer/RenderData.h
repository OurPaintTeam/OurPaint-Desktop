#ifndef OURPAINT_RENDERING_RENDERSCENE_H_
#define OURPAINT_RENDERING_RENDERSCENE_H_

#include <cmath>
#include <optional>
#include <string>
#include <vector>

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
struct Circle {
    float x;
    float y;
    float r;
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
    std::vector<Point> points;
    std::vector<Line> lines;
    std::vector<Circle> circles;

    struct Layer {
        std::vector<Point> points;
        std::vector<Line> lines;
        std::vector<Circle> circles;

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

    struct to {
        std::string str;
        int posX{};
        int posY{};
        int a{};
    };

    std::vector<to> texts;

    to linePrview;

    std::string pos;
    int posX{};
    int posY{};
};
}

#endif // ! OURPAINT_RENDERING_RENDERSCENE_H_