#include "RenderDataBuilder.h"
#include "Scene.h"
#include "OverlayModel.h"
#include "RenderData.h"

RenderDataBuilder::RenderDataBuilder(const Scene& scene, const OverlayModel& overlay, renderer::RenderData& renderData)
    : scene_(scene), overlay_(overlay), renderData_(renderData) {}
#include <cmath>

struct CircleArc {
    float cx, cy, r;
    float startAngle, endAngle;
    bool valid;
};

static float normalizeAngle(float a) {
    constexpr float TAU = 2.0f * static_cast<float>(M_PI);
    a = std::fmod(a, TAU);
    if (a < 0.0f) a += TAU;
    return a;
}

// true, если angle лежит на дуге от start до end при обходе против часовой стрелки
static bool angleOnCCWArc(float angle, float start, float end) {
    angle = normalizeAngle(angle);
    start = normalizeAngle(start);
    end   = normalizeAngle(end);

    if (start <= end) {
        return angle >= start && angle <= end;
    } else {
        return angle >= start || angle <= end;
    }
}

CircleArc buildArcFromThreePoints(const ObjectData& p0,
                                  const ObjectData& p1,
                                  const ObjectData& p2)
{
    const float x0 = p0.params[0], y0 = p0.params[1];
    const float x1 = p1.params[0], y1 = p1.params[1];
    const float x2 = p2.params[0], y2 = p2.params[1];

    // Удвоенная ориентированная площадь треугольника
    const float det =
        x0 * (y1 - y2) +
        x1 * (y2 - y0) +
        x2 * (y0 - y1);

    if (std::fabs(det) < 1e-6f) {
        return {0, 0, 0, 0, 0, false};
    }

    const float s0 = x0 * x0 + y0 * y0;
    const float s1 = x1 * x1 + y1 * y1;
    const float s2 = x2 * x2 + y2 * y2;

    const float cx =
        (s0 * (y1 - y2) +
         s1 * (y2 - y0) +
         s2 * (y0 - y1)) / (2.0f * det);

    const float cy =
        (s0 * (x2 - x1) +
         s1 * (x0 - x2) +
         s2 * (x1 - x0)) / (2.0f * det);

    const float r = std::hypot(x0 - cx, y0 - cy);

    auto angleOf = [&](float x, float y) -> float {
        return normalizeAngle(std::atan2(y - cy, x - cx));
    };

    const float a0 = angleOf(x0, y0);
    const float a1m = angleOf(x1, y1);
    const float a2 = angleOf(x2, y2);

    float startAngle;
    float endAngle;

    // Если средняя точка лежит на CCW-дуге от p0 к p2,
    // значит именно эта дуга нам и нужна.
    if (angleOnCCWArc(a1m, a0, a2)) {
        startAngle = a0;
        endAngle   = a2;
    } else {
        startAngle = a2;
        endAngle   = a0;
    }

    return {cx, cy, r, startAngle, endAngle, true};
}
void RenderDataBuilder::rebuild() {
    renderData_.points.clear();
    renderData_.lines.clear();
    renderData_.circles.clear();
    //renderData_.overlay.clear();
    renderData_.selected.clear();
    renderData_.selectionRect.reset();
    renderData_.special.clear();


    // Points
    std::vector<ObjectData> points = scene_.getPoints();
    for (auto& p : points) {
        const double& x = p.params[0];
        const double& y = p.params[1];
        renderData_.points.push_back(renderer::Point(x, y));
    }

    // Lines
    std::vector<ObjectData> lines = scene_.getLines();
    for (auto& l : lines) {
        const double& x1 = l.params[0];
        const double& y1 = l.params[1];
        const double& x2 = l.params[2];
        const double& y2 = l.params[3];
        renderData_.lines.push_back(renderer::Line(x1, y1, x2, y2));
    }

    // Circles
    std::vector<ObjectData> circles = scene_.getCircles();
    for (auto& c : circles) {
        const double& x = c.params[0];
        const double& y = c.params[1];
        const double& r = c.params[2];
        renderData_.circles.push_back(renderer::CircleArc(x, y, r));
        renderData_.points.push_back(renderer::Point(x, y));
    }

    // Selection objects
    for (const auto& id : overlay_.selection_.items()) {
        ObjectData od = scene_.getObjectData(id);
        if (od.et == ObjType::ET_POINT) {
            renderData_.selected.points.push_back({static_cast<float>(od.params[0]), static_cast<float>(od.params[1])});
        }
        else if (od.et == ObjType::ET_LINE) {
            renderData_.selected.lines.push_back({
                static_cast<float>(od.params[0]),
                static_cast<float>(od.params[1]),
                static_cast<float>(od.params[2]),
                static_cast<float>(od.params[3])
            });
        }
        else if (od.et == ObjType::ET_CIRCLE) {
            renderData_.selected.circles.push_back({
                static_cast<float>(od.params[0]),
                static_cast<float>(od.params[1]),
                static_cast<float>(od.params[2])
            });
        }
    }

    // Overlay selection rectangle
    if (overlay_.rect_.has_value()) {
        renderer::Rect r;
        r.xMin = overlay_.rect_->xMin;
        r.yMin = overlay_.rect_->yMin;
        r.xMax = overlay_.rect_->xMax;
        r.yMax = overlay_.rect_->yMax;
        renderData_.selectionRect = r;
    }

    // Preview circle
    if (overlay_.previewCircle_.has_value()) {
        renderer::CircleArc c;
        c.x = overlay_.previewCircle_.value().cx;
        c.y = overlay_.previewCircle_.value().cy;
        c.r = overlay_.previewCircle_.value().r;
        renderData_.circles.push_back(c);
        renderData_.points.push_back({c.x, c.y});
    }

    // Cubic bezier curve
    std::vector<ObjectData> beziers = scene_.getBeziers();
    for (size_t i = 0; i < beziers.size(); i++) {
        ObjectData od = beziers[i];

        renderer::Point p0(od.params[0], od.params[1]);
        renderer::Point p3(od.params[2], od.params[3]);

        renderer::Point p1(od.params[4], od.params[5]);
        renderer::Point p2(od.params[6], od.params[7]);

        renderData_.special.lines.push_back(renderer::Line(od.params[0], od.params[1], od.params[4], od.params[5]));
        renderData_.special.lines.push_back(renderer::Line(od.params[2], od.params[3], od.params[6], od.params[7]));

        double t = 0.0;
        double x0 = (1-t)*(1-t)*(1-t)*p0.x + 3*(1-t)*(1-t)*t*p1.x + 3*(1-t)*t*t*p2.x + t*t*t*p3.x;
        double y0 = (1-t)*(1-t)*(1-t)*p0.y + 3*(1-t)*(1-t)*t*p1.y + 3*(1-t)*t*t*p2.y + t*t*t*p3.y;
        while (t < 1.01) {
            double x1 = (1-t)*(1-t)*(1-t)*p0.x + 3*(1-t)*(1-t)*t*p1.x + 3*(1-t)*t*t*p2.x + t*t*t*p3.x;
            double y1 = (1-t)*(1-t)*(1-t)*p0.y + 3*(1-t)*(1-t)*t*p1.y + 3*(1-t)*t*t*p2.y + t*t*t*p3.y;
            t += 0.01;
            renderData_.lines.push_back(renderer::Line(x0, y0, x1, y1));
            x0 = x1;
            y0 = y1;
        }
    }

    // if (points.size() >= 3) {
    //     CircleArc arc = buildArcFromThreePoints(points[0], points[1], points[2]);
    //     if (arc.valid) {
    //         renderer::CircleArc c(arc.cx, arc.cy, arc.r, arc.startAngle, arc.endAngle);
    //         renderData_.circles.push_back(c);
    //     }
    // }
}


