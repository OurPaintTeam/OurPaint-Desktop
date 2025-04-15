#ifndef OURPAINT_SMILERIGHTCLICKFILTER_H
#define OURPAINT_SMILERIGHTCLICKFILTER_H

#include <QMouseEvent>

class SmileRightClickFilter : public QObject {
Q_OBJECT

// Используется для отслеживания нажатий правой кнопки мыши для открытия смайликов в диалоге

public:
    SmileRightClickFilter(QObject *parent = nullptr) : QObject(parent) {}

signals:

    void rightClicked(QObject *obj);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::RightButton) {
                emit rightClicked(obj);
                return true;
            }
        }
        return QObject::eventFilter(obj, event);
    }
};

#endif //OURPAINT_SMILERIGHTCLICKFILTER_H
