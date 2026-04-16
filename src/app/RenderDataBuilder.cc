#include "RenderDataObserver.h"
#include "RenderData.h"

RenderDataObserver::RenderDataObserver(renderer::RenderData& data, OverlayModel& overlay)
    : renderData_(data), storage_(nullptr), overlay_(overlay) {}

void RenderDataObserver::onObjectAdded(const Scene::ObjDescriptor& obj) {
    renderData_.selected.clear();
    if (obj.type == Scene::Type::ET_POINT2D) {
        const float& x = obj.coords[0];
        const float& y = obj.coords[1];
        renderData_.points.push_back(renderer::Point(x, y));
    }
    else if (obj.type == Scene::Type::ET_LINE) {
        float x1 = obj.coords[0];
        float y1 = obj.coords[1];
        float x2 = obj.coords[2];
        float y2 = obj.coords[3];
        renderData_.lines.push_back(renderer::Line(x1, y1, x2, y2));
        renderData_.points.push_back(renderer::Point(x1, y1));
        renderData_.points.push_back(renderer::Point(x2, y2));
    }
    else if (obj.type == Scene::Type::ET_CIRCLE) {
        float x = obj.coords[0];
        float y = obj.coords[1];
        float r = obj.radius.value();
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
    renderData_.points.clear();
    renderData_.lines.clear();
    renderData_.circles.clear();
    renderData_.overlay.clear();
    renderData_.selected.clear();
    auto& points = storage_->pointsWithIds();
    for (auto& p : points) {
        const double& x = p.ptr->x();
        const double& y = p.ptr->y();

        renderData_.points.push_back(renderer::Point(x, y));
    }

    auto& lines = storage_->linesWithIds();
    for (auto& s : lines) {
        const double& x1 = s.ptr->p1->x();
        const double& y1 = s.ptr->p1->y();
        const double& x2 = s.ptr->p2->x();
        const double& y2 = s.ptr->p2->y();

        renderData_.lines.push_back(renderer::Line(x1, y1, x2, y2));
    }

    auto& cirlces = storage_->circlesWithIds();
    for (auto& c : cirlces) {
        const double& x = c.ptr->center->x();
        const double& y = c.ptr->center->y();
        const double& r = c.ptr->radius;
        renderData_.circles.push_back(renderer::Circle(x, y, r));
        renderData_.points.push_back(renderer::Point(x, y));
    }

    for (const auto& id : overlay_.selection_.items()) {
        for (const auto& p : points) {
            if (p.id.id == id.get() + 1) {
                renderData_.selected.points.push_back({static_cast<float>(p.ptr->coords[0]), static_cast<float>(p.ptr->coords[1])});
            }
        }
        for (const auto& l : lines) {
            if (l.id.id == id.get() + 1) {
                renderData_.selected.lines.push_back({
                    static_cast<float>(l.ptr->p1->coords[0]),
                    static_cast<float>(l.ptr->p1->coords[1]),
                    static_cast<float>(l.ptr->p2->coords[0]),
                    static_cast<float>(l.ptr->p2->coords[1])
                });

            }
        }
        for (const auto& c : cirlces) {
            if (c.id.id == id.get() + 1) {
                renderData_.selected.circles.push_back({
                    static_cast<float>(c.ptr->center->coords[0]),
                    static_cast<float>(c.ptr->center->coords[1]),
                    static_cast<float>(c.ptr->radius)
                });

            }
        }
    }

}




