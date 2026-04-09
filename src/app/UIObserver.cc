#include "UIObserver.h"
#include "Document.h"
#include "Scene.h"

UIObserver::UIObserver()
        : QObject(nullptr) {}

void UIObserver::pointAdded(SceneObjects::ID id, const double* x, const double* y) {
    emit pointAddedQt(id, x, y);
    emit sceneChanged();
}

void UIObserver::sectionAdded(SceneObjects::ID id, const double* x1, const double* y1, const double* x2, const double* y2) {
    emit sectionAddedQt(id, x1, y1, x2, y2);
    emit sceneChanged();
}

void UIObserver::circleAdded(SceneObjects::ID id, const double* x, const double* y, const double* r) {
    emit circleAddedQt(id, x, y, r);
    emit sceneChanged();
}

void UIObserver::arcAdded(SceneObjects::ID id,
                                const double* beg_x,
                                const double* beg_y,
                                const double* end_x,
                                const double* end_y,
                                const double* center_x,
                                const double* center_y) {
    emit arcAddedQt(id, beg_x, beg_y, end_x, end_y, center_x, center_y);
    emit sceneChanged();
}

void UIObserver::reqAdded(const Requirement& req) {
    emit reqAddedQt(req);
    emit sceneChanged();
}

#include "moc_UIObserver.cpp"
