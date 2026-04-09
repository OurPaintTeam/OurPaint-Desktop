#ifndef OURPAINT_HEADERS_SCENE_OBSERVER_H_
#define OURPAINT_HEADERS_SCENE_OBSERVER_H_

namespace SceneObjects {class ID;}
struct Requirement;

class ISceneObserver {
public:
    virtual void pointAdded(SceneObjects::ID id, const double* x, const double* y) = 0;
    virtual void sectionAdded(SceneObjects::ID id, const double* x1, const double* y1, const double* x2, const double* y2) = 0;
    virtual void circleAdded(SceneObjects::ID id, const double* x, const double* y, const double* r) = 0;
    virtual void arcAdded(SceneObjects::ID id,
                    const double* beg_x,
                    const double* beg_y,
                    const double* end_x,
                    const double* end_y,
                    const double* center_x,
                    const double* center_y) = 0;
    virtual void reqAdded(const Requirement& req) = 0;

    virtual ~ISceneObserver() = default;
};

#ifdef QT_VERSION
#include <QtPlugin>
Q_DECLARE_INTERFACE(ISceneObserver,"org.ourpaint.ISceneObserver/1.0")
#endif

#endif // ! OURPAINT_HEADERS_SCENE_OBSERVER_H_