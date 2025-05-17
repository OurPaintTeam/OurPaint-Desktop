#include "SmileRightClickFilter.h"

bool SmileRightClickFilter::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent * mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            emit rightClicked(obj);
            return true;
        }
    }
    return QObject::eventFilter(obj, event);
}