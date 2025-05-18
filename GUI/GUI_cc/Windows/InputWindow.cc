#include "InputWindow.h"

InputWindow::InputWindow(const QString& message, QWidget* parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
    setWindowModality(Qt::WindowModal);

    setWindowModality(Qt::ApplicationModal);  // For the modality only within the parent window

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);

    QHBoxLayout * topLayout = new QHBoxLayout();
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    // Creating a closure icon
    closeButton = new QPushButton(this);
    closeButton->setFixedSize(25, 25);
    closeButton->setIcon(QIcon("../Static/icons/topRight/Close.png")); // Icon Installation
    closeButton->setStyleSheet("QPushButton { background: none; border: none; color: white; border-radius: 5px; }"
                               "QPushButton:hover { background-color: rgba(255, 255, 255, 0.3); }"); // Illumination when pointing
    connect(closeButton, &QPushButton::clicked, this, &InputWindow::CloseClicked);

    QLabel* label = new QLabel(message, this);
    label->setStyleSheet("color: #D8D8F6; font-weight: bold; font-size: 16px;");

    // Adding the label first, and then the close button
    topLayout->addWidget(label);
    topLayout->addWidget(closeButton, 0, Qt::AlignRight);

    layout->addLayout(topLayout);

    lineEdit = new QLineEdit(this);
    layout->addWidget(lineEdit);

    okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &InputWindow::OkClicked);
    layout->addWidget(okButton);

    lineEdit->installEventFilter(this);

    setLayout(layout);
    resize(250, 100);
}

QString InputWindow::getText() const {
    return lineEdit->text();
}


void InputWindow::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(50, 50, 50, 240));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);
}

bool InputWindow::eventFilter(QObject* enter, QEvent* event) {
    if (enter == lineEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            OkClicked();
            return true;
        }
    }
    return QWidget::eventFilter(enter, event);
}