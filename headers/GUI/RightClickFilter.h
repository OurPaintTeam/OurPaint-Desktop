#ifndef OURPAINT_HEADERS_GUI_RIGHTCLICKFILTER_H_
#define OURPAINT_HEADERS_GUI_RIGHTCLICKFILTER_H_

#include <QMouseEvent>
class RightClickFilter : public QObject {
    Q_OBJECT

public:
    RightClickFilter(QObject *parent = nullptr) : QObject(parent) {}

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
#endif // ! OURPAINT_HEADERS_GUI_RIGHTCLICKFILTER_H_
