#ifndef OURPAINT_KEYEVENTOOLPANEL_H
#define OURPAINT_KEYEVENTOOLPANEL_H

#include <QKeyEvent>
#include <QWidget>

class [[maybe_unused]] KeyEventToolPanel : public QWidget {
Q_OBJECT

public:
    [[maybe_unused]] KeyEventToolPanel(QWidget *parent) : QWidget(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        QWidget::keyPressEvent(event);
    }
};

#endif //OURPAINT_KEYEVENTOOLPANEL_H
