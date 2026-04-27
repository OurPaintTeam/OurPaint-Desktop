#ifndef OURPAINT_HEADERS_SCENE_OBSERVER_H_
#define OURPAINT_HEADERS_SCENE_OBSERVER_H_

#include "Scene.h"

class ISceneObserver {
public:
    virtual ~ISceneObserver() = default;

    virtual void onObjectAdded          (const core::Scene::ObjDescriptor& obj) = 0;
    virtual void onObjectUpdated        (core::Scene::DCM_ID id) = 0;
    virtual void onObjectRemoved        (core::Scene::DCM_ID id) = 0;

    virtual void onRequirementAdded     (const core::Scene::ReqDescriptor& req) = 0;
    virtual void onRequirementUpdated   (core::Scene::DCM_ID req) = 0;
    virtual void onRequirementRemoved   (core::Scene::DCM_ID req) = 0;

    virtual void onObjectAdded          (const std::vector<core::Scene::DCM_ID>& id) = 0;
    virtual void onObjectUpdated        (const std::vector<core::Scene::DCM_ID>& id) = 0;
    //virtual void onObjectRemoved        (const std::vector<Scene::DCM_ID>& id) = 0;

    virtual void onRequirementAdded     (const std::vector<core::Scene::DCM_ID>& req) = 0;
    //virtual void onRequirementUpdated   (const std::vector<Scene::ID>& req) = 0;
    //virtual void onRequirementRemoved   (const std::vector<Scene::ID>& req) = 0;

    virtual void setStorage(const core::Scene::Storage* storage) = 0;
};

#endif // ! OURPAINT_HEADERS_SCENE_OBSERVER_H_