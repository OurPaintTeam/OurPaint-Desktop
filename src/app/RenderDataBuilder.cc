#include "RenderDataBuilder.h"
#include "Scene.h"
#include "OverlayModel.h"
#include "RenderData.h"
#include "objects/Objects.h"
#include "AxisTexts.h"

using namespace core;

RenderDataBuilder::RenderDataBuilder(const Scene& scene,
                                     const OverlayModel& overlay,
                                     AxisTexts& axis,
                                     renderer::RenderData& renderData)
    : scene_(scene), overlay_(overlay), axis_(axis), renderData_(renderData) {}

void RenderDataBuilder::rebuild() {
    renderData_.points.clear();
    renderData_.lines.clear();
    renderData_.circles.clear();
    //renderData_.overlay.clear();
    renderData_.selected.clear();
    renderData_.selectionRect.reset();
    renderData_.special.clear();


    // Base points
    std::vector<ObjectData> points = scene_.getPoints();
    for (auto& p : points) {
        const double& x = p.params[0];
        const double& y = p.params[1];
        renderData_.points.push_back(renderer::Point(x, y));
    }

    // Base lines
    std::vector<ObjectData> lines = scene_.getLines();
    for (auto& l : lines) {
        const double& x1 = l.params[0];
        const double& y1 = l.params[1];
        const double& x2 = l.params[2];
        const double& y2 = l.params[3];
        renderData_.lines.push_back(renderer::Line(x1, y1, x2, y2));
    }

    // Base circles
    std::vector<ObjectData> circles = scene_.getCircles();
    for (auto& c : circles) {
        const double& x = c.params[0];
        const double& y = c.params[1];
        const double& r = c.params[2];
        renderData_.circles.push_back(renderer::CircleArc(x, y, r));
        renderData_.points.push_back(renderer::Point(x, y));
    }

    // Selection objects
    for (const auto& id : overlay_.selection_.model.items()) {
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
    if (overlay_.selectionRect_.has_value()) {
        renderer::Rect r;
        r.xMin = overlay_.selectionRect_->xMin;
        r.yMin = overlay_.selectionRect_->yMin;
        r.xMax = overlay_.selectionRect_->xMax;
        r.yMax = overlay_.selectionRect_->yMax;
        renderData_.selectionRect = r;
    }

    // Overlay points
    for (const auto& point : overlay_.points_) {
        renderer::Point p;
        p.x = point.x;
        p.y = point.y;
        renderData_.points.push_back(p);
    }

    // Overlay line
    for (const auto& line : overlay_.lines_) {
        renderer::Line l;
        l.x1 = line.x1;
        l.y1 = line.y1;
        l.x2 = line.x2;
        l.y2 = line.y2;
        renderData_.lines.push_back(l);
    }

    // Overlay circle
    for (const auto& circle : overlay_.circles_) {
        renderer::CircleArc c;
        c.x = circle.cx;
        c.y = circle.cy;
        c.r = circle.r;
        renderData_.circles.push_back(c);
    }

    // Overlay arc
    for (const auto& arc : overlay_.arcs_) {
        renderer::CircleArc ca;
        ca.x = arc.cx;
        ca.y = arc.cy;
        ca.r = arc.r;
        ca.startAngle = arc.startAngle;
        ca.endAngle = arc.endAngle;
        renderData_.circles.push_back(ca);
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





    // TODO text EXPERIMENTAL

    axis_.update();
    renderData_.gridInfo.cellSize = axis_.getGridInfo().cellSize;
    renderData_.gridInfo.subCellSize = axis_.getGridInfo().subCellSize;
    renderData_.gridInfo.gridColor = axis_.getGridInfo().gridColor;
    renderData_.gridInfo.axisColor = axis_.getGridInfo().axisColor;

    renderData_.textObjects_.clear();
    renderData_.textObjects_ = axis_.getLabels();
}


