#ifndef OURPAINT_HEADERS_SCENE_QT_ADAPTER_H_
#define OURPAINT_HEADERS_SCENE_QT_ADAPTER_H_

#include <QObject>
#include "SceneObserver.h"

class Scene;
class ID;
struct Requirement;

class SceneQtAdapter final
        : public QObject,
          public ISceneObserver {
    Q_OBJECT
    Q_INTERFACES(ISceneObserver)
public:
    explicit SceneQtAdapter(Scene& scene);

    void pointAdded(ID id, const double* x, const double* y) override;
    void sectionAdded(ID id, const double* x1, const double* y1, const double* x2, const double* y2) override;
    void circleAdded(ID id, const double* x, const double* y, const double* r) override;
    void arcAdded(ID id,
                    const double* beg_x,
                    const double* beg_y,
                    const double* end_x,
                    const double* end_y,
                    const double* center_x,
                    const double* center_y) override;
    void reqAdded(const Requirement& req) override;

signals:
    void pointAddedQt(ID id, const double* x, const double* y);
    void sectionAddedQt(ID id, const double* x1, const double* y1, const double* x2, const double* y2);
    void circleAddedQt(ID id, const double* x, const double* y, const double* r);
    void arcAddedQt(ID id,
                    const double* beg_x,
                    const double* beg_y,
                    const double* end_x,
                    const double* end_y,
                    const double* center_x,
                    const double* center_y);
    void reqAddedQt(const Requirement& req);
};

#endif // ! OURPAINT_HEADERS_SCENE_QT_ADAPTER_H_
