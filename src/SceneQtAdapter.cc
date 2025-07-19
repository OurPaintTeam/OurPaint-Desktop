#include "SceneQtAdapter.h"
#include "Scene.h"

SceneQtAdapter::SceneQtAdapter(Scene& scene)
        : QObject(nullptr)
{
    scene.setObserver(this);
}

void SceneQtAdapter::pointAdded(ID id, const double* x, const double* y) {
    emit pointAddedQt(id, x, y);
}

void SceneQtAdapter::sectionAdded(ID id, const double* x1, const double* y1, const double* x2, const double* y2) {
    emit sectionAddedQt(id, x1, y1, x2, y2);
}

void SceneQtAdapter::circleAdded(ID id, const double* x, const double* y, const double* r) {
    emit circleAddedQt(id, x, y, r);
}

void SceneQtAdapter::arcAdded(ID id,
                                const double* beg_x,
                                const double* beg_y,
                                const double* end_x,
                                const double* end_y,
                                const double* center_x,
                                const double* center_y) {
    emit arcAddedQt(id, beg_x, beg_y, end_x, end_y, center_x, center_y);
}

void SceneQtAdapter::reqAdded(const Requirement& req) {
    emit reqAddedQt(req);
}

#include "moc_SceneQtAdapter.cpp"
