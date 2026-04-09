#ifndef OURPAINT_HEADERS_SCENE_QT_ADAPTER_H_
#define OURPAINT_HEADERS_SCENE_QT_ADAPTER_H_

#include <QObject>
#include "ISceneObserver.h"

namespace SceneObjects {class ID;}
struct Requirement;

class UIObserver final
        : public QObject,
          public ISceneObserver {
    Q_OBJECT
public:
    explicit UIObserver();

    void pointAdded(SceneObjects::ID id, const double* x, const double* y) override;
    void sectionAdded(SceneObjects::ID id, const double* x1, const double* y1, const double* x2, const double* y2) override;
    void circleAdded(SceneObjects::ID id, const double* x, const double* y, const double* r) override;
    void arcAdded(SceneObjects::ID id,
                    const double* beg_x,
                    const double* beg_y,
                    const double* end_x,
                    const double* end_y,
                    const double* center_x,
                    const double* center_y) override;
    void reqAdded(const Requirement& req) override;

signals:
    void pointAddedQt(SceneObjects::ID id, const double* x, const double* y);
    void sectionAddedQt(SceneObjects::ID id, const double* x1, const double* y1, const double* x2, const double* y2);
    void circleAddedQt(SceneObjects::ID id, const double* x, const double* y, const double* r);
    void arcAddedQt(SceneObjects::ID id,
                    const double* beg_x,
                    const double* beg_y,
                    const double* end_x,
                    const double* end_y,
                    const double* center_x,
                    const double* center_y);
    void reqAddedQt(const Requirement& req);
    void sceneChanged();
};

#endif // ! OURPAINT_HEADERS_SCENE_QT_ADAPTER_H_
