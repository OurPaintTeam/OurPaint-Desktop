#ifndef OURPAINT_SMILERIGHTCLICKFILTER_H
#define OURPAINT_SMILERIGHTCLICKFILTER_H

#include <QMouseEvent>

// Used to track right-click
// clicks to open emoticons in the dialog

class SmileRightClickFilter : public QObject {
Q_OBJECT

public:
    SmileRightClickFilter(QObject *parent = nullptr) : QObject(parent) {}
signals:
    void rightClicked(QObject *obj);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif //OURPAINT_SMILERIGHTCLICKFILTER_H
