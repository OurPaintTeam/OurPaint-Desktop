#include "RenderDataObserver.h"
#include "RenderData.h"

RenderDataObserver::RenderDataObserver(renderer::RenderData& data) : renderData_(data), storage_(nullptr) {}

void RenderDataObserver::onObjectAdded(const Scene::ObjDescriptor& obj) {
    if (obj.type == Scene::Type::ET_POINT2D) {
        const double& x = obj.coords[0];
        const double& y = obj.coords[1];

        renderData_.points.push_back(renderer::Point(x, y));
    }
    else if (obj.type == Scene::Type::ET_LINE) {
        double x1 = obj.coords[0];
        double y1 = obj.coords[1];
        double x2 = obj.coords[2];
        double y2 = obj.coords[3];

        renderData_.lines.push_back(renderer::Line(x1, y1, x2, y2));
        renderData_.points.push_back(renderer::Point(x1, y1));
        renderData_.points.push_back(renderer::Point(x2, y2));
    }
    else if (obj.type == Scene::Type::ET_CIRCLE) {
        double x = obj.coords[0];
        double y = obj.coords[1];
        double r = obj.radius.value();
        renderData_.circles.push_back(renderer::Circle(x, y, r));
        renderData_.points.push_back(renderer::Point(x, y));
    }
}

void RenderDataObserver::onObjectUpdated(Scene::ID id) {
    (void)id;
    rebuildRenderData();
}

void RenderDataObserver::onObjectRemoved(Scene::ID id) {
    (void)id;
    rebuildRenderData();
}

void RenderDataObserver::onRequirementAdded(const Scene::ReqDescriptor& req) {
    (void)req;
    rebuildRenderData();
}

void RenderDataObserver::onRequirementUpdated(Scene::ID req) {
    (void)req;
    rebuildRenderData();
}

void RenderDataObserver::onRequirementRemoved(Scene::ID req) {
    (void)req;
    rebuildRenderData();
}

void RenderDataObserver::onObjectAdded(const std::vector<Scene::ID>& id) {
    (void)id;
    rebuildRenderData();
}

void RenderDataObserver::onObjectUpdated(const std::vector<Scene::ID>& ids) {
    (void)ids;
    rebuildRenderData();
}

void RenderDataObserver::onRequirementAdded(const std::vector<Scene::ID>& req) {
    (void)req;
    rebuildRenderData();
}

void RenderDataObserver::setStorage(const Scene::Storage* storage) {
    storage_ = storage;
}

void RenderDataObserver::rebuildRenderData() {
    renderData_.clear();
    for (auto& p : storage_->allPoints()) {
        const double& x = p.x();
        const double& y = p.y();

        renderData_.points.push_back(renderer::Point(x, y));
    }

    for (auto& s : storage_->allLines()) {
        double x1 = s.p1->x();
        double y1 = s.p1->y();
        double x2 = s.p2->x();
        double y2 = s.p2->y();

        renderData_.lines.push_back(renderer::Line(x1, y1, x2, y2));
    }

    for (auto& c : storage_->allCircles()) {
        double x = c.center->x();
        double y = c.center->y();
        double r = c.radius;
        renderData_.circles.push_back(renderer::Circle(x, y, r));
        renderData_.points.push_back(renderer::Point(x, y));
    }
}




