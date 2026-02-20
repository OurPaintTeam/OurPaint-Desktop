#include "AnimationWidget.h"


AnimationWidget::AnimationWidget(QWidget* parent)
        : QWidget(parent) {
    setAttribute(Qt::WA_OpaquePaintEvent);

    label = new QLabel("In development...", this);
    QFont font = label->font();
    constexpr qint8 size = 24;
    font.setPointSize(size);
    font.setBold(true);
    label->setFont(font);
    label->adjustSize();
    label->show();
}


void AnimationWidget::resizeEvent(QResizeEvent* event) {
    if (label) {
        qint32 x = (width() - label->width()) / 2;
        qint32 y = (height() - label->height()) / 2;
        label->move(x, y);
    }
    QWidget::resizeEvent(event);
}


void AnimationWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    constexpr QColor backgroundColor(73, 72, 80);
    painter.fillRect(rect(), backgroundColor);
}








































