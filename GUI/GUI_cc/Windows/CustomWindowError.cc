#include "CustomWindowError.h"

CustomWindowError::CustomWindowError(const QString& message, QWidget* parent) : QWidget(parent) {
    setParent(parent, Qt::SubWindow);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    QHBoxLayout * hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 10, 10, 10);

    QLabel* label = new QLabel(message, this);
    label->setStyleSheet("color: #D8D8F6;");
    hLayout->addWidget(label);

    QPixmap iconPixmap("../Static/icons/error.ico");
    QLabel* iconLabel = new QLabel(this);
    iconLabel->setPixmap(iconPixmap.scaled(32, 32, Qt::KeepAspectRatio)); // Zooming the icon
    hLayout->addWidget(iconLabel);

    setLayout(hLayout);
    updatePosition(parent);

    QTimer::singleShot(2000, this, &CustomWindowError::close);
}

void CustomWindowError::updatePosition(QWidget* parent) {
    if (parent) {
        int x = parent->x() + (parent->width() - width()) / 2;
        int y = parent->y() + (parent->height() - height()) / 2;
        move(x, y);
    }
}

void CustomWindowError::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(50, 50, 50, 240));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);
}

void CustomWindowError::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(500); // Sets the duration of the animation
    animation->setStartValue(0); // Initial transparency
    animation->setEndValue(1);   // The ultimate transparency
    animation->start();
}