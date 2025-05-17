#ifndef OURPAINT_GLOBALEVENFILTER_H
#define OURPAINT_GLOBALEVENFILTER_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>

class [[maybe_unused]] GlobalEventFilter : public QObject {
Q_OBJECT

public:
    bool eventFilter(QObject *obj, QEvent *event) override {
        return QObject::eventFilter(obj, event);
    }
};

#endif //OURPAINT_GLOBALEVENFILTER_H
