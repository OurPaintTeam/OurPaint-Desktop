#include "RenderDataBuilder.h"
#include "Scene.h"
#include "OverlayModel.h"
#include "RenderData.h"

RenderDataBuilder::RenderDataBuilder(const Scene& scene, const OverlayModel& overlay, renderer::RenderData& renderData)
    : scene_(scene), overlay_(overlay), renderData_(renderData) {}

void RenderDataBuilder::rebuild() {
    renderData_.points.clear();
    renderData_.lines.clear();
    renderData_.circles.clear();
    //renderData_.overlay.clear();
    renderData_.selected.clear();
    renderData_.selectionRect.reset();
    renderData_.special.clear();
    std::vector<ObjectData> points = scene_.getPoints();
    for (auto& p : points) {
        const double& x = p.params[0];
        const double& y = p.params[1];
        renderData_.points.push_back(renderer::Point(x, y));
    }

    std::vector<ObjectData> lines = scene_.getLines();
    for (auto& l : lines) {
        const double& x1 = l.params[0];
        const double& y1 = l.params[1];
        const double& x2 = l.params[2];
        const double& y2 = l.params[3];
        renderData_.lines.push_back(renderer::Line(x1, y1, x2, y2));
    }

    std::vector<ObjectData> circles = scene_.getCircles();
    for (auto& c : circles) {
        const double& x = c.params[0];
        const double& y = c.params[1];
        const double& r = c.params[2];
        renderData_.circles.push_back(renderer::Circle(x, y, r));
        renderData_.points.push_back(renderer::Point(x, y));
    }

    for (const auto& id : overlay_.selection_.items()) {
        for (const auto& p : points) {
            if (p.id.get() == id.get()) {
                renderData_.selected.points.push_back({static_cast<float>(p.params[0]), static_cast<float>(p.params[1])});
            }
        }
        for (const auto& l : lines) {
            if (l.id.get() == id.get()) {
                renderData_.selected.lines.push_back({
                    static_cast<float>(l.params[0]),
                    static_cast<float>(l.params[1]),
                    static_cast<float>(l.params[2]),
                    static_cast<float>(l.params[3])
                });

            }
        }
        for (const auto& c : circles) {
            if (c.id.get() == id.get()) {
                renderData_.selected.circles.push_back({
                    static_cast<float>(c.params[0]),
                    static_cast<float>(c.params[1]),
                    static_cast<float>(c.params[2])
                });

            }
        }
    }

    if (overlay_.rect_.has_value()) {
        renderer::Rect r;
        r.xMin = overlay_.rect_->xMin;
        r.yMin = overlay_.rect_->yMin;
        r.xMax = overlay_.rect_->xMax;
        r.yMax = overlay_.rect_->yMax;

        renderData_.selectionRect = r;
    }

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
}


