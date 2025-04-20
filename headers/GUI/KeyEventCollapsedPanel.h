#ifndef OURPAINT_MOUSEEVENTCOLLAPSEDPANEL_H
#define OURPAINT_MOUSEEVENTCOLLAPSEDPANEL_H

#include <QKeyEvent>
#include <QWidget>

class KeyEventcollapsedPanel : public QWidget {
Q_OBJECT

public:
    KeyEventcollapsedPanel(QWidget *parent) : QWidget(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        QWidget::keyPressEvent(event);
    }
};

#endif //OURPAINT_MOUSEEVENTCOLLAPSEDPANEL_H
