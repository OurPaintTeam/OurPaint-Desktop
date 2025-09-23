#include "SaveDialog.h"

SaveDialog::SaveDialog(QWidget* parent) : QDialog(parent) {
    setParent(parent, Qt::SubWindow);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowModality(Qt::ApplicationModal); // For the modality only within the parent window

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    QLabel* label = new QLabel("Save the project?", this);
    layout->addWidget(label);

    QHBoxLayout * buttonLayout = new QHBoxLayout();

    QPushButton* yesButton = new QPushButton("Yes", this);
    QPushButton* noButton = new QPushButton("No", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addWidget(yesButton);
    buttonLayout->addWidget(noButton);
    buttonLayout->addWidget(cancelButton);

    layout->addLayout(buttonLayout);

    connect(yesButton, &QPushButton::clicked, this, [this]() {
        done(QMessageBox::Yes);
    });
    connect(noButton, &QPushButton::clicked, this, [this]() {
        done(QMessageBox::No);
    });
    connect(cancelButton, &QPushButton::clicked, this, [this]() {
        done(QMessageBox::Cancel);
    });
}

void SaveDialog::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(50, 50, 50, 240));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);
}

void SaveDialog::mousePressEvent(QMouseEvent* event) { // Press
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::SizeAllCursor);
        mousePressed = true;
        mousePos = event->pos();
        event->accept();
    }
}

void SaveDialog::mouseMoveEvent(QMouseEvent* event) { // Moving
    if (mousePressed) {
//move(event->globalPos() - mousePos);
        QPoint Pos = pos();
        QPoint newPos = Pos + (event->pos() - mousePos);
        move(newPos);
        event->accept();
    }
}

void SaveDialog::mouseReleaseEvent(QMouseEvent* event) { // release
    if (event->button() == Qt::LeftButton) {
        setCursor(Qt::ArrowCursor);
        mousePressed = false;
        event->accept();
    }
}