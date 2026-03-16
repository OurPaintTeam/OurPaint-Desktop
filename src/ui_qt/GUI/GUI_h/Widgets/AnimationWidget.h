#ifndef OURPAINT_ANIMATIONWIDGET_H
#define OURPAINT_ANIMATIONWIDGET_H

#include <QWidget>
#include <QObject>
#include <QLineEdit>
#include <QLabel>
#include <QPainter>


// This class is for drawing animations on the home page.

class AnimationWidget : public QWidget {
Q_OBJECT

private:
    QLabel* label;

public:
    explicit AnimationWidget(QWidget* parent = nullptr);

protected:
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent* event);
};

#endif //OURPAINT_ANIMATIONWIDGET_H
