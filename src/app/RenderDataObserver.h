#ifndef OURPAINT_APPLICATION_RENDER_DATA_OBSERVER_H_
#define OURPAINT_APPLICATION_RENDER_DATA_OBSERVER_H_

#include "ISceneObserver.h"
namespace renderer {class RenderData;}

class RenderDataObserver : public ISceneObserver {
public:
    RenderDataObserver(renderer::RenderData& scene);
    ~RenderDataObserver() override = default;

    void onObjectAdded          (const Scene::ObjDescriptor& obj) override;
    void onObjectUpdated        (Scene::ID id) override;
    void onObjectRemoved        (Scene::ID id) override;

    void onRequirementAdded     (const Scene::ReqDescriptor& req) override;
    void onRequirementUpdated   (Scene::ID req) override;
    void onRequirementRemoved   (Scene::ID req) override;

    void onObjectAdded          (const std::vector<Scene::ID>& id) override;
    void onObjectUpdated        (const std::vector<Scene::ID>& ids) override;
    void onRequirementAdded     (const std::vector<Scene::ID>& req) override;

    void setStorage(const Scene::Storage* storage) override;

private:
    void rebuildRenderData();

    renderer::RenderData& renderData_;
    const Scene::Storage* storage_;
};

#endif // ! OURPAINT_APPLICATION_RENDER_DATA_OBSERVER_H_