#ifndef OURPAINT_RENDERING_RENDERSCENE_H_
#define OURPAINT_RENDERING_RENDERSCENE_H_

#include <vector>
#include <cmath>

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

class RenderData {
public:
    std::vector<Point> points;
    std::vector<Line> lines;
    std::vector<Circle> circles;

    struct {
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
    } overlay;

    void clear() {
        points.clear();
        lines.clear();
        circles.clear();
    }
};
}

#endif // ! OURPAINT_RENDERING_RENDERSCENE_H_