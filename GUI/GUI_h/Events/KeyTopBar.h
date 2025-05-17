#ifndef OURPAINT_KEYEVENTTOPBAR_H
#define OURPAINT_KEYEVENTTOPBAR_H

#include <QKeyEvent>
#include <QWidget>

class [[maybe_unused]] KeyEventTopBar : public QWidget {
Q_OBJECT

public:
    [[maybe_unused]] KeyEventTopBar(QWidget *parent) : QWidget(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        QWidget::keyPressEvent(event);
    }
};

#endif //OURPAINT_KEYEVENTTOPBAR_H
