#ifndef OURPAINT_HEADERS_SCENE_OBSERVER_H_
#define OURPAINT_HEADERS_SCENE_OBSERVER_H_

#include <QObject>
class Scene;
class ID;
class Requirement;

class SceneQtAdapter : public QObject {
    Q_OBJECT
public:
    explicit SceneQtAdapter(Scene& scene);

    void onPointAdded(ID id, const double* x, const double* y);
    void onSectionAdded(ID id, const double* x1, const double* y1, const double* x2, const double* y2);
    void onCircleAdded(ID id, const double* x, const double* y, const double* r);
    void onArcAdded(ID id,
                    const double* beg_x,
                    const double* beg_y,
                    const double* end_x,
                    const double* end_y,
                    const double* center_x,
                    const double* center_y);
    void onReqAdded(const Requirement& req);

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


#endif // ! OURPAINT_HEADERS_SCENE_OBSERVER_H_