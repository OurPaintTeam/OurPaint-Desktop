#ifndef OURPAINT_HEADERS_SCENE_OBSERVER_H_
#define OURPAINT_HEADERS_SCENE_OBSERVER_H_

#include "Scene.h"

class ISceneObserver {
public:
    virtual ~ISceneObserver() = default;

    virtual void onObjectAdded          (const Scene::ObjDescriptor& obj) = 0;
    virtual void onObjectUpdated        (Scene::DCM_ID id) = 0;
    virtual void onObjectRemoved        (Scene::DCM_ID id) = 0;

    virtual void onRequirementAdded     (const Scene::ReqDescriptor& req) = 0;
    virtual void onRequirementUpdated   (Scene::DCM_ID req) = 0;
    virtual void onRequirementRemoved   (Scene::DCM_ID req) = 0;

    virtual void onObjectAdded          (const std::vector<Scene::DCM_ID>& id) = 0;
    virtual void onObjectUpdated        (const std::vector<Scene::DCM_ID>& id) = 0;
    //virtual void onObjectRemoved        (const std::vector<Scene::DCM_ID>& id) = 0;

    virtual void onRequirementAdded     (const std::vector<Scene::DCM_ID>& req) = 0;
    //virtual void onRequirementUpdated   (const std::vector<Scene::ID>& req) = 0;
    //virtual void onRequirementRemoved   (const std::vector<Scene::ID>& req) = 0;

    virtual void setStorage(const Scene::Storage* storage) = 0;
};

#endif // ! OURPAINT_HEADERS_SCENE_OBSERVER_H_