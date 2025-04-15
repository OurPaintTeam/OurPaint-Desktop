#ifndef OURPAINT_KEYEVENTTOPBAR_H
#define OURPAINT_KEYEVENTTOPBAR_H

#include <QKeyEvent>
#include <QWidget>

class KeyEventTopBar : public QWidget {
Q_OBJECT

public:
    KeyEventTopBar(QWidget *parent) : QWidget(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        QWidget::keyPressEvent(event);
    }
};

#endif //OURPAINT_KEYEVENTTOPBAR_H
